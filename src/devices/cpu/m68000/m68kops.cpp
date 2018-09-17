// Generated source, edits will be lost.  Run m68kmake.py instead

#include "emu.h"
#include "m68000.h"

void m68000_base_device::m68k_op_1010_0()
{
	m68ki_exception_1010();


}
void m68000_base_device::m68k_op_1111_0()
{
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_040fpu0_l_0()
{
	if(m_has_fpu)
	{
		m68040_fpu_op0();
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_040fpu1_l_0()
{
	if(m_has_fpu)
	{
		m68040_fpu_op1();
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_abcd_b_0()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void m68000_base_device::m68k_op_abcd_b_1()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_abcd_b_2()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_abcd_b_3()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_abcd_b_4()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_1();
	uint32_t corf = 0;

	if(res > 9)
		corf = 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	m_v_flag = ~res; /* Undefined V behavior */
	res += corf;
	m_x_flag = m_c_flag = (res > 0x9f) << 8;
	if(m_c_flag)
		res -= 0xa0;

	m_v_flag &= res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_add_b_0()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_pi7()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_pd7()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_A7_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_di()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_al()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_1_i()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_2()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_3()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(AY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_di()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_al()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_w_4_i()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_5()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_6()
{
	uint32_t* r_dst = &DX();
	uint32_t src = AY();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_di()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_al()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_l_7_i()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_add_b_8_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_b_8_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_b_8_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_b_8_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_b_8_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_b_8_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_b_8_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_b_8_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_b_8_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_w_9_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_w_9_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_w_9_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_w_9_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_w_9_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_w_9_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_w_9_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_l_10_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_l_10_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_l_10_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_l_10_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_l_10_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_l_10_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_add_l_10_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_adda_w_0()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(DY()));


}
void m68000_base_device::m68k_op_adda_w_1()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(AY()));


}
void m68000_base_device::m68k_op_adda_w_2_ai()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_pi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_pd()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_di()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_ix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_aw()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AW_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_al()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AL_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_pcdi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_pcix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_w_2_i()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_I_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_3()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + DY());


}
void m68000_base_device::m68k_op_adda_l_4()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + AY());


}
void m68000_base_device::m68k_op_adda_l_5_ai()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_AI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_pi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_pd()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PD_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_di()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_DI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_ix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_IX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_aw()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AW_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_al()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AL_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_pcdi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCDI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_pcix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCIX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_adda_l_5_i()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_I_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);


}
void m68000_base_device::m68k_op_addi_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_addi_b_1_ai()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_b_1_pi()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_b_1_pi7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_b_1_pd()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_b_1_pd7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_b_1_di()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_b_1_ix()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_b_1_aw()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_b_1_al()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_w_2()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_addi_w_3_ai()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_w_3_pi()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_w_3_pd()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_w_3_di()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_w_3_ix()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_w_3_aw()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_w_3_al()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_l_4()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_32();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_addi_l_5_ai()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_l_5_pi()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_l_5_pd()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_l_5_di()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_l_5_ix()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_l_5_aw()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addi_l_5_al()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_addq_b_1_ai()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_1_pi()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_1_pi7()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_1_pd()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_1_pd7()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_1_di()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_1_ix()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_1_aw()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_b_1_al()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_w_2()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_addq_w_3()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((m_ir >> 9) - 1) & 7) + 1);


}
void m68000_base_device::m68k_op_addq_w_4_ai()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_w_4_pi()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_w_4_pd()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_w_4_di()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_w_4_ix()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_w_4_aw()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_w_4_al()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst;

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_l_5()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = *r_dst;
	uint32_t res = src + dst;

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_addq_l_6()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((m_ir >> 9) - 1) & 7) + 1);


}
void m68000_base_device::m68k_op_addq_l_7_ai()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_l_7_pi()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_l_7_pd()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_l_7_di()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_l_7_ix()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_l_7_aw()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addq_l_7_al()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst;


	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_addx_b_0()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void m68000_base_device::m68k_op_addx_w_1()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;


}
void m68000_base_device::m68k_op_addx_l_2()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	*r_dst = res;


}
void m68000_base_device::m68k_op_addx_b_3()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_addx_b_4()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_addx_b_5()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_addx_b_6()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_v_flag = VFLAG_ADD_8(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_addx_w_7()
{
	uint32_t src = OPER_AY_PD_16();
	uint32_t ea  = EA_AX_PD_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_v_flag = VFLAG_ADD_16(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_addx_l_8()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t ea  = EA_AX_PD_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = src + dst + XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_ADD_32(src, dst, res);
	m_x_flag = m_c_flag = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_and_b_0()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (DY() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_ai()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_AI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_pi()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_PI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_pi7()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_A7_PI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_pd()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_PD_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_pd7()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_A7_PD_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_di()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_DI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_ix()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_IX_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_aw()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AW_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_al()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AL_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_pcdi()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_PCDI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_pcix()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_PCIX_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_1_i()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_I_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_2()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (DY() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_ai()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_AI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_pi()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_PI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_pd()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_PD_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_di()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_DI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_ix()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_IX_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_aw()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AW_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_al()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AL_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_pcdi()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_PCDI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_pcix()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_PCIX_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_w_3_i()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_I_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_4()
{
	m_not_z_flag = DX() &= DY();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_ai()
{
	m_not_z_flag = DX() &= OPER_AY_AI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_pi()
{
	m_not_z_flag = DX() &= OPER_AY_PI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_pd()
{
	m_not_z_flag = DX() &= OPER_AY_PD_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_di()
{
	m_not_z_flag = DX() &= OPER_AY_DI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_ix()
{
	m_not_z_flag = DX() &= OPER_AY_IX_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_aw()
{
	m_not_z_flag = DX() &= OPER_AW_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_al()
{
	m_not_z_flag = DX() &= OPER_AL_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_pcdi()
{
	m_not_z_flag = DX() &= OPER_PCDI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_pcix()
{
	m_not_z_flag = DX() &= OPER_PCIX_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_l_5_i()
{
	m_not_z_flag = DX() &= OPER_I_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_and_b_6_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_b_6_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_b_6_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_b_6_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_b_6_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_b_6_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_b_6_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_b_6_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_b_6_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_w_7_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_w_7_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_w_7_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_w_7_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_w_7_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_w_7_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_w_7_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_and_l_8_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_and_l_8_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_and_l_8_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_and_l_8_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_and_l_8_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_and_l_8_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_and_l_8_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_andi_b_0()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DY() &= (OPER_I_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_andi_b_1_ai()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_b_1_pi()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_b_1_pi7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_b_1_pd()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_b_1_pd7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_b_1_di()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_b_1_ix()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_b_1_aw()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_b_1_al()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t res = src & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_andi_w_2()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DY() &= (OPER_I_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_andi_w_3_ai()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = src & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_andi_w_3_pi()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = src & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_andi_w_3_pd()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = src & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_andi_w_3_di()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = src & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_andi_w_3_ix()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = src & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_andi_w_3_aw()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t res = src & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_andi_w_3_al()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t res = src & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_andi_l_4()
{
	m_not_z_flag = DY() &= (OPER_I_32());

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_andi_l_5_ai()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = src & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_andi_l_5_pi()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = src & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_andi_l_5_pd()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = src & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_andi_l_5_di()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = src & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_andi_l_5_ix()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = src & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_andi_l_5_aw()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t res = src & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_andi_l_5_al()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t res = src & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_andi_w_6()
{
	m68ki_set_ccr(m68ki_get_ccr() & OPER_I_8());


}
void m68000_base_device::m68k_op_andi_w_7()
{
	if(m_s_flag)
	{
		uint32_t src = OPER_I_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() & src);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_asr_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	if(GET_MSB_8(src))
		res |= m68ki_shift_8_table[shift];

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);


}
void m68000_base_device::m68k_op_asr_w_1()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	if(GET_MSB_16(src))
		res |= m68ki_shift_16_table[shift];

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);


}
void m68000_base_device::m68k_op_asr_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	if(GET_MSB_32(src))
		res |= m68ki_shift_32_table[shift];

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);


}
void m68000_base_device::m68k_op_asr_b_3()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift < 8)
		{
			if(GET_MSB_8(src))
				res |= m68ki_shift_8_table[shift];

			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

			m_x_flag = m_c_flag = src << (9-shift);
			m_n_flag = NFLAG_8(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_8(src))
		{
			*r_dst |= 0xff;
			m_c_flag = CFLAG_SET;
			m_x_flag = XFLAG_SET;
			m_n_flag = NFLAG_SET;
			m_not_z_flag = ZFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_8(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_asr_w_4()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift < 16)
		{
			if(GET_MSB_16(src))
				res |= m68ki_shift_16_table[shift];

			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

			m_c_flag = m_x_flag = (src >> (shift - 1))<<8;
			m_n_flag = NFLAG_16(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_16(src))
		{
			*r_dst |= 0xffff;
			m_c_flag = CFLAG_SET;
			m_x_flag = XFLAG_SET;
			m_n_flag = NFLAG_SET;
			m_not_z_flag = ZFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_16(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_asr_l_5()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift < 32)
		{
			if(GET_MSB_32(src))
				res |= m68ki_shift_32_table[shift];

			*r_dst = res;

			m_c_flag = m_x_flag = (src >> (shift - 1))<<8;
			m_n_flag = NFLAG_32(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_32(src))
		{
			*r_dst = 0xffffffff;
			m_c_flag = CFLAG_SET;
			m_x_flag = XFLAG_SET;
			m_n_flag = NFLAG_SET;
			m_not_z_flag = ZFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_32(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_asr_w_6_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void m68000_base_device::m68k_op_asr_w_6_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void m68000_base_device::m68k_op_asr_w_6_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void m68000_base_device::m68k_op_asr_w_6_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void m68000_base_device::m68k_op_asr_w_6_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void m68000_base_device::m68k_op_asr_w_6_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void m68000_base_device::m68k_op_asr_w_6_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = m_x_flag = src << 8;


}
void m68000_base_device::m68k_op_asl_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_x_flag = m_c_flag = src << shift;
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	src &= m68ki_shift_8_table[shift + 1];
	m_v_flag = (!(src == 0 || (src == m68ki_shift_8_table[shift + 1] && shift < 8)))<<7;


}
void m68000_base_device::m68k_op_asl_w_1()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (8-shift);
	src &= m68ki_shift_16_table[shift + 1];
	m_v_flag = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;


}
void m68000_base_device::m68k_op_asl_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (24-shift);
	src &= m68ki_shift_32_table[shift + 1];
	m_v_flag = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;


}
void m68000_base_device::m68k_op_asl_b_3()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift < 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			m_x_flag = m_c_flag = src << shift;
			m_n_flag = NFLAG_8(res);
			m_not_z_flag = res;
			src &= m68ki_shift_8_table[shift + 1];
			m_v_flag = (!(src == 0 || src == m68ki_shift_8_table[shift + 1]))<<7;
			return;
		}

		*r_dst &= 0xffffff00;
		m_x_flag = m_c_flag = ((shift == 8 ? src & 1 : 0))<<8;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = (!(src == 0))<<7;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_8(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_asl_w_4()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift < 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			m_x_flag = m_c_flag = (src << shift) >> 8;
			m_n_flag = NFLAG_16(res);
			m_not_z_flag = res;
			src &= m68ki_shift_16_table[shift + 1];
			m_v_flag = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
			return;
		}

		*r_dst &= 0xffff0000;
		m_x_flag = m_c_flag = ((shift == 16 ? src & 1 : 0))<<8;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = (!(src == 0))<<7;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_16(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_asl_l_5()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift < 32)
		{
			*r_dst = res;
			m_x_flag = m_c_flag = (src >> (32 - shift)) << 8;
			m_n_flag = NFLAG_32(res);
			m_not_z_flag = res;
			src &= m68ki_shift_32_table[shift + 1];
			m_v_flag = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
			return;
		}

		*r_dst = 0;
		m_x_flag = m_c_flag = ((shift == 32 ? src & 1 : 0))<<8;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = (!(src == 0))<<7;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_32(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_asl_w_6_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void m68000_base_device::m68k_op_asl_w_6_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void m68000_base_device::m68k_op_asl_w_6_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void m68000_base_device::m68k_op_asl_w_6_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void m68000_base_device::m68k_op_asl_w_6_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void m68000_base_device::m68k_op_asl_w_6_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void m68000_base_device::m68k_op_asl_w_6_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	src &= 0xc000;
	m_v_flag = (!(src == 0 || src == 0xc000))<<7;


}
void m68000_base_device::m68k_op_bhi_b_0()
{
	if(COND_HI())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bls_b_0()
{
	if(COND_LS())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bcc_b_3()
{
	if(COND_CC())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bcs_b_0()
{
	if(COND_CS())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bne_b_0()
{
	if(COND_NE())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_beq_b_0()
{
	if(COND_EQ())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bvc_b_0()
{
	if(COND_VC())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bvs_b_0()
{
	if(COND_VS())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bpl_b_0()
{
	if(COND_PL())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bmi_b_0()
{
	if(COND_MI())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bge_b_0()
{
	if(COND_GE())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_blt_b_0()
{
	if(COND_LT())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bgt_b_0()
{
	if(COND_GT())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_ble_b_0()
{
	if(COND_LE())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_icount -= m_cyc_bcc_notake_b;


}
void m68000_base_device::m68k_op_bhi_w_1()
{
	if(COND_HI())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bls_w_1()
{
	if(COND_LS())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bcc_w_4()
{
	if(COND_CC())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bcs_w_1()
{
	if(COND_CS())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bne_w_1()
{
	if(COND_NE())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_beq_w_1()
{
	if(COND_EQ())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bvc_w_1()
{
	if(COND_VC())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bvs_w_1()
{
	if(COND_VS())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bpl_w_1()
{
	if(COND_PL())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bmi_w_1()
{
	if(COND_MI())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bge_w_1()
{
	if(COND_GE())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_blt_w_1()
{
	if(COND_LT())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bgt_w_1()
{
	if(COND_GT())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_ble_w_1()
{
	if(COND_LE())
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_bcc_notake_w;


}
void m68000_base_device::m68k_op_bhi_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_HI())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_HI())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bls_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LS())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_LS())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bcc_l_5()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CC())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_CC())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bcs_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CS())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_CS())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bne_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_NE())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_NE())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_beq_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_EQ())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_EQ())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bvc_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VC())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_VC())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bvs_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VS())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_VS())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bpl_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_PL())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_PL())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bmi_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_MI())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_MI())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bge_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GE())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_GE())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_blt_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LT())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_LT())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bgt_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GT())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_GT())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_ble_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LE())
		{
			uint32_t offset = OPER_I_32();
			m_pc -= 4;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		m_pc += 4;
		return;
	}
	else
	{
		if(COND_LE())
		{
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
			return;
		}
		m_icount -= m_cyc_bcc_notake_b;
	}


}
void m68000_base_device::m68k_op_bchg_l_0()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst ^= mask;


}
void m68000_base_device::m68k_op_bchg_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst ^= mask;


}
void m68000_base_device::m68k_op_bchg_b_3_ai()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_3_pi()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_3_pi7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_3_pd()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_3_pd7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_3_di()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_3_ix()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_3_aw()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bchg_b_3_al()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);


}
void m68000_base_device::m68k_op_bclr_l_0()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst &= ~mask;


}
void m68000_base_device::m68k_op_bclr_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst &= ~mask;


}
void m68000_base_device::m68k_op_bclr_b_3_ai()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_3_pi()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_3_pi7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_3_pd()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_3_pd7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_3_di()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_3_ix()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_3_aw()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bclr_b_3_al()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);


}
void m68000_base_device::m68k_op_bfchg_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t* data = &DY();
		uint64_t mask;


		if(BIT_B(word2))
			offset = REG_D()[offset&7];
		if(BIT_5(word2))
			width = REG_D()[width&7];

		offset &= 31;
		width = ((width-1) & 31) + 1;

		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		m_n_flag = NFLAG_32(*data<<offset);
		m_not_z_flag = *data & mask;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		*data ^= mask;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfchg_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfchg_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfchg_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfchg_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfchg_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfclr_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t* data = &DY();
		uint64_t mask;


		if(BIT_B(word2))
			offset = REG_D()[offset&7];
		if(BIT_5(word2))
			width = REG_D()[width&7];


		offset &= 31;
		width = ((width-1) & 31) + 1;


		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		m_n_flag = NFLAG_32(*data<<offset);
		m_not_z_flag = *data & mask;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		*data &= ~mask;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfclr_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfclr_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfclr_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfclr_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfclr_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfexts_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint64_t data = DY();


		if(BIT_B(word2))
			offset = REG_D()[offset&7];
		if(BIT_5(word2))
			width = REG_D()[width&7];

		offset &= 31;
		width = ((width-1) & 31) + 1;

		data = ROL_32(data, offset);
		m_n_flag = NFLAG_32(data);
		data = MAKE_INT_32(data) >> (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2>>12)&7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfexts_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfexts_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfexts_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfexts_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfexts_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfexts_l_1_pcdi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_PCDI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfexts_l_1_pcix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_PCIX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfextu_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint64_t data = DY();


		if(BIT_B(word2))
			offset = REG_D()[offset&7];
		if(BIT_5(word2))
			width = REG_D()[width&7];

		offset &= 31;
		width = ((width-1) & 31) + 1;

		data = ROL_32(data, offset);
		m_n_flag = NFLAG_32(data);
		data >>= 32 - width;

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2>>12)&7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfextu_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AY_AI_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfextu_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AY_DI_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfextu_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AY_IX_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfextu_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AW_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfextu_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_AL_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfextu_l_1_pcdi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_PCDI_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfextu_l_1_pcix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t data;
		uint32_t ea = EA_PCIX_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		REG_D()[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfffo_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint64_t data = DY();
		uint32_t bit;


		if(BIT_B(word2))
			offset = REG_D()[offset&7];
		if(BIT_5(word2))
			width = REG_D()[width&7];

		offset &= 31;
		width = ((width-1) & 31) + 1;

		data = ROL_32(data, offset);
		m_n_flag = NFLAG_32(data);
		data >>= 32 - width;

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D()[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfffo_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		int32_t local_offset;
		uint32_t width = word2;
		uint32_t data;
		uint32_t bit;
		uint32_t ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D()[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfffo_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		int32_t local_offset;
		uint32_t width = word2;
		uint32_t data;
		uint32_t bit;
		uint32_t ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D()[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfffo_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		int32_t local_offset;
		uint32_t width = word2;
		uint32_t data;
		uint32_t bit;
		uint32_t ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D()[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfffo_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		int32_t local_offset;
		uint32_t width = word2;
		uint32_t data;
		uint32_t bit;
		uint32_t ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D()[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfffo_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		int32_t local_offset;
		uint32_t width = word2;
		uint32_t data;
		uint32_t bit;
		uint32_t ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D()[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfffo_l_1_pcdi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		int32_t local_offset;
		uint32_t width = word2;
		uint32_t data;
		uint32_t bit;
		uint32_t ea = EA_PCDI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D()[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfffo_l_1_pcix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		int32_t local_offset;
		uint32_t width = word2;
		uint32_t data;
		uint32_t bit;
		uint32_t ea = EA_PCIX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = (offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		m_n_flag = NFLAG_32(data);
		data  >>= (32 - width);

		m_not_z_flag = data;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D()[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfins_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t* data = &DY();
		uint64_t mask;
		uint64_t insert = REG_D()[(word2>>12)&7];


		if(BIT_B(word2))
			offset = REG_D()[offset&7];
		if(BIT_5(word2))
			width = REG_D()[width&7];


		offset &= 31;
		width = ((width-1) & 31) + 1;


		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		insert = MASK_OUT_ABOVE_32(insert << (32 - width));
		m_n_flag = NFLAG_32(insert);
		m_not_z_flag = insert;
		insert = ROR_32(insert, offset);

		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		*data &= ~mask;
		*data |= insert;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfins_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t insert_base = REG_D()[(word2>>12)&7];
		uint32_t insert_long;
		uint32_t insert_byte;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		m_n_flag = NFLAG_32(insert_base);
		m_not_z_flag = insert_base;
		insert_long = insert_base >> offset;

		data_long = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) < 8)
		{
			m68ki_write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
		}
		else if((width + offset) < 16)
		{
			m68ki_write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
		}
		else
		{
			m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);
		}

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (insert_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfins_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t insert_base = REG_D()[(word2>>12)&7];
		uint32_t insert_long;
		uint32_t insert_byte;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		m_n_flag = NFLAG_32(insert_base);
		m_not_z_flag = insert_base;
		insert_long = insert_base >> offset;

		data_long = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) < 8)
		{
			m68ki_write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
		}
		else if((width + offset) < 16)
		{
			m68ki_write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
		}
		else
		{
			m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);
		}

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (insert_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfins_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t insert_base = REG_D()[(word2>>12)&7];
		uint32_t insert_long;
		uint32_t insert_byte;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		m_n_flag = NFLAG_32(insert_base);
		m_not_z_flag = insert_base;
		insert_long = insert_base >> offset;

		data_long = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) < 8)
		{
			m68ki_write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
		}
		else if((width + offset) < 16)
		{
			m68ki_write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
		}
		else
		{
			m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);
		}

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (insert_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfins_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t insert_base = REG_D()[(word2>>12)&7];
		uint32_t insert_long;
		uint32_t insert_byte;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		m_n_flag = NFLAG_32(insert_base);
		m_not_z_flag = insert_base;
		insert_long = insert_base >> offset;

		data_long = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) < 8)
		{
			m68ki_write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
		}
		else if((width + offset) < 16)
		{
			m68ki_write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
		}
		else
		{
			m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);
		}

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (insert_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfins_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t insert_base = REG_D()[(word2>>12)&7];
		uint32_t insert_long;
		uint32_t insert_byte;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		if(BIT_B(word2))
		{
			/* Offset is signed so we have to use ugly math =( */
			ea += offset / 8;
			offset %= 8;
			if(offset < 0)
			{
				offset += 8;
				ea--;
			}
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		m_n_flag = NFLAG_32(insert_base);
		m_not_z_flag = insert_base;
		insert_long = insert_base >> offset;

		data_long = (offset+width) < 8 ? (m68ki_read_8(ea) << 24) :
				(offset+width) < 16 ? (m68ki_read_16(ea) << 16) : m68ki_read_32(ea);
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) < 8)
		{
			m68ki_write_8(ea, ((data_long & ~mask_long) | insert_long) >> 24);
		}
		else if((width + offset) < 16)
		{
			m68ki_write_16(ea, ((data_long & ~mask_long) | insert_long) >> 16);
		}
		else
		{
			m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);
		}

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			insert_byte = MASK_OUT_ABOVE_8(insert_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (insert_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfset_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t* data = &DY();
		uint64_t mask;


		if(BIT_B(word2))
			offset = REG_D()[offset&7];
		if(BIT_5(word2))
			width = REG_D()[width&7];


		offset &= 31;
		width = ((width-1) & 31) + 1;


		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		m_n_flag = NFLAG_32(*data<<offset);
		m_not_z_flag = *data & mask;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		*data |= mask;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfset_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_AI_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfset_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_DI_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfset_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_IX_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfset_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AW_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bfset_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AL_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = NFLAG_32(data_long << offset);
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bftst_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t* data = &DY();
		uint64_t mask;


		if(BIT_B(word2))
			offset = REG_D()[offset&7];
		if(BIT_5(word2))
			width = REG_D()[width&7];


		offset &= 31;
		width = ((width-1) & 31) + 1;


		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		m_n_flag = NFLAG_32(*data<<offset);
		m_not_z_flag = *data & mask;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bftst_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_AI_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bftst_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_DI_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bftst_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AY_IX_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bftst_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AW_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bftst_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_AL_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bftst_l_1_pcdi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_PCDI_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bftst_l_1_pcix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t offset = (word2>>6)&31;
		uint32_t width = word2;
		uint32_t mask_base;
		uint32_t data_long;
		uint32_t mask_long;
		uint32_t data_byte = 0;
		uint32_t mask_byte = 0;
		uint32_t ea = EA_PCIX_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D()[offset&7]);
		if(BIT_5(word2))
			width = REG_D()[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		m_n_flag = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		m_not_z_flag = data_long & mask_long;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base) << (8-offset);
			data_byte = m68ki_read_8(ea+4);
			m_not_z_flag |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bkpt_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		(void)m_cpu_space->read_dword((m_ir & 7) << 2, 0xffffffff);
	}
	else if(CPU_TYPE_IS_010_PLUS())
	{
		(void)m_cpu_space->read_word(0x000000, 0xffff);
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_bra_b_0()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_bra_w_1()
{
	uint32_t offset = OPER_I_16();
	m_pc -= 2;
	m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
	m68ki_branch_16(offset);
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_bra_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t offset = OPER_I_32();
		m_pc -= 4;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_32(offset);
		if(m_pc == m_ppc && m_icount > 0)
			m_icount = 0;
		return;
	}
	else
	{
		m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		if(m_pc == m_ppc && m_icount > 0)
			m_icount = 0;
	}


}
void m68000_base_device::m68k_op_bset_l_0()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst |= mask;


}
void m68000_base_device::m68k_op_bset_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst |= mask;


}
void m68000_base_device::m68k_op_bset_b_3_ai()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_3_pi()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_3_pi7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_3_pd()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_3_pd7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_3_di()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_3_ix()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_3_aw()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bset_b_3_al()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);


}
void m68000_base_device::m68k_op_bsr_b_0()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));


}
void m68000_base_device::m68k_op_bsr_w_1()
{
	uint32_t offset = OPER_I_16();
	m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m_pc -= 2;
	m68ki_branch_16(offset);


}
void m68000_base_device::m68k_op_bsr_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t offset = OPER_I_32();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_push_32(m_pc);
		m_pc -= 4;
		m68ki_branch_32(offset);
		return;
	}
	else
	{
		m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
		m68ki_push_32(m_pc);
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
	}


}
void m68000_base_device::m68k_op_btst_l_0()
{
	m_not_z_flag = DY() & (1 << (DX() & 0x1f));


}
void m68000_base_device::m68k_op_btst_b_1_ai()
{
	m_not_z_flag = OPER_AY_AI_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_pi()
{
	m_not_z_flag = OPER_AY_PI_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_pi7()
{
	m_not_z_flag = OPER_A7_PI_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_pd()
{
	m_not_z_flag = OPER_AY_PD_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_pd7()
{
	m_not_z_flag = OPER_A7_PD_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_di()
{
	m_not_z_flag = OPER_AY_DI_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_ix()
{
	m_not_z_flag = OPER_AY_IX_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_aw()
{
	m_not_z_flag = OPER_AW_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_al()
{
	m_not_z_flag = OPER_AL_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_pcdi()
{
	m_not_z_flag = OPER_PCDI_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_pcix()
{
	m_not_z_flag = OPER_PCIX_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_b_1_i()
{
	m_not_z_flag = OPER_I_8() & (1 << (DX() & 7));


}
void m68000_base_device::m68k_op_btst_l_2()
{
	m_not_z_flag = DY() & (1 << (OPER_I_8() & 0x1f));


}
void m68000_base_device::m68k_op_btst_b_3_ai()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_AI_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_pi()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_PI_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_pi7()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_A7_PI_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_pd()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_PD_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_pd7()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_A7_PD_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_di()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_DI_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_ix()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_IX_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_aw()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AW_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_al()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AL_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_pcdi()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_PCDI_8() & (1 << bit);


}
void m68000_base_device::m68k_op_btst_b_3_pcix()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_PCIX_8() & (1 << bit);


}
void m68000_base_device::m68k_op_callm_l_0_ai()
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT())
	{
		uint32_t ea = EA_AY_AI_32();

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_pc += 2;
		(void)ea;   /* just to avoid an 'unused variable' warning */
		logerror("%s at %08x: called unimplemented instruction %04x (callm)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_callm_l_0_di()
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT())
	{
		uint32_t ea = EA_AY_DI_32();

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_pc += 2;
		(void)ea;   /* just to avoid an 'unused variable' warning */
		logerror("%s at %08x: called unimplemented instruction %04x (callm)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_callm_l_0_ix()
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT())
	{
		uint32_t ea = EA_AY_IX_32();

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_pc += 2;
		(void)ea;   /* just to avoid an 'unused variable' warning */
		logerror("%s at %08x: called unimplemented instruction %04x (callm)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_callm_l_0_aw()
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT())
	{
		uint32_t ea = EA_AW_32();

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_pc += 2;
		(void)ea;   /* just to avoid an 'unused variable' warning */
		logerror("%s at %08x: called unimplemented instruction %04x (callm)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_callm_l_0_al()
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT())
	{
		uint32_t ea = EA_AL_32();

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_pc += 2;
		(void)ea;   /* just to avoid an 'unused variable' warning */
		logerror("%s at %08x: called unimplemented instruction %04x (callm)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_callm_l_0_pcdi()
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT())
	{
		uint32_t ea = EA_PCDI_32();

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_pc += 2;
		(void)ea;   /* just to avoid an 'unused variable' warning */
		logerror("%s at %08x: called unimplemented instruction %04x (callm)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_callm_l_0_pcix()
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT())
	{
		uint32_t ea = EA_PCIX_32();

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_pc += 2;
		(void)ea;   /* just to avoid an 'unused variable' warning */
		logerror("%s at %08x: called unimplemented instruction %04x (callm)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_pi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_pi7()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_A7_PI_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_pd()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_pd7()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_A7_PD_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_b_0_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_8();
		uint32_t dest = m68ki_read_8(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(*compare, dest, res);
		m_c_flag = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_w_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_16();
		uint32_t dest = m68ki_read_16(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(*compare, dest, res);
		m_c_flag = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_w_1_pi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_16();
		uint32_t dest = m68ki_read_16(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(*compare, dest, res);
		m_c_flag = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_w_1_pd()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_16();
		uint32_t dest = m68ki_read_16(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(*compare, dest, res);
		m_c_flag = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_w_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_16();
		uint32_t dest = m68ki_read_16(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(*compare, dest, res);
		m_c_flag = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_w_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_16();
		uint32_t dest = m68ki_read_16(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(*compare, dest, res);
		m_c_flag = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_w_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_16();
		uint32_t dest = m68ki_read_16(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(*compare, dest, res);
		m_c_flag = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_w_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_16();
		uint32_t dest = m68ki_read_16(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(*compare, dest, res);
		m_c_flag = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			m_icount -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_l_2_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_AI_32();
		uint32_t dest = m68ki_read_32(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - *compare;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(*compare, dest, res);
		m_c_flag = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			m_icount -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_l_2_pi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PI_32();
		uint32_t dest = m68ki_read_32(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - *compare;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(*compare, dest, res);
		m_c_flag = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			m_icount -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_l_2_pd()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_PD_32();
		uint32_t dest = m68ki_read_32(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - *compare;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(*compare, dest, res);
		m_c_flag = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			m_icount -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_l_2_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_DI_32();
		uint32_t dest = m68ki_read_32(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - *compare;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(*compare, dest, res);
		m_c_flag = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			m_icount -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_l_2_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AY_IX_32();
		uint32_t dest = m68ki_read_32(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - *compare;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(*compare, dest, res);
		m_c_flag = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			m_icount -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_l_2_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AW_32();
		uint32_t dest = m68ki_read_32(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - *compare;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(*compare, dest, res);
		m_c_flag = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			m_icount -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas_l_2_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint32_t ea = EA_AL_32();
		uint32_t dest = m68ki_read_32(ea);
		uint32_t* compare = &REG_D()[word2 & 7];
		uint32_t res = dest - *compare;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(*compare, dest, res);
		m_c_flag = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			m_icount -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas2_w_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_32();
		uint32_t* compare1 = &REG_D()[(word2 >> 16) & 7];
		uint32_t ea1 = REG_DA()[(word2 >> 28) & 15];
		uint32_t dest1 = m68ki_read_16(ea1);
		uint32_t res1 = dest1 - MASK_OUT_ABOVE_16(*compare1);
		uint32_t* compare2 = &REG_D()[word2 & 7];
		uint32_t ea2 = REG_DA()[(word2 >> 12) & 15];
		uint32_t dest2 = m68ki_read_16(ea2);
		uint32_t res2;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_16(res1);
		m_not_z_flag = MASK_OUT_ABOVE_16(res1);
		m_v_flag = VFLAG_SUB_16(*compare1, dest1, res1);
		m_c_flag = CFLAG_16(res1);

		if(COND_EQ())
		{
			res2 = dest2 - MASK_OUT_ABOVE_16(*compare2);

			m_n_flag = NFLAG_16(res2);
			m_not_z_flag = MASK_OUT_ABOVE_16(res2);
			m_v_flag = VFLAG_SUB_16(*compare2, dest2, res2);
			m_c_flag = CFLAG_16(res2);

			if(COND_EQ())
			{
				m_icount -= 3;
				m68ki_write_16(ea1, REG_D()[(word2 >> 22) & 7]);
				m68ki_write_16(ea2, REG_D()[(word2 >> 6) & 7]);
				return;
			}
		}
		*compare1 = BIT_1F(word2) ? MAKE_INT_16(dest1) : MASK_OUT_BELOW_16(*compare1) | dest1;
		*compare2 = BIT_F(word2) ? MAKE_INT_16(dest2) : MASK_OUT_BELOW_16(*compare2) | dest2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cas2_l_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_32();
		uint32_t* compare1 = &REG_D()[(word2 >> 16) & 7];
		uint32_t ea1 = REG_DA()[(word2 >> 28) & 15];
		uint32_t dest1 = m68ki_read_32(ea1);
		uint32_t res1 = dest1 - *compare1;
		uint32_t* compare2 = &REG_D()[word2 & 7];
		uint32_t ea2 = REG_DA()[(word2 >> 12) & 15];
		uint32_t dest2 = m68ki_read_32(ea2);
		uint32_t res2;

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_n_flag = NFLAG_32(res1);
		m_not_z_flag = MASK_OUT_ABOVE_32(res1);
		m_v_flag = VFLAG_SUB_32(*compare1, dest1, res1);
		m_c_flag = CFLAG_SUB_32(*compare1, dest1, res1);

		if(COND_EQ())
		{
			res2 = dest2 - *compare2;

			m_n_flag = NFLAG_32(res2);
			m_not_z_flag = MASK_OUT_ABOVE_32(res2);
			m_v_flag = VFLAG_SUB_32(*compare2, dest2, res2);
			m_c_flag = CFLAG_SUB_32(*compare2, dest2, res2);

			if(COND_EQ())
			{
				m_icount -= 3;
				m68ki_write_32(ea1, REG_D()[(word2 >> 22) & 7]);
				m68ki_write_32(ea2, REG_D()[(word2 >> 6) & 7]);
				return;
			}
		}
		*compare1 = dest1;
		*compare2 = dest2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_w_0()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(DY());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_ai()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_AI_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_pi()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_PI_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_pd()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_PD_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_di()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_DI_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_ix()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AY_IX_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_aw()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AW_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_al()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_AL_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_pcdi()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_PCDI_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_pcix()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_PCIX_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_w_1_i()
{
	int32_t src = MAKE_INT_16(DX());
	int32_t bound = MAKE_INT_16(OPER_I_16());

	m_not_z_flag = ZFLAG_16(src); /* Undocumented */
	m_v_flag = VFLAG_CLEAR;   /* Undocumented */
	m_c_flag = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	m_n_flag = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);


}
void m68000_base_device::m68k_op_chk_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(DY());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_AY_AI_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_pi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_AY_PI_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_pd()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_AY_PD_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_AY_DI_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_AY_IX_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_AW_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_AL_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_pcdi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_PCDI_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_pcix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_PCIX_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk_l_3_i()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		int32_t src = MAKE_INT_32(DX());
		int32_t bound = MAKE_INT_32(OPER_I_32());

		m_not_z_flag = ZFLAG_32(src); /* Undocumented */
		m_v_flag = VFLAG_CLEAR;   /* Undocumented */
		m_c_flag = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		m_n_flag = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_b_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xff;

		uint32_t ea = EA_PCDI_8();
		int32_t lower_bound = m68ki_read_pcrel_8(ea);
		int32_t upper_bound = m68ki_read_pcrel_8(ea + 1);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80)
		{
			lower_bound = (int32_t)(int8_t)lower_bound;
			upper_bound = (int32_t)(int8_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int8_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_b_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xff;

		uint32_t ea = EA_PCIX_8();
		int32_t lower_bound = m68ki_read_pcrel_8(ea);
		int32_t upper_bound = m68ki_read_pcrel_8(ea + 1);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80)
		{
			lower_bound = (int32_t)(int8_t)lower_bound;
			upper_bound = (int32_t)(int8_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int8_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_b_2_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xff;

		uint32_t ea = EA_AY_AI_8();
		int32_t lower_bound = m68ki_read_8(ea);
		int32_t upper_bound = m68ki_read_8(ea + 1);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80)
		{
			lower_bound = (int32_t)(int8_t)lower_bound;
			upper_bound = (int32_t)(int8_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int8_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_b_2_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xff;

		uint32_t ea = EA_AY_DI_8();
		int32_t lower_bound = m68ki_read_8(ea);
		int32_t upper_bound = m68ki_read_8(ea + 1);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80)
		{
			lower_bound = (int32_t)(int8_t)lower_bound;
			upper_bound = (int32_t)(int8_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int8_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_b_2_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xff;

		uint32_t ea = EA_AY_IX_8();
		int32_t lower_bound = m68ki_read_8(ea);
		int32_t upper_bound = m68ki_read_8(ea + 1);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80)
		{
			lower_bound = (int32_t)(int8_t)lower_bound;
			upper_bound = (int32_t)(int8_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int8_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_b_2_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xff;

		uint32_t ea = EA_AW_8();
		int32_t lower_bound = m68ki_read_8(ea);
		int32_t upper_bound = m68ki_read_8(ea + 1);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80)
		{
			lower_bound = (int32_t)(int8_t)lower_bound;
			upper_bound = (int32_t)(int8_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int8_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_b_2_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xff;

		uint32_t ea = EA_AL_8();
		int32_t lower_bound = m68ki_read_8(ea);
		int32_t upper_bound = m68ki_read_8(ea + 1);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80)
		{
			lower_bound = (int32_t)(int8_t)lower_bound;
			upper_bound = (int32_t)(int8_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int8_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_w_3()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xffff;

		uint32_t ea = EA_PCDI_16();
		int32_t lower_bound = m68ki_read_pcrel_16(ea);
		int32_t upper_bound = m68ki_read_pcrel_16(ea + 2);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x8000)
		{
			lower_bound = (int32_t)(int16_t)lower_bound;
			upper_bound = (int32_t)(int16_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int16_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_w_4()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xffff;

		uint32_t ea = EA_PCIX_16();
		int32_t lower_bound = m68ki_read_pcrel_16(ea);
		int32_t upper_bound = m68ki_read_pcrel_16(ea + 2);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x8000)
		{
			lower_bound = (int32_t)(int16_t)lower_bound;
			upper_bound = (int32_t)(int16_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int16_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_w_5_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xffff;

		uint32_t ea = EA_AY_AI_16();
		int32_t lower_bound = m68ki_read_16(ea);
		int32_t upper_bound = m68ki_read_16(ea + 2);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x8000)
		{
			lower_bound = (int32_t)(int16_t)lower_bound;
			upper_bound = (int32_t)(int16_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int16_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_w_5_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xffff;

		uint32_t ea = EA_AY_DI_16();
		int32_t lower_bound = m68ki_read_16(ea);
		int32_t upper_bound = m68ki_read_16(ea + 2);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x8000)
		{
			lower_bound = (int32_t)(int16_t)lower_bound;
			upper_bound = (int32_t)(int16_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int16_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_w_5_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xffff;

		uint32_t ea = EA_AY_IX_16();
		int32_t lower_bound = m68ki_read_16(ea);
		int32_t upper_bound = m68ki_read_16(ea + 2);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x8000)
		{
			lower_bound = (int32_t)(int16_t)lower_bound;
			upper_bound = (int32_t)(int16_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int16_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_w_5_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xffff;

		uint32_t ea = EA_AW_16();
		int32_t lower_bound = m68ki_read_16(ea);
		int32_t upper_bound = m68ki_read_16(ea + 2);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x8000)
		{
			lower_bound = (int32_t)(int16_t)lower_bound;
			upper_bound = (int32_t)(int16_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int16_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_w_5_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int32_t compare = (int32_t)REG_DA()[(word2 >> 12) & 15];
		if(!BIT_F(word2))
			compare &= 0xffff;

		uint32_t ea = EA_AL_16();
		int32_t lower_bound = m68ki_read_16(ea);
		int32_t upper_bound = m68ki_read_16(ea + 2);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x8000)
		{
			lower_bound = (int32_t)(int16_t)lower_bound;
			upper_bound = (int32_t)(int16_t)upper_bound;

			if(!BIT_F(word2))
				compare = (int32_t)(int16_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_l_6()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int64_t compare = REG_DA()[(word2 >> 12) & 15];
		uint32_t ea = EA_PCDI_32();
		int64_t lower_bound = m68ki_read_pcrel_32(ea);
		int64_t upper_bound = m68ki_read_pcrel_32(ea + 4);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80000000)
		{
			lower_bound = (int64_t)(int32_t)lower_bound;
			upper_bound = (int64_t)(int32_t)upper_bound;
			compare = (int64_t)(int32_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_l_7()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int64_t compare = REG_DA()[(word2 >> 12) & 15];
		uint32_t ea = EA_PCIX_32();
		int64_t lower_bound = m68ki_read_pcrel_32(ea);
		int64_t upper_bound = m68ki_read_pcrel_32(ea + 4);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80000000)
		{
			lower_bound = (int64_t)(int32_t)lower_bound;
			upper_bound = (int64_t)(int32_t)upper_bound;
			compare = (int64_t)(int32_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_l_8_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int64_t compare = REG_DA()[(word2 >> 12) & 15];
		uint32_t ea = EA_AY_AI_32();
		int64_t lower_bound = m68ki_read_32(ea);
		int64_t upper_bound = m68ki_read_32(ea + 4);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80000000)
		{
			lower_bound = (int64_t)(int32_t)lower_bound;
			upper_bound = (int64_t)(int32_t)upper_bound;
			compare = (int64_t)(int32_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_l_8_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int64_t compare = REG_DA()[(word2 >> 12) & 15];
		uint32_t ea = EA_AY_DI_32();
		int64_t lower_bound = m68ki_read_32(ea);
		int64_t upper_bound = m68ki_read_32(ea + 4);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80000000)
		{
			lower_bound = (int64_t)(int32_t)lower_bound;
			upper_bound = (int64_t)(int32_t)upper_bound;
			compare = (int64_t)(int32_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_l_8_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int64_t compare = REG_DA()[(word2 >> 12) & 15];
		uint32_t ea = EA_AY_IX_32();
		int64_t lower_bound = m68ki_read_32(ea);
		int64_t upper_bound = m68ki_read_32(ea + 4);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80000000)
		{
			lower_bound = (int64_t)(int32_t)lower_bound;
			upper_bound = (int64_t)(int32_t)upper_bound;
			compare = (int64_t)(int32_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_l_8_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int64_t compare = REG_DA()[(word2 >> 12) & 15];
		uint32_t ea = EA_AW_32();
		int64_t lower_bound = m68ki_read_32(ea);
		int64_t upper_bound = m68ki_read_32(ea + 4);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80000000)
		{
			lower_bound = (int64_t)(int32_t)lower_bound;
			upper_bound = (int64_t)(int32_t)upper_bound;
			compare = (int64_t)(int32_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_chk2cmp2_l_8_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		int64_t compare = REG_DA()[(word2 >> 12) & 15];
		uint32_t ea = EA_AL_32();
		int64_t lower_bound = m68ki_read_32(ea);
		int64_t upper_bound = m68ki_read_32(ea + 4);

		// for signed compare, the arithmetically smaller value is the lower bound
		if (lower_bound & 0x80000000)
		{
			lower_bound = (int64_t)(int32_t)lower_bound;
			upper_bound = (int64_t)(int32_t)upper_bound;
			compare = (int64_t)(int32_t)compare;
		}

		m_c_flag = (compare >= lower_bound && compare <= upper_bound) ? CFLAG_CLEAR : CFLAG_SET;
		m_not_z_flag = ((upper_bound == compare) || (lower_bound == compare)) ? 0 : 1;

		if(COND_CS() && BIT_B(word2))
			m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_clr_b_0()
{
	DY() &= 0xffffff00;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_aw()
{
	uint32_t ea = EA_AW_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_b_1_al()
{
	uint32_t ea = EA_AL_8();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_8(ea);   /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_8(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_w_2()
{
	DY() &= 0xffff0000;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_w_3_ai()
{
	uint32_t ea = EA_AY_AI_16();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_16(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_w_3_pi()
{
	uint32_t ea = EA_AY_PI_16();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_16(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_w_3_pd()
{
	uint32_t ea = EA_AY_PD_16();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_16(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_w_3_di()
{
	uint32_t ea = EA_AY_DI_16();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_16(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_w_3_ix()
{
	uint32_t ea = EA_AY_IX_16();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_16(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_w_3_aw()
{
	uint32_t ea = EA_AW_16();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_16(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_w_3_al()
{
	uint32_t ea = EA_AL_16();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_16(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_16(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_l_4()
{
	DY() = 0;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_l_5_ai()
{
	uint32_t ea = EA_AY_AI_32();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_l_5_pi()
{
	uint32_t ea = EA_AY_PI_32();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_l_5_pd()
{
	uint32_t ea = EA_AY_PD_32();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_l_5_di()
{
	uint32_t ea = EA_AY_DI_32();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_l_5_ix()
{
	uint32_t ea = EA_AY_IX_32();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_l_5_aw()
{
	uint32_t ea = EA_AW_32();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_clr_l_5_al()
{
	uint32_t ea = EA_AL_32();

	if(CPU_TYPE_IS_000())
	{
		m68ki_read_32(ea);  /* the 68000 does a dummy read, the value is discarded */
	}

	m68ki_write_32(ea, 0);

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;


}
void m68000_base_device::m68k_op_cmp_b_0()
{
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_ai()
{
	uint32_t src = OPER_AY_AI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_pi()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_pi7()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_pd()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_pd7()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_di()
{
	uint32_t src = OPER_AY_DI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_ix()
{
	uint32_t src = OPER_AY_IX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_aw()
{
	uint32_t src = OPER_AW_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_al()
{
	uint32_t src = OPER_AL_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_pcdi()
{
	uint32_t src = OPER_PCDI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_pcix()
{
	uint32_t src = OPER_PCIX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_b_1_i()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmp_w_2()
{
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_3()
{
	uint32_t src = MASK_OUT_ABOVE_16(AY());
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_ai()
{
	uint32_t src = OPER_AY_AI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_pi()
{
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_pd()
{
	uint32_t src = OPER_AY_PD_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_di()
{
	uint32_t src = OPER_AY_DI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_ix()
{
	uint32_t src = OPER_AY_IX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_aw()
{
	uint32_t src = OPER_AW_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_al()
{
	uint32_t src = OPER_AL_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_pcdi()
{
	uint32_t src = OPER_PCDI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_pcix()
{
	uint32_t src = OPER_PCIX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_w_4_i()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmp_l_5()
{
	uint32_t src = DY();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_6()
{
	uint32_t src = AY();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_ai()
{
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_pi()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_pd()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_di()
{
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_ix()
{
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_aw()
{
	uint32_t src = OPER_AW_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_al()
{
	uint32_t src = OPER_AL_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_pcdi()
{
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_pcix()
{
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmp_l_7_i()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_0()
{
	uint32_t src = MAKE_INT_16(DY());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_1()
{
	uint32_t src = MAKE_INT_16(AY());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_ai()
{
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_pi()
{
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_pd()
{
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_di()
{
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_ix()
{
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_aw()
{
	uint32_t src = MAKE_INT_16(OPER_AW_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_al()
{
	uint32_t src = MAKE_INT_16(OPER_AL_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_pcdi()
{
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_pcix()
{
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_w_2_i()
{
	uint32_t src = MAKE_INT_16(OPER_I_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_3()
{
	uint32_t src = DY();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_4()
{
	uint32_t src = AY();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_ai()
{
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_pi()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_pd()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_di()
{
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_ix()
{
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_aw()
{
	uint32_t src = OPER_AW_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_al()
{
	uint32_t src = OPER_AL_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_pcdi()
{
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_pcix()
{
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpa_l_5_i()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_b_0()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DY());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_ai()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_AI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_pi()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_pi7()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_pd()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_PD_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_pd7()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_A7_PD_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_di()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_DI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_ix()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_IX_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_aw()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AW_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_1_al()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AL_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpi_b_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t src = OPER_I_8();
		uint32_t dst = OPER_PCDI_8();
		uint32_t res = dst - src;

		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(src, dst, res);
		m_c_flag = CFLAG_8(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cmpi_b_3()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t src = OPER_I_8();
		uint32_t dst = OPER_PCIX_8();
		uint32_t res = dst - src;

		m_n_flag = NFLAG_8(res);
		m_not_z_flag = MASK_OUT_ABOVE_8(res);
		m_v_flag = VFLAG_SUB_8(src, dst, res);
		m_c_flag = CFLAG_8(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cmpi_w_4()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DY());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpi_w_5_ai()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_AI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpi_w_5_pi()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_PI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpi_w_5_pd()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_PD_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpi_w_5_di()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_DI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpi_w_5_ix()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_IX_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpi_w_5_aw()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AW_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpi_w_5_al()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AL_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpi_w_6()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t src = OPER_I_16();
		uint32_t dst = OPER_PCDI_16();
		uint32_t res = dst - src;

		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(src, dst, res);
		m_c_flag = CFLAG_16(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cmpi_w_7()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t src = OPER_I_16();
		uint32_t dst = OPER_PCIX_16();
		uint32_t res = dst - src;

		m_n_flag = NFLAG_16(res);
		m_not_z_flag = MASK_OUT_ABOVE_16(res);
		m_v_flag = VFLAG_SUB_16(src, dst, res);
		m_c_flag = CFLAG_16(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cmpi_l_8()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = DY();
	uint32_t res = dst - src;

	if (!m_cmpild_instr_callback.isnull())
		(m_cmpild_instr_callback)(*m_program, m_ir & 7, src, 0xffffffff);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_l_9_ai()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_AI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_l_9_pi()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_PI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_l_9_pd()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_PD_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_l_9_di()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_DI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_l_9_ix()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_IX_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_l_9_aw()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AW_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_l_9_al()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AL_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cmpi_l_10()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t src = OPER_I_32();
		uint32_t dst = OPER_PCDI_32();
		uint32_t res = dst - src;

		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(src, dst, res);
		m_c_flag = CFLAG_SUB_32(src, dst, res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cmpi_l_11()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t src = OPER_I_32();
		uint32_t dst = OPER_PCIX_32();
		uint32_t res = dst - src;

		m_n_flag = NFLAG_32(res);
		m_not_z_flag = MASK_OUT_ABOVE_32(res);
		m_v_flag = VFLAG_SUB_32(src, dst, res);
		m_c_flag = CFLAG_SUB_32(src, dst, res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cmpm_b_0()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpm_b_1()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = OPER_AX_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpm_b_2()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpm_b_3()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = OPER_AX_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);


}
void m68000_base_device::m68k_op_cmpm_w_4()
{
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = OPER_AX_PI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);


}
void m68000_base_device::m68k_op_cmpm_l_5()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = OPER_AX_PI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);


}
void m68000_base_device::m68k_op_cpbcc_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpbcc)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_cpdbcc_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpdbcc)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_cpgen_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS() && (m_has_fpu || m_has_pmmu))
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpgen)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_cpscc_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpscc)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_cptrapcc_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cptrapcc)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_ftrapcc_l_0()
{
	if(m_has_fpu)
	{
		m68881_ftrap();
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_dbt_w_0()
{
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbf_w_0()
{
	uint32_t* r_dst = &DY();
	uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
	if(res != 0xffff)
	{
		uint32_t offset = OPER_I_16();
		m_pc -= 2;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		m_icount -= m_cyc_dbcc_f_noexp;
		return;
	}
	m_pc += 2;
	m_icount -= m_cyc_dbcc_f_exp;


}
void m68000_base_device::m68k_op_dbhi_w_0()
{
	if(COND_NOT_HI())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbls_w_0()
{
	if(COND_NOT_LS())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbcc_w_1()
{
	if(COND_NOT_CC())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbcs_w_0()
{
	if(COND_NOT_CS())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbne_w_0()
{
	if(COND_NOT_NE())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbeq_w_0()
{
	if(COND_NOT_EQ())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbvc_w_0()
{
	if(COND_NOT_VC())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbvs_w_0()
{
	if(COND_NOT_VS())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbpl_w_0()
{
	if(COND_NOT_PL())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbmi_w_0()
{
	if(COND_NOT_MI())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbge_w_0()
{
	if(COND_NOT_GE())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dblt_w_0()
{
	if(COND_NOT_LT())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dbgt_w_0()
{
	if(COND_NOT_GT())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_dble_w_0()
{
	if(COND_NOT_LE())
	{
		uint32_t* r_dst = &DY();
		uint32_t res = MASK_OUT_ABOVE_16(*r_dst - 1);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		if(res != 0xffff)
		{
			uint32_t offset = OPER_I_16();
			m_pc -= 2;
			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			m68ki_branch_16(offset);
			m_icount -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_icount -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;


}
void m68000_base_device::m68k_op_divs_w_0()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(DY());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_ai()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_AI_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_pi()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_PI_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_pd()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_PD_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_di()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_DI_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_ix()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AY_IX_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_aw()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AW_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_al()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_AL_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_pcdi()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_PCDI_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_pcix()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_PCIX_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divs_w_1_i()
{
	uint32_t* r_dst = &DX();
	int32_t src = MAKE_INT_16(OPER_I_16());
	int32_t quotient;
	int32_t remainder;

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		if((uint32_t)*r_dst == 0x80000000 && src == -1)
		{
			m_not_z_flag = 0;
			m_n_flag = NFLAG_CLEAR;
			m_v_flag = VFLAG_CLEAR;
			*r_dst = 0;
			return;
		}

		quotient = MAKE_INT_32(*r_dst) / src;
		remainder = MAKE_INT_32(*r_dst) % src;

		if(quotient == MAKE_INT_16(quotient))
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_0()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_di()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_al()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divu_w_1_i()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_16();

	if(src != 0)
	{
		m_c_flag = CFLAG_CLEAR;
		uint32_t quotient = *r_dst / src;
		uint32_t remainder = *r_dst % src;

		if(quotient < 0x10000)
		{
			m_not_z_flag = quotient;
			m_n_flag = NFLAG_16(quotient);
			m_v_flag = VFLAG_CLEAR;
			*r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
			return;
		}
		m_v_flag = VFLAG_SET;
		return;
	}
	m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);


}
void m68000_base_device::m68k_op_divl_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor   = DY();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_AY_AI_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_pi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_AY_PI_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_pd()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_AY_PD_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_AY_DI_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_AY_IX_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_AW_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_AL_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_pcdi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_PCDI_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_pcix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_PCIX_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_divl_l_1_i()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t divisor = OPER_I_32();
		uint64_t dividend  = 0;
		uint64_t quotient  = 0;
		uint64_t remainder = 0;

		if(divisor != 0)
		{
			if(BIT_A(word2))    /* 64 bit */
			{
				dividend = REG_D()[word2 & 7];
				dividend <<= 32;
				dividend |= REG_D()[(word2 >> 12) & 7];

				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)dividend / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)dividend % (int64_t)((int32_t)divisor));
					if((int64_t)quotient != (int64_t)((int32_t)quotient))
					{
						m_v_flag = VFLAG_SET;
						return;
					}
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					if(quotient > 0xffffffff)
					{
						m_v_flag = VFLAG_SET;
						return;
					}
					remainder = dividend % divisor;
				}
			}
			else    /* 32 bit */
			{
				dividend = REG_D()[(word2 >> 12) & 7];
				if(BIT_B(word2))       /* signed */
				{
					quotient  = (uint64_t)((int64_t)((int32_t)dividend) / (int64_t)((int32_t)divisor));
					remainder = (uint64_t)((int64_t)((int32_t)dividend) % (int64_t)((int32_t)divisor));
				}
				else                    /* unsigned */
				{
					quotient = dividend / divisor;
					remainder = dividend % divisor;
				}
			}

			REG_D()[word2 & 7] = remainder;
			REG_D()[(word2 >> 12) & 7] = quotient;

			m_n_flag = NFLAG_32(quotient);
			m_not_z_flag = quotient;
			m_v_flag = VFLAG_CLEAR;
			m_c_flag = CFLAG_CLEAR;
			return;
		}
		m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_eor_b_0()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY() ^= MASK_OUT_ABOVE_8(DX()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_w_2()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() ^= MASK_OUT_ABOVE_16(DX()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_w_3_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_w_3_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_w_3_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_w_3_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_w_3_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_w_3_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_w_3_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_l_4()
{
	uint32_t res = DY() ^= DX();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_l_5_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_l_5_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_l_5_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_l_5_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_l_5_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_l_5_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eor_l_5_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_0()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY() ^= OPER_I_8());

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_ai()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_pi()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_pi7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_pd()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_pd7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_di()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_ix()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_aw()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_b_1_al()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t res = src ^ m68ki_read_8(ea);

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_2()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() ^= OPER_I_16());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_3_ai()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_3_pi()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_3_pd()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_3_di()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_3_ix()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_3_aw()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_3_al()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t res = src ^ m68ki_read_16(ea);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_l_4()
{
	uint32_t res = DY() ^= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_l_5_ai()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_l_5_pi()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_l_5_pd()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_l_5_di()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_l_5_ix()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_l_5_aw()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_l_5_al()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t res = src ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_eori_w_6()
{
	m68ki_set_ccr(m68ki_get_ccr() ^ OPER_I_8());


}
void m68000_base_device::m68k_op_eori_w_7()
{
	if(m_s_flag)
	{
		uint32_t src = OPER_I_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() ^ src);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_exg_l_0()
{
	uint32_t* reg_a = &DX();
	uint32_t* reg_b = &DY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;


}
void m68000_base_device::m68k_op_exg_l_1()
{
	uint32_t* reg_a = &AX();
	uint32_t* reg_b = &AY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;


}
void m68000_base_device::m68k_op_exg_l_2()
{
	uint32_t* reg_a = &DX();
	uint32_t* reg_b = &AY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;


}
void m68000_base_device::m68k_op_ext_w_0()
{
	uint32_t* r_dst = &DY();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | MASK_OUT_ABOVE_8(*r_dst) | (GET_MSB_8(*r_dst) ? 0xff00 : 0);

	m_n_flag = NFLAG_16(*r_dst);
	m_not_z_flag = MASK_OUT_ABOVE_16(*r_dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ext_l_1()
{
	uint32_t* r_dst = &DY();

	*r_dst = MASK_OUT_ABOVE_16(*r_dst) | (GET_MSB_16(*r_dst) ? 0xffff0000 : 0);

	m_n_flag = NFLAG_32(*r_dst);
	m_not_z_flag = *r_dst;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_extb_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t* r_dst = &DY();

		*r_dst = MASK_OUT_ABOVE_8(*r_dst) | (GET_MSB_8(*r_dst) ? 0xffffff00 : 0);

		m_n_flag = NFLAG_32(*r_dst);
		m_not_z_flag = *r_dst;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_illegal_0()
{
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_jmp_l_0_ai()
{
	m68ki_jump(EA_AY_AI_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_jmp_l_0_di()
{
	m68ki_jump(EA_AY_DI_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_jmp_l_0_ix()
{
	m68ki_jump(EA_AY_IX_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_jmp_l_0_aw()
{
	m68ki_jump(EA_AW_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_jmp_l_0_al()
{
	m68ki_jump(EA_AL_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_jmp_l_0_pcdi()
{
	m68ki_jump(EA_PCDI_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_jmp_l_0_pcix()
{
	m68ki_jump(EA_PCIX_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_icount > 0)
		m_icount = 0;


}
void m68000_base_device::m68k_op_jsr_l_0_ai()
{
	uint32_t ea = EA_AY_AI_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);


}
void m68000_base_device::m68k_op_jsr_l_0_di()
{
	uint32_t ea = EA_AY_DI_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);


}
void m68000_base_device::m68k_op_jsr_l_0_ix()
{
	uint32_t ea = EA_AY_IX_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);


}
void m68000_base_device::m68k_op_jsr_l_0_aw()
{
	uint32_t ea = EA_AW_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);


}
void m68000_base_device::m68k_op_jsr_l_0_al()
{
	uint32_t ea = EA_AL_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);


}
void m68000_base_device::m68k_op_jsr_l_0_pcdi()
{
	uint32_t ea = EA_PCDI_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);


}
void m68000_base_device::m68k_op_jsr_l_0_pcix()
{
	uint32_t ea = EA_PCIX_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);


}
void m68000_base_device::m68k_op_lea_l_0_ai()
{
	AX() = EA_AY_AI_32();


}
void m68000_base_device::m68k_op_lea_l_0_di()
{
	AX() = EA_AY_DI_32();


}
void m68000_base_device::m68k_op_lea_l_0_ix()
{
	AX() = EA_AY_IX_32();


}
void m68000_base_device::m68k_op_lea_l_0_aw()
{
	AX() = EA_AW_32();


}
void m68000_base_device::m68k_op_lea_l_0_al()
{
	AX() = EA_AL_32();


}
void m68000_base_device::m68k_op_lea_l_0_pcdi()
{
	AX() = EA_PCDI_32();


}
void m68000_base_device::m68k_op_lea_l_0_pcix()
{
	AX() = EA_PCIX_32();


}
void m68000_base_device::m68k_op_link_w_0()
{
	REG_A()[7] -= 4;
	m68ki_write_32(REG_A()[7], REG_A()[7]);
	REG_A()[7] = MASK_OUT_ABOVE_32(REG_A()[7] + MAKE_INT_16(OPER_I_16()));


}
void m68000_base_device::m68k_op_link_w_1()
{
	uint32_t* r_dst = &AY();

	m68ki_push_32(*r_dst);
	*r_dst = REG_A()[7];
	REG_A()[7] = MASK_OUT_ABOVE_32(REG_A()[7] + MAKE_INT_16(OPER_I_16()));


}
void m68000_base_device::m68k_op_link_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		REG_A()[7] -= 4;
		m68ki_write_32(REG_A()[7], REG_A()[7]);
		REG_A()[7] = MASK_OUT_ABOVE_32(REG_A()[7] + OPER_I_32());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_link_l_3()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t* r_dst = &AY();

		m68ki_push_32(*r_dst);
		*r_dst = REG_A()[7];
		REG_A()[7] = MASK_OUT_ABOVE_32(REG_A()[7] + OPER_I_32());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_lsr_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_1()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_b_3()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift <= 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			m_x_flag = m_c_flag = src << (9-shift);
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		m_x_flag = XFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_8(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_4()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift <= 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			m_c_flag = m_x_flag = (src >> (shift - 1))<<8;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		m_x_flag = XFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_16(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_l_5()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift < 32)
		{
			*r_dst = res;
			m_c_flag = m_x_flag = (src >> (shift - 1))<<8;
			m_n_flag = NFLAG_CLEAR;
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		m_x_flag = m_c_flag = (shift == 32 ? GET_MSB_32(src)>>23 : 0);
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_32(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_6_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_6_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_6_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_6_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_6_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_6_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsr_w_6_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = src >> 1;

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_c_flag = m_x_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << shift;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_1()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (8-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (24-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_b_3()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift <= 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			m_x_flag = m_c_flag = src << shift;
			m_n_flag = NFLAG_8(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		m_x_flag = XFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_8(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_4()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift <= 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			m_x_flag = m_c_flag = (src << shift) >> 8;
			m_n_flag = NFLAG_16(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		m_x_flag = XFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_16(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_l_5()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		m_icount -= shift<<m_cyc_shift;

		if(shift < 32)
		{
			*r_dst = res;
			m_x_flag = m_c_flag = (src >> (32 - shift)) << 8;
			m_n_flag = NFLAG_32(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		m_x_flag = m_c_flag = ((shift == 32 ? src & 1 : 0))<<8;
		m_n_flag = NFLAG_CLEAR;
		m_not_z_flag = ZFLAG_SET;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_32(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_6_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_6_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_6_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_6_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_6_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_6_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_lsl_w_6_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_0()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_1_i()
{
	uint32_t res = OPER_I_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_2()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_3_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_4()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_5()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_6_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_7_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_8()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_9()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_10_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_11_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_12()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_13_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_14()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_15_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_16()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_17_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_18()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_b_19_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_20()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_21()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_22_i()
{
	uint32_t res = OPER_I_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_23()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_24()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_25_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_26()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_27()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_28_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_29()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_30()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_31_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_32()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_33()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_34_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_35()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_36()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_37_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_38()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_39()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_40_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_41()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_42()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_w_43_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_44()
{
	uint32_t res = DY();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_45()
{
	uint32_t res = AY();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_46_i()
{
	uint32_t res = OPER_I_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_47()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_48()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_49_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_50()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_51()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_52_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_53()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_54()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_55_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_PD_32();

	m68ki_write_16(ea+2, res & 0xFFFF );
	m68ki_write_16(ea, (res >> 16) & 0xFFFF );

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_56()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_57()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_58_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_59()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_60()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_61_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_62()
{
	uint32_t res = DY();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_63()
{
	uint32_t res = AY();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_64_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_65()
{
	uint32_t res = DY();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_66()
{
	uint32_t res = AY();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move_l_67_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_movea_w_0()
{
	AX() = MAKE_INT_16(DY());


}
void m68000_base_device::m68k_op_movea_w_1()
{
	AX() = MAKE_INT_16(AY());


}
void m68000_base_device::m68k_op_movea_w_2_ai()
{
	AX() = MAKE_INT_16(OPER_AY_AI_16());


}
void m68000_base_device::m68k_op_movea_w_2_pi()
{
	AX() = MAKE_INT_16(OPER_AY_PI_16());


}
void m68000_base_device::m68k_op_movea_w_2_pd()
{
	AX() = MAKE_INT_16(OPER_AY_PD_16());


}
void m68000_base_device::m68k_op_movea_w_2_di()
{
	AX() = MAKE_INT_16(OPER_AY_DI_16());


}
void m68000_base_device::m68k_op_movea_w_2_ix()
{
	AX() = MAKE_INT_16(OPER_AY_IX_16());


}
void m68000_base_device::m68k_op_movea_w_2_aw()
{
	AX() = MAKE_INT_16(OPER_AW_16());


}
void m68000_base_device::m68k_op_movea_w_2_al()
{
	AX() = MAKE_INT_16(OPER_AL_16());


}
void m68000_base_device::m68k_op_movea_w_2_pcdi()
{
	AX() = MAKE_INT_16(OPER_PCDI_16());


}
void m68000_base_device::m68k_op_movea_w_2_pcix()
{
	AX() = MAKE_INT_16(OPER_PCIX_16());


}
void m68000_base_device::m68k_op_movea_w_2_i()
{
	AX() = MAKE_INT_16(OPER_I_16());


}
void m68000_base_device::m68k_op_movea_l_3()
{
	AX() = DY();


}
void m68000_base_device::m68k_op_movea_l_4()
{
	AX() = AY();


}
void m68000_base_device::m68k_op_movea_l_5_ai()
{
	AX() = OPER_AY_AI_32();


}
void m68000_base_device::m68k_op_movea_l_5_pi()
{
	AX() = OPER_AY_PI_32();


}
void m68000_base_device::m68k_op_movea_l_5_pd()
{
	AX() = OPER_AY_PD_32();


}
void m68000_base_device::m68k_op_movea_l_5_di()
{
	AX() = OPER_AY_DI_32();


}
void m68000_base_device::m68k_op_movea_l_5_ix()
{
	AX() = OPER_AY_IX_32();


}
void m68000_base_device::m68k_op_movea_l_5_aw()
{
	AX() = OPER_AW_32();


}
void m68000_base_device::m68k_op_movea_l_5_al()
{
	AX() = OPER_AL_32();


}
void m68000_base_device::m68k_op_movea_l_5_pcdi()
{
	AX() = OPER_PCDI_32();


}
void m68000_base_device::m68k_op_movea_l_5_pcix()
{
	AX() = OPER_PCIX_32();


}
void m68000_base_device::m68k_op_movea_l_5_i()
{
	AX() = OPER_I_32();


}
void m68000_base_device::m68k_op_move_w_68()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		DY() = MASK_OUT_BELOW_16(DY()) | m68ki_get_ccr();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_move_w_69_ai()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_AI_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_move_w_69_pi()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_PI_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_move_w_69_pd()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_PD_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_move_w_69_di()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_DI_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_move_w_69_ix()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_IX_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_move_w_69_aw()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AW_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_move_w_69_al()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AL_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_move_w_70()
{
	m68ki_set_ccr(DY());


}
void m68000_base_device::m68k_op_move_w_71_ai()
{
	m68ki_set_ccr(OPER_AY_AI_16());


}
void m68000_base_device::m68k_op_move_w_71_pi()
{
	m68ki_set_ccr(OPER_AY_PI_16());


}
void m68000_base_device::m68k_op_move_w_71_pd()
{
	m68ki_set_ccr(OPER_AY_PD_16());


}
void m68000_base_device::m68k_op_move_w_71_di()
{
	m68ki_set_ccr(OPER_AY_DI_16());


}
void m68000_base_device::m68k_op_move_w_71_ix()
{
	m68ki_set_ccr(OPER_AY_IX_16());


}
void m68000_base_device::m68k_op_move_w_71_aw()
{
	m68ki_set_ccr(OPER_AW_16());


}
void m68000_base_device::m68k_op_move_w_71_al()
{
	m68ki_set_ccr(OPER_AL_16());


}
void m68000_base_device::m68k_op_move_w_71_pcdi()
{
	m68ki_set_ccr(OPER_PCDI_16());


}
void m68000_base_device::m68k_op_move_w_71_pcix()
{
	m68ki_set_ccr(OPER_PCIX_16());


}
void m68000_base_device::m68k_op_move_w_71_i()
{
	m68ki_set_ccr(OPER_I_16());


}
void m68000_base_device::m68k_op_move_w_72()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		DY() = MASK_OUT_BELOW_16(DY()) | m68ki_get_sr();
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_73_ai()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_AI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_73_pi()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_PI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_73_pd()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_PD_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_73_di()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_DI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_73_ix()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_IX_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_73_aw()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AW_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_73_al()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AL_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_74()
{
	if(m_s_flag)
	{
		m68ki_set_sr(DY());
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_ai()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_AY_AI_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_pi()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_AY_PI_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_pd()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_AY_PD_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_di()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_AY_DI_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_ix()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_AY_IX_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_aw()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_AW_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_al()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_AL_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_pcdi()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_PCDI_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_pcix()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_PCIX_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_w_75_i()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_I_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(new_sr);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_l_76()
{
	if(m_s_flag)
	{
		AY() = REG_USP();
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_move_l_77()
{
	if(m_s_flag)
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		REG_USP() = AY();
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_movec_l_0()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();

			m68ki_trace_t0();          /* auto-disable (see m68kcpu.h) */
			switch (word2 & 0xfff)
			{
			case 0x000:            /* SFC */
				REG_DA()[(word2 >> 12) & 15] = m_sfc;
				return;
			case 0x001:            /* DFC */
				REG_DA()[(word2 >> 12) & 15] = m_dfc;
				return;
			case 0x002:            /* CACR */
				if(CPU_TYPE_IS_EC020_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_cacr;
					return;
				}
				return;
			case 0x800:            /* USP */
				REG_DA()[(word2 >> 12) & 15] = REG_USP();
				return;
			case 0x801:            /* VBR */
				REG_DA()[(word2 >> 12) & 15] = m_vbr;
				return;
			case 0x802:            /* CAAR */
				if(CPU_TYPE_IS_EC020_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_caar;
					return;
				}
				m68ki_exception_illegal();
				break;
			case 0x803:            /* MSP */
				if(CPU_TYPE_IS_EC020_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_m_flag ? REG_SP() : REG_MSP();
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x804:            /* ISP */
				if(CPU_TYPE_IS_EC020_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_m_flag ? REG_ISP() : REG_SP();
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x003:             /* TC */
				if(CPU_TYPE_IS_040_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_tc;
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x004:             /* ITT0 (040+, ACR0 on ColdFire) */
				if(CPU_TYPE_IS_040_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_itt0;
					return;
				}
				else if(CPU_TYPE_IS_COLDFIRE())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_acr0;
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x005:             /* ITT1 (040+, ACR1 on ColdFire) */
				if(CPU_TYPE_IS_040_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_itt1;
					return;
				}
				else if(CPU_TYPE_IS_COLDFIRE())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_acr1;
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x006:             /* DTT0 */
				if(CPU_TYPE_IS_040_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_dtt0;
					return;
				}
				else if(CPU_TYPE_IS_COLDFIRE())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_acr2;
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x007:             /* DTT1 */
				if(CPU_TYPE_IS_040_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_dtt1;
					return;
				}
				else if(CPU_TYPE_IS_COLDFIRE())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_acr3;
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x805:             /* MMUSR */
				if(CPU_TYPE_IS_040_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_sr_040;
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x806:             /* URP */
				if(CPU_TYPE_IS_040_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_urp_aptr;
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x807:             /* SRP */
				if(CPU_TYPE_IS_040_PLUS())
				{
					REG_DA()[(word2 >> 12) & 15] = m_mmu_srp_aptr;
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc00: // ROMBAR0
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc01: // ROMBAR1
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc04: // RAMBAR0
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc05: // RAMBAR1
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc0c: // MPCR
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc0d: // EDRAMBAR
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc0e: // SECMBAR
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc0f: // MBAR
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			default:
				m68ki_exception_illegal();
				return;
			}
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_movec_l_1()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();

			m68ki_trace_t0();          /* auto-disable (see m68kcpu.h) */
			switch (word2 & 0xfff)
			{
			case 0x000:            /* SFC */
				m_sfc = REG_DA()[(word2 >> 12) & 15] & 7;
				return;
			case 0x001:            /* DFC */
				m_dfc = REG_DA()[(word2 >> 12) & 15] & 7;
				return;
			case 0x002:            /* CACR */
				/* Only EC020 and later have CACR */
				if(CPU_TYPE_IS_EC020_PLUS())
				{
					/* 68030 can write all bits except 5-7, 040 can write all */
					if (CPU_TYPE_IS_040_PLUS())
					{
						m_cacr = REG_DA()[(word2 >> 12) & 15];
					}
					else if (CPU_TYPE_IS_030_PLUS())
					{
						m_cacr = REG_DA()[(word2 >> 12) & 15] & 0xff1f;
					}
					else
					{
						m_cacr = REG_DA()[(word2 >> 12) & 15] & 0x0f;
					}

                    // logerror("movec to cacr=%04x\n", m_cacr);
					if (m_cacr & (M68K_CACR_CI | M68K_CACR_CEI))
					{
						m68ki_ic_clear();
					}
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x800:            /* USP */
				REG_USP() = REG_DA()[(word2 >> 12) & 15];
				return;
			case 0x801:            /* VBR */
				m_vbr = REG_DA()[(word2 >> 12) & 15];
				return;
			case 0x802:            /* CAAR */
				if(CPU_TYPE_IS_EC020_PLUS())
				{
					m_caar = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x803:            /* MSP */
				if(CPU_TYPE_IS_EC020_PLUS())
				{
					/* we are in supervisor mode so just check for M flag */
					if(!m_m_flag)
					{
						REG_MSP() = REG_DA()[(word2 >> 12) & 15];
						return;
					}
					REG_SP() = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x804:            /* ISP */
				if(CPU_TYPE_IS_EC020_PLUS())
				{
					if(!m_m_flag)
					{
						REG_SP() = REG_DA()[(word2 >> 12) & 15];
						return;
					}
					REG_ISP() = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x003:         /* TC */
				if (CPU_TYPE_IS_040_PLUS())
				{
					m_mmu_tc = REG_DA()[(word2 >> 12) & 15];

					if (m_mmu_tc & 0x8000)
					{
						m_pmmu_enabled = 1;
					}
					else
					{
						m_pmmu_enabled = 0;
					}
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x004:         /* ITT0 */
				if (CPU_TYPE_IS_040_PLUS())
				{
					m_mmu_itt0 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				else if(CPU_TYPE_IS_COLDFIRE())
				{
					m_mmu_acr0 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x005:         /* ITT1 */
				if (CPU_TYPE_IS_040_PLUS())
				{
					m_mmu_itt1 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				else if(CPU_TYPE_IS_COLDFIRE())
				{
					m_mmu_acr1 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x006:         /* DTT0 */
				if (CPU_TYPE_IS_040_PLUS())
				{
					m_mmu_dtt0 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				else if(CPU_TYPE_IS_COLDFIRE())
				{
					m_mmu_acr2 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x007:         /* DTT1 */
				if (CPU_TYPE_IS_040_PLUS())
				{
					m_mmu_dtt1 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				else if(CPU_TYPE_IS_COLDFIRE())
				{
					m_mmu_acr3 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x805:         /* MMUSR */
				if (CPU_TYPE_IS_040_PLUS())
				{
					m_mmu_sr_040 = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x806:         /* URP */
				if (CPU_TYPE_IS_040_PLUS())
				{
					m_mmu_urp_aptr = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0x807:         /* SRP */
				if (CPU_TYPE_IS_040_PLUS())
				{
					m_mmu_srp_aptr = REG_DA()[(word2 >> 12) & 15];
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc00: // ROMBAR0
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc01: // ROMBAR1
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc04: // RAMBAR0
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc05: // RAMBAR1
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc0c: // MPCR
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc0d: // EDRAMBAR
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc0e: // SECMBAR
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			case 0xc0f: // MBAR
				if(CPU_TYPE_IS_COLDFIRE())
				{
					/* TODO */
					return;
				}
				m68ki_exception_illegal();
				return;
			default:
				m68ki_exception_illegal();
				return;
			}
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_movem_w_0()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = AY();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			ea -= 2;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[15-i]));
			count++;
		}
	AY() = ea;

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_1_ai()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_1_di()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_1_ix()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_1_aw()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_1_al()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA()[i]));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_l_2()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = AY();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			ea -= 4;
			m68ki_write_16(ea+2, REG_DA()[15-i] & 0xFFFF );
			m68ki_write_16(ea, (REG_DA()[15-i] >> 16) & 0xFFFF );
			count++;
		}
	AY() = ea;

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_3_ai()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_AI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_3_di()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_DI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_3_ix()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_IX_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_3_aw()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AW_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_3_al()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AL_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			m68ki_write_32(ea, REG_DA()[i]);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_w_4()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = AY();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}
	AY() = ea;

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_5()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_PCDI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_pcrel_16(ea)));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_6()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_PCIX_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_pcrel_16(ea)));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_7_ai()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_7_di()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_7_ix()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_7_aw()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_w_7_al()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
			ea += 2;
			count++;
		}

	m_icount -= count<<m_cyc_movem_w;


}
void m68000_base_device::m68k_op_movem_l_8()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = AY();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}
	AY() = ea;

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_9()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_PCDI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = m68ki_read_pcrel_32(ea);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_10()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_PCIX_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = m68ki_read_pcrel_32(ea);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_11_ai()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_AI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_11_di()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_DI_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_11_ix()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AY_IX_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_11_aw()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AW_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movem_l_11_al()
{
	uint32_t i = 0;
	uint32_t register_list = OPER_I_16();
	uint32_t ea = EA_AL_32();
	uint32_t count = 0;

	for(; i < 16; i++)
		if(register_list & (1 << i))
		{
			REG_DA()[i] = m68ki_read_32(ea);
			ea += 4;
			count++;
		}

	m_icount -= count<<m_cyc_movem_l;


}
void m68000_base_device::m68k_op_movep_w_0()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = DX();

	m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 8));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));


}
void m68000_base_device::m68k_op_movep_l_1()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = DX();

	m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 24));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 16));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 8));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));


}
void m68000_base_device::m68k_op_movep_w_2()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | ((m68ki_read_8(ea) << 8) + m68ki_read_8(ea + 2));


}
void m68000_base_device::m68k_op_movep_l_3()
{
	uint32_t ea = EA_AY_DI_32();

	DX() = (m68ki_read_8(ea) << 24) + (m68ki_read_8(ea + 2) << 16)
		+ (m68ki_read_8(ea + 4) << 8) + m68ki_read_8(ea + 6);


}
void m68000_base_device::m68k_op_moves_b_0_ai()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_AI_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_b_0_pi()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_PI_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_b_0_pi7()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_A7_PI_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_b_0_pd()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_PD_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_b_0_pd7()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_A7_PD_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_b_0_di()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_DI_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_b_0_ix()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_IX_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_b_0_aw()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AW_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_b_0_al()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AL_8();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_8_fc(ea, m_dfc, MASK_OUT_ABOVE_8(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_8(m68ki_read_8_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_w_1_ai()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_AI_16();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_16_fc(ea, m_dfc, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(m68ki_read_16_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_w_1_pi()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_PI_16();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_16_fc(ea, m_dfc, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(m68ki_read_16_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_w_1_pd()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_PD_16();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_16_fc(ea, m_dfc, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(m68ki_read_16_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_w_1_di()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_DI_16();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_16_fc(ea, m_dfc, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(m68ki_read_16_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_w_1_ix()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_IX_16();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_16_fc(ea, m_dfc, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(m68ki_read_16_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_w_1_aw()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AW_16();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_16_fc(ea, m_dfc, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(m68ki_read_16_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_w_1_al()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AL_16();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_16_fc(ea, m_dfc, MASK_OUT_ABOVE_16(REG_DA()[(word2 >> 12) & 15]));
				return;
			}
			if(BIT_F(word2))           /* Memory to address register */
			{
				REG_A()[(word2 >> 12) & 7] = MAKE_INT_16(m68ki_read_16_fc(ea, m_sfc));
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_l_2_ai()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_AI_32();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_32_fc(ea, m_dfc, REG_DA()[(word2 >> 12) & 15]);
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_l_2_pi()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_PI_32();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_32_fc(ea, m_dfc, REG_DA()[(word2 >> 12) & 15]);
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_l_2_pd()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_PD_32();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_32_fc(ea, m_dfc, REG_DA()[(word2 >> 12) & 15]);
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_l_2_di()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_DI_32();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_32_fc(ea, m_dfc, REG_DA()[(word2 >> 12) & 15]);
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_l_2_ix()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AY_IX_32();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_32_fc(ea, m_dfc, REG_DA()[(word2 >> 12) & 15]);
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_l_2_aw()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AW_32();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_32_fc(ea, m_dfc, REG_DA()[(word2 >> 12) & 15]);
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moves_l_2_al()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if(m_s_flag)
		{
			uint32_t word2 = OPER_I_16();
			uint32_t ea = EA_AL_32();

			m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
			if(BIT_B(word2))           /* Register to memory */
			{
				m68ki_write_32_fc(ea, m_dfc, REG_DA()[(word2 >> 12) & 15]);
				if(CPU_TYPE_IS_020_VARIANT())
					m_icount -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_icount -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_moveq_l_0()
{
	uint32_t res = DX() = MAKE_INT_8(MASK_OUT_ABOVE_8(m_ir));

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_move16_l_0()
{
	uint16_t w2 = OPER_I_16();
	int ax = m_ir & 7;
	int ay = (w2 >> 12) & 7;
	m68ki_write_32(REG_A()[ay],    m68ki_read_32(REG_A()[ax]));
	m68ki_write_32(REG_A()[ay]+4,  m68ki_read_32(REG_A()[ax]+4));
	m68ki_write_32(REG_A()[ay]+8,  m68ki_read_32(REG_A()[ax]+8));
	m68ki_write_32(REG_A()[ay]+12, m68ki_read_32(REG_A()[ax]+12));

	REG_A()[ax] += 16;
	REG_A()[ay] += 16;


}
void m68000_base_device::m68k_op_muls_w_0()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(DY()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_AI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_PI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_PD_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_di()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_DI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_IX_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AW_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_al()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AL_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_PCDI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_PCIX_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_muls_w_1_i()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_I_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_0()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_16(DY()) * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_AI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_PI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_PD_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_di()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_DI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_IX_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AW_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_al()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AL_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_PCDI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_PCIX_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mulu_w_1_i()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_I_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_mull_l_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = DY();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_ai()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_AY_AI_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_pi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_AY_PI_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_pd()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_AY_PD_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_di()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_AY_DI_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_ix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_AY_IX_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_aw()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_AW_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_al()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_AL_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_pcdi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_PCDI_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_pcix()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_PCIX_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_mull_l_1_i()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t word2 = OPER_I_16();
		uint64_t src = OPER_I_32();
		uint64_t dst = REG_D()[(word2 >> 12) & 7];
		uint64_t res;

		m_c_flag = CFLAG_CLEAR;

		if(BIT_B(word2))               /* signed */
		{
			res = (int64_t)((int32_t)src) * (int64_t)((int32_t)dst);
			if(!BIT_A(word2))
			{
				m_not_z_flag = MASK_OUT_ABOVE_32(res);
				m_n_flag = NFLAG_32(res);
				m_v_flag = ((int64_t)res != (int32_t)res)<<7;
				REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
				return;
			}
			m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
			m_n_flag = NFLAG_64(res);
			m_v_flag = VFLAG_CLEAR;
			REG_D()[word2 & 7] = (res >> 32);
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
			return;
		}

		res = src * dst;
		if(!BIT_A(word2))
		{
			m_not_z_flag = MASK_OUT_ABOVE_32(res);
			m_n_flag = NFLAG_32(res);
			m_v_flag = (res > 0xffffffff)<<7;
			REG_D()[(word2 >> 12) & 7] = m_not_z_flag;
			return;
		}
		m_not_z_flag = MASK_OUT_ABOVE_32(res) | (res>>32);
		m_n_flag = NFLAG_64(res);
		m_v_flag = VFLAG_CLEAR;
		REG_D()[word2 & 7] = (res >> 32);
		REG_D()[(word2 >> 12) & 7] = MASK_OUT_ABOVE_32(res);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_nbcd_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_nbcd_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = -dst - XFLAG_1();

	if(res != 0)
	{
		m_v_flag = res; /* Undefined V behavior */

		if(((res|dst) & 0x0f) == 0)
			res = (res & 0xf0) | 6;

		res = MASK_OUT_ABOVE_8(res + 0x9a);

		m_v_flag &= ~res; /* Undefined V behavior part II */

		m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

		m_not_z_flag |= res;
		m_c_flag = CFLAG_SET;
		m_x_flag = XFLAG_SET;
	}
	else
	{
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		m_x_flag = XFLAG_CLEAR;
	}
	m_n_flag = NFLAG_8(res);  /* Undefined N behavior */


}
void m68000_base_device::m68k_op_neg_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_8(*r_dst);

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = *r_dst & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_neg_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = src & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_w_2()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_16(*r_dst);

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (*r_dst & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_neg_w_3_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_w_3_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_w_3_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_w_3_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_w_3_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_w_3_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_w_3_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_l_4()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - *r_dst;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(*r_dst, 0, res);
	m_v_flag = (*r_dst & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_neg_l_5_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_l_5_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_l_5_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_l_5_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_l_5_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_l_5_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_neg_l_5_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - src;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_negx_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_8(*r_dst) - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = *r_dst & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void m68000_base_device::m68k_op_negx_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t res = 0 - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = src & res;

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_negx_w_2()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_16(*r_dst) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (*r_dst & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;


}
void m68000_base_device::m68k_op_negx_w_3_ai()
{
	uint32_t ea  = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_negx_w_3_pi()
{
	uint32_t ea  = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_negx_w_3_pd()
{
	uint32_t ea  = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_negx_w_3_di()
{
	uint32_t ea  = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_negx_w_3_ix()
{
	uint32_t ea  = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_negx_w_3_aw()
{
	uint32_t ea  = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_negx_w_3_al()
{
	uint32_t ea  = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = (src & res)>>8;

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_negx_l_4()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_32(*r_dst) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(*r_dst, 0, res);
	m_v_flag = (*r_dst & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	*r_dst = res;


}
void m68000_base_device::m68k_op_negx_l_5_ai()
{
	uint32_t ea  = EA_AY_AI_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_negx_l_5_pi()
{
	uint32_t ea  = EA_AY_PI_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_negx_l_5_pd()
{
	uint32_t ea  = EA_AY_PD_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_negx_l_5_di()
{
	uint32_t ea  = EA_AY_DI_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_negx_l_5_ix()
{
	uint32_t ea  = EA_AY_IX_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_negx_l_5_aw()
{
	uint32_t ea  = EA_AW_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_negx_l_5_al()
{
	uint32_t ea  = EA_AL_32();
	uint32_t src = m68ki_read_32(ea);
	uint32_t res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, 0, res);
	m_v_flag = (src & res)>>24;

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_nop_0()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */


}
void m68000_base_device::m68k_op_not_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t res = MASK_OUT_ABOVE_8(~*r_dst);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_w_2()
{
	uint32_t* r_dst = &DY();
	uint32_t res = MASK_OUT_ABOVE_16(~*r_dst);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_w_3_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_w_3_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_w_3_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_w_3_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_w_3_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_w_3_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_w_3_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_l_4()
{
	uint32_t* r_dst = &DY();
	uint32_t res = *r_dst = MASK_OUT_ABOVE_32(~*r_dst);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_l_5_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_l_5_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_l_5_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_l_5_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_l_5_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_l_5_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_not_l_5_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_0()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= MASK_OUT_ABOVE_8(DY())));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_ai()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_AI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_pi()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_PI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_pi7()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_A7_PI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_pd()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_PD_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_pd7()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_A7_PD_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_di()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_DI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_ix()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_IX_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_aw()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AW_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_al()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AL_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_pcdi()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_PCDI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_pcix()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_PCIX_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_1_i()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_I_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_2()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= MASK_OUT_ABOVE_16(DY())));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_ai()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_AI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_pi()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_PI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_pd()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_PD_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_di()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_DI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_ix()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_IX_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_aw()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AW_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_al()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AL_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_pcdi()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_PCDI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_pcix()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_PCIX_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_3_i()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_I_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_4()
{
	uint32_t res = DX() |= DY();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_ai()
{
	uint32_t res = DX() |= OPER_AY_AI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_pi()
{
	uint32_t res = DX() |= OPER_AY_PI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_pd()
{
	uint32_t res = DX() |= OPER_AY_PD_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_di()
{
	uint32_t res = DX() |= OPER_AY_DI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_ix()
{
	uint32_t res = DX() |= OPER_AY_IX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_aw()
{
	uint32_t res = DX() |= OPER_AW_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_al()
{
	uint32_t res = DX() |= OPER_AL_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_pcdi()
{
	uint32_t res = DX() |= OPER_PCDI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_pcix()
{
	uint32_t res = DX() |= OPER_PCIX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_5_i()
{
	uint32_t res = DX() |= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_b_6_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_7_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_7_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_7_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_7_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_7_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_7_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_w_7_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_8_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_8_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_8_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_8_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_8_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_8_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_or_l_8_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_0()
{
	uint32_t res = MASK_OUT_ABOVE_8((DY() |= OPER_I_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_ai()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_pi()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_pi7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_pd()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_pd7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_di()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_ix()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_aw()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_b_1_al()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_2()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() |= OPER_I_16());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_3_ai()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_3_pi()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_3_pd()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_3_di()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_3_ix()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_3_aw()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_3_al()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_l_4()
{
	uint32_t res = DY() |= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_l_5_ai()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_l_5_pi()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_l_5_pd()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_l_5_di()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_l_5_ix()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_l_5_aw()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_l_5_al()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t res = src | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ori_w_6()
{
	m68ki_set_ccr(m68ki_get_ccr() | OPER_I_8());


}
void m68000_base_device::m68k_op_ori_w_7()
{
	if(m_s_flag)
	{
		uint32_t src = OPER_I_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() | src);
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_pack_w_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		/* Note: DX() and DY() are reversed in Motorola's docs */
		uint32_t src = DY() + OPER_I_16();
		uint32_t* r_dst = &DX();

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | ((src >> 4) & 0x00f0) | (src & 0x000f);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_pack_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint32_t ea_src = EA_AY_PD_8();
		uint32_t src = m68ki_read_8(ea_src);
		ea_src = EA_AY_PD_8();
		src = ((src << 8) | m68ki_read_8(ea_src)) + OPER_I_16();

		m68ki_write_8(EA_A7_PD_8(), ((src >> 8) & 0x000f) | ((src<<4) & 0x00f0));
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_pack_w_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint32_t ea_src = EA_A7_PD_8();
		uint32_t src = m68ki_read_8(ea_src);
		ea_src = EA_A7_PD_8();
		src = (src | (m68ki_read_8(ea_src) << 8)) + OPER_I_16();

		m68ki_write_8(EA_AX_PD_8(), ((src >> 4) & 0x00f0) | (src & 0x00f));
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_pack_w_3()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t ea_src = EA_A7_PD_8();
		uint32_t src = m68ki_read_8(ea_src);
		ea_src = EA_A7_PD_8();
		src = (src | (m68ki_read_8(ea_src) << 8)) + OPER_I_16();

		m68ki_write_8(EA_A7_PD_8(), ((src >> 4) & 0x00f0) | (src & 0x000f));
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_pack_w_4()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint32_t ea_src = EA_AY_PD_8();
		uint32_t src = m68ki_read_8(ea_src);
		ea_src = EA_AY_PD_8();
		src = (src | (m68ki_read_8(ea_src) << 8)) + OPER_I_16();

		m68ki_write_8(EA_AX_PD_8(), ((src >> 4) & 0x00f0) | (src & 0x000f));
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_pea_l_0_ai()
{
	uint32_t ea = EA_AY_AI_32();

	m68ki_push_32(ea);


}
void m68000_base_device::m68k_op_pea_l_0_di()
{
	uint32_t ea = EA_AY_DI_32();

	m68ki_push_32(ea);


}
void m68000_base_device::m68k_op_pea_l_0_ix()
{
	uint32_t ea = EA_AY_IX_32();

	m68ki_push_32(ea);


}
void m68000_base_device::m68k_op_pea_l_0_aw()
{
	uint32_t ea = EA_AW_32();

	m68ki_push_32(ea);


}
void m68000_base_device::m68k_op_pea_l_0_al()
{
	uint32_t ea = EA_AL_32();

	m68ki_push_32(ea);


}
void m68000_base_device::m68k_op_pea_l_0_pcdi()
{
	uint32_t ea = EA_PCDI_32();

	m68ki_push_32(ea);


}
void m68000_base_device::m68k_op_pea_l_0_pcix()
{
	uint32_t ea = EA_PCIX_32();

	m68ki_push_32(ea);


}
void m68000_base_device::m68k_op_pflusha_l_0()
{
	if ((CPU_TYPE_IS_EC020_PLUS()) && (m_has_pmmu))
	{
		logerror("68040: unhandled PFLUSHA (ir=%04x)\n", m_ir);
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_pflushan_l_0()
{
	if ((CPU_TYPE_IS_EC020_PLUS()) && (m_has_pmmu))
	{
		logerror("68040: unhandled PFLUSHAN (ir=%04x)\n", m_ir);
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_pmmu_l_0()
{
	if ((CPU_TYPE_IS_EC020_PLUS()) && (m_has_pmmu))
	{
		m68851_mmu_ops();
	}
	else
	{
		m68ki_exception_1111();
	}


}
void m68000_base_device::m68k_op_ptest_l_0()
{
	if ((CPU_TYPE_IS_040_PLUS()) && (m_has_pmmu))
	{
		logerror("68040: unhandled PTEST\n");
		return;
	}
	else
	{
		m68ki_exception_1111();
	}


}
void m68000_base_device::m68k_op_reset_0()
{
	if(m_s_flag)
	{
		if (!m_reset_instr_callback.isnull())
			(m_reset_instr_callback)(1);
		m68k_reset_peripherals();
		m_icount -= m_cyc_reset;
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_ror_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_8(src, shift);

	if(orig_shift != 0)
		m_icount -= orig_shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = src << (9-orig_shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_1()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_16(src, shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src = *r_dst;
	uint32_t res = ROR_32(src, shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_b_3()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_8(src, shift);

	if(orig_shift != 0)
	{
		m_icount -= orig_shift<<m_cyc_shift;

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		m_c_flag = src << (8-((shift-1)&7));
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_8(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_4()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 15;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_16(src, shift);

	if(orig_shift != 0)
	{
		m_icount -= orig_shift<<m_cyc_shift;

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		m_c_flag = (src >> ((shift - 1) & 15)) << 8;
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_16(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_l_5()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 31;
	uint64_t src = *r_dst;
	uint32_t res = ROR_32(src, shift);

	if(orig_shift != 0)
	{
		m_icount -= orig_shift<<m_cyc_shift;

		*r_dst = res;
		m_c_flag = (src >> ((shift - 1) & 31)) << 8;
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_32(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_6_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_6_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_6_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_6_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_6_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_6_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_ror_w_6_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_16(src, 1);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << 8;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_8(src, shift);

	if(orig_shift != 0)
		m_icount -= orig_shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = src << orig_shift;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_1()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROL_16(src, shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> (8-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_l_2()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src = *r_dst;
	uint32_t res = ROL_32(src, shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = src >> (24-shift);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_b_3()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_8(src, shift);

	if(orig_shift != 0)
	{
		m_icount -= orig_shift<<m_cyc_shift;

		if(shift != 0)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			m_c_flag = src << shift;
			m_n_flag = NFLAG_8(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}
		m_c_flag = (src & 1)<<8;
		m_n_flag = NFLAG_8(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_8(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_4()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 15;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, shift));

	if(orig_shift != 0)
	{
		m_icount -= orig_shift<<m_cyc_shift;

		if(shift != 0)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			m_c_flag = (src << shift) >> 8;
			m_n_flag = NFLAG_16(res);
			m_not_z_flag = res;
			m_v_flag = VFLAG_CLEAR;
			return;
		}
		m_c_flag = (src & 1)<<8;
		m_n_flag = NFLAG_16(src);
		m_not_z_flag = src;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_16(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_l_5()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 31;
	uint64_t src = *r_dst;
	uint32_t res = ROL_32(src, shift);

	if(orig_shift != 0)
	{
		m_icount -= orig_shift<<m_cyc_shift;

		*r_dst = res;

		m_c_flag = (src >> ((32 - shift) & 0x1f)) << 8;
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = CFLAG_CLEAR;
	m_n_flag = NFLAG_32(src);
	m_not_z_flag = src;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_6_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_6_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_6_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_6_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_6_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_6_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rol_w_6_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> 7;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_9(src | (XFLAG_1() << 8), shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	m_c_flag = m_x_flag = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_1()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_l_2()
{
	uint32_t*  r_dst = &DY();
	uint32_t   shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src   = *r_dst;
	uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	res = ROR_33_64(res, shift);

	m_c_flag = m_x_flag = res >> 24;
	res = MASK_OUT_ABOVE_32(res);

	*r_dst =  res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_b_3()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 9;
		uint32_t src   = MASK_OUT_ABOVE_8(*r_dst);
		uint32_t res   = ROR_9(src | (XFLAG_1() << 8), shift);

		m_icount -= orig_shift<<m_cyc_shift;

		m_c_flag = m_x_flag = res;
		res = MASK_OUT_ABOVE_8(res);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = m_x_flag;
	m_n_flag = NFLAG_8(*r_dst);
	m_not_z_flag = MASK_OUT_ABOVE_8(*r_dst);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_4()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 17;
		uint32_t src   = MASK_OUT_ABOVE_16(*r_dst);
		uint32_t res   = ROR_17(src | (XFLAG_1() << 16), shift);

		m_icount -= orig_shift<<m_cyc_shift;

		m_c_flag = m_x_flag = res >> 8;
		res = MASK_OUT_ABOVE_16(res);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = m_x_flag;
	m_n_flag = NFLAG_16(*r_dst);
	m_not_z_flag = MASK_OUT_ABOVE_16(*r_dst);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_l_5()
{
	uint32_t*  r_dst = &DY();
	uint32_t   orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t   shift = orig_shift % 33;
		uint64_t src   = *r_dst;
		uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

		res = ROR_33_64(res, shift);

		m_icount -= orig_shift<<m_cyc_shift;

		m_c_flag = m_x_flag = res >> 24;
		res = MASK_OUT_ABOVE_32(res);

		*r_dst = res;
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = m_x_flag;
	m_n_flag = NFLAG_32(*r_dst);
	m_not_z_flag = *r_dst;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_6_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_6_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_6_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_6_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_6_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_6_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxr_w_6_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_9(src | (XFLAG_1() << 8), shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	m_c_flag = m_x_flag = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_1()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), shift);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_l_2()
{
	uint32_t*  r_dst = &DY();
	uint32_t   shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src   = *r_dst;
	uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

	if(shift != 0)
		m_icount -= shift<<m_cyc_shift;

	res = ROL_33_64(res, shift);

	m_c_flag = m_x_flag = res >> 24;
	res = MASK_OUT_ABOVE_32(res);

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_b_3()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;


	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 9;
		uint32_t src   = MASK_OUT_ABOVE_8(*r_dst);
		uint32_t res   = ROL_9(src | (XFLAG_1() << 8), shift);

		m_icount -= orig_shift<<m_cyc_shift;

		m_c_flag = m_x_flag = res;
		res = MASK_OUT_ABOVE_8(res);

		*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = m_x_flag;
	m_n_flag = NFLAG_8(*r_dst);
	m_not_z_flag = MASK_OUT_ABOVE_8(*r_dst);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_4()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 17;
		uint32_t src   = MASK_OUT_ABOVE_16(*r_dst);
		uint32_t res   = ROL_17(src | (XFLAG_1() << 16), shift);

		m_icount -= orig_shift<<m_cyc_shift;

		m_c_flag = m_x_flag = res >> 8;
		res = MASK_OUT_ABOVE_16(res);

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = m_x_flag;
	m_n_flag = NFLAG_16(*r_dst);
	m_not_z_flag = MASK_OUT_ABOVE_16(*r_dst);
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_l_5()
{
	uint32_t*  r_dst = &DY();
	uint32_t   orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t   shift = orig_shift % 33;
		uint64_t src   = *r_dst;
		uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

		res = ROL_33_64(res, shift);

		m_icount -= orig_shift<<m_cyc_shift;

		m_c_flag = m_x_flag = res >> 24;
		res = MASK_OUT_ABOVE_32(res);

		*r_dst = res;
		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		return;
	}

	m_c_flag = m_x_flag;
	m_n_flag = NFLAG_32(*r_dst);
	m_not_z_flag = *r_dst;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_6_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_6_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_6_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_6_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_6_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_6_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_roxl_w_6_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = m68ki_read_16(ea);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), 1);

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_rtd_l_0()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		uint32_t new_pc = m68ki_pull_32();

		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		REG_A()[7] = MASK_OUT_ABOVE_32(REG_A()[7] + MAKE_INT_16(OPER_I_16()));
		m68ki_jump(new_pc);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_rte_l_0()
{
	if(m_s_flag)
	{
		uint32_t new_sr;
		uint32_t new_pc;
		uint32_t format_word;

		if (!m_rte_instr_callback.isnull())
			(m_rte_instr_callback)(1);
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */

		if(CPU_TYPE_IS_000())
		{
			new_sr = m68ki_pull_16();
			new_pc = m68ki_pull_32();
			m68ki_jump(new_pc);
			m68ki_set_sr(new_sr);

			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;

			return;
		}

		if(CPU_TYPE_IS_010())
		{
			format_word = m68ki_read_16(REG_A()[7]+6) >> 12;
			if(format_word == 0)
			{
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();   /* format word */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				m_instr_mode = INSTRUCTION_YES;
				m_run_mode = RUN_MODE_NORMAL;
				return;
			}
			m_instr_mode = INSTRUCTION_YES;
			m_run_mode = RUN_MODE_NORMAL;
			/* Not handling bus fault (9) */
			m68ki_exception_format_error();
			return;
		}

		/* Otherwise it's 020 */
	rte_loop:
		format_word = m68ki_read_16(REG_A()[7]+6) >> 12;
		switch(format_word)
		{
			case 0: /* Normal */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();   /* format word */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				m_instr_mode = INSTRUCTION_YES;
				m_run_mode = RUN_MODE_NORMAL;
				return;
			case 1: /* Throwaway */
				new_sr = m68ki_pull_16();
				m68ki_fake_pull_32();   /* program counter */
				m68ki_fake_pull_16();   /* format word */
				m68ki_set_sr_noint(new_sr);
				goto rte_loop;
			case 2: /* Trap */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();   /* format word */
				m68ki_fake_pull_32();   /* address */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				m_instr_mode = INSTRUCTION_YES;
				m_run_mode = RUN_MODE_NORMAL;
				return;
			case 7: /* 68040 access error */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();   /* $06: format word */
				m68ki_fake_pull_32();   /* $08: effective address */
				m68ki_fake_pull_16();   /* $0c: special status word */
				m68ki_fake_pull_16();   /* $0e: wb3s */
				m68ki_fake_pull_16();   /* $10: wb2s */
				m68ki_fake_pull_16();   /* $12: wb1s */
				m68ki_fake_pull_32();   /* $14: data fault address */
				m68ki_fake_pull_32();   /* $18: wb3a */
				m68ki_fake_pull_32();   /* $1c: wb3d */
				m68ki_fake_pull_32();   /* $20: wb2a */
				m68ki_fake_pull_32();   /* $24: wb2d */
				m68ki_fake_pull_32();   /* $28: wb1a */
				m68ki_fake_pull_32();   /* $2c: wb1d/pd0 */
				m68ki_fake_pull_32();   /* $30: pd1 */
				m68ki_fake_pull_32();   /* $34: pd2 */
				m68ki_fake_pull_32();   /* $38: pd3 */
				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				m_instr_mode = INSTRUCTION_YES;
				m_run_mode = RUN_MODE_NORMAL;
				return;

			case 0x0a: /* Bus Error at instruction boundary */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();   /* $06: format word */
				m68ki_fake_pull_16();   /* $08: internal register */
				m68ki_fake_pull_16();   /* $0a: special status word */
				m68ki_fake_pull_16();   /* $0c: instruction pipe stage c */
				m68ki_fake_pull_16();   /* $0e: instruction pipe stage b */
				m68ki_fake_pull_32();   /* $10: data fault address */
				m68ki_fake_pull_32();   /* $14: internal registers */
				m68ki_fake_pull_32();   /* $18: data output buffer */
				m68ki_fake_pull_32();   /* $1c: internal registers */

				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				m_instr_mode = INSTRUCTION_YES;
				m_run_mode = RUN_MODE_NORMAL;
				return;
			case 0x0b: /* Bus Error - Instruction Execution in Progress */
				new_sr = m68ki_pull_16();
				new_pc = m68ki_pull_32();
				m68ki_fake_pull_16();   /* $06: format word */
				m68ki_fake_pull_16();   /* $08: internal register */
				m68ki_fake_pull_16();   /* $0a: special status word */
				m68ki_fake_pull_16();   /* $0c: instruction pipe stage c */
				m68ki_fake_pull_16();   /* $0e: instruction pipe stage b */
				m68ki_fake_pull_32();   /* $10: data fault address */
				m68ki_fake_pull_32();   /* $14: internal registers */
				m68ki_fake_pull_32();   /* $18: data output buffer */
				m68ki_fake_pull_32();   /* $1c: internal registers */
				m68ki_fake_pull_32();   /* $20:  */
				m68ki_fake_pull_32();   /* $24: stage B address */
				m68ki_fake_pull_32();   /* $28:  */
				m68ki_fake_pull_32();   /* $2c: data input buffer */
				m68ki_fake_pull_32();   /* $30:  */
				m68ki_fake_pull_16();   /* $34:  */
				m68ki_fake_pull_16();   /* $36: version #, internal information */
				m68ki_fake_pull_32();   /* $38:  */
				m68ki_fake_pull_32();   /* $3c:  */
				m68ki_fake_pull_32();   /* $40:  */
				m68ki_fake_pull_32();   /* $44:  */
				m68ki_fake_pull_32();   /* $48:  */
				m68ki_fake_pull_32();   /* $4c:  */
				m68ki_fake_pull_32();   /* $50:  */
				m68ki_fake_pull_32();   /* $54:  */
				m68ki_fake_pull_32();   /* $58:  */

				m68ki_jump(new_pc);
				m68ki_set_sr(new_sr);
				m_instr_mode = INSTRUCTION_YES;
				m_run_mode = RUN_MODE_NORMAL;
				return;
		}
		/* Not handling long or short bus fault */
		m_instr_mode = INSTRUCTION_YES;
		m_run_mode = RUN_MODE_NORMAL;
		m68ki_exception_format_error();
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_rtm_l_0()
{
	if(CPU_TYPE_IS_020_VARIANT())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		logerror("%s at %08x: called unimplemented instruction %04x (rtm)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_rtr_l_0()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_set_ccr(m68ki_pull_16());
	m68ki_jump(m68ki_pull_32());


}
void m68000_base_device::m68k_op_rts_l_0()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_jump(m68ki_pull_32());


}
void m68000_base_device::m68k_op_sbcd_b_0()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff)
	{
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	}
	else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void m68000_base_device::m68k_op_sbcd_b_1()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff)
	{
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	}
	else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_sbcd_b_2()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff)
	{
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	}
	else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_sbcd_b_3()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff)
	{
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	}
	else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_sbcd_b_4()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_1();
	uint32_t corf = 0;

	if(res > 0xf)
		corf = 6;
	res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
	m_v_flag = res; /* Undefined V behavior */
	if(res > 0xff)
	{
		res += 0xa0;
		m_x_flag = m_c_flag = CFLAG_SET;
	}
	else if(res < corf)
		m_x_flag = m_c_flag = CFLAG_SET;
	else
		m_n_flag = m_x_flag = m_c_flag = 0;

	res = MASK_OUT_ABOVE_8(res - corf);

	m_v_flag &= ~res; /* Undefined V behavior part II */
	m_n_flag = NFLAG_8(res); /* Undefined N behavior */
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_st_b_0()
{
	DY() |= 0xff;


}
void m68000_base_device::m68k_op_st_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), 0xff);


}
void m68000_base_device::m68k_op_st_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), 0xff);


}
void m68000_base_device::m68k_op_st_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), 0xff);


}
void m68000_base_device::m68k_op_st_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), 0xff);


}
void m68000_base_device::m68k_op_st_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), 0xff);


}
void m68000_base_device::m68k_op_st_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), 0xff);


}
void m68000_base_device::m68k_op_st_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), 0xff);


}
void m68000_base_device::m68k_op_st_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), 0xff);


}
void m68000_base_device::m68k_op_st_b_1_al()
{
	m68ki_write_8(EA_AL_8(), 0xff);


}
void m68000_base_device::m68k_op_sf_b_0()
{
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_sf_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), 0);


}
void m68000_base_device::m68k_op_sf_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), 0);


}
void m68000_base_device::m68k_op_sf_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), 0);


}
void m68000_base_device::m68k_op_sf_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), 0);


}
void m68000_base_device::m68k_op_sf_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), 0);


}
void m68000_base_device::m68k_op_sf_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), 0);


}
void m68000_base_device::m68k_op_sf_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), 0);


}
void m68000_base_device::m68k_op_sf_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), 0);


}
void m68000_base_device::m68k_op_sf_b_1_al()
{
	m68ki_write_8(EA_AL_8(), 0);


}
void m68000_base_device::m68k_op_shi_b_0()
{
	if(COND_HI())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_sls_b_0()
{
	if(COND_LS())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_scc_b_2()
{
	if(COND_CC())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_scs_b_0()
{
	if(COND_CS())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_sne_b_0()
{
	if(COND_NE())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_seq_b_0()
{
	if(COND_EQ())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_svc_b_0()
{
	if(COND_VC())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_svs_b_0()
{
	if(COND_VS())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_spl_b_0()
{
	if(COND_PL())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_smi_b_0()
{
	if(COND_MI())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_sge_b_0()
{
	if(COND_GE())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_slt_b_0()
{
	if(COND_LT())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_sgt_b_0()
{
	if(COND_GT())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_sle_b_0()
{
	if(COND_LE())
	{
		DY() |= 0xff;
		m_icount -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;


}
void m68000_base_device::m68k_op_shi_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_shi_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_shi_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_shi_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_shi_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_shi_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_shi_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_shi_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_shi_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_HI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sls_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_LS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_aw()
{
	m68ki_write_8(EA_AW_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scc_b_3_al()
{
	m68ki_write_8(EA_AL_8(), COND_CC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_scs_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_CS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sne_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_NE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_seq_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_EQ() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svc_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_VC() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_svs_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_VS() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_spl_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_PL() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_smi_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_MI() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sge_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_GE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_slt_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_LT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sgt_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_GT() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_aw()
{
	m68ki_write_8(EA_AW_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_sle_b_1_al()
{
	m68ki_write_8(EA_AL_8(), COND_LE() ? 0xff : 0);


}
void m68000_base_device::m68k_op_stop_0()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_I_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_stopped |= STOP_LEVEL_STOP;
		m68ki_set_sr(new_sr);
		m_icount = 0;
		return;
	}
	m68ki_exception_privilege_violation();


}
void m68000_base_device::m68k_op_sub_b_0()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_pi7()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_pd7()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_A7_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_di()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_al()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_1_i()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_2()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_3()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(AY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_di()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_al()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_w_4_i()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_5()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_6()
{
	uint32_t* r_dst = &DX();
	uint32_t src = AY();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_di()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AW_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_al()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_AL_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_l_7_i()
{
	uint32_t* r_dst = &DX();
	uint32_t src = OPER_I_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_sub_b_8_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_b_8_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_b_8_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_b_8_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_b_8_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_b_8_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_b_8_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_b_8_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_b_8_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = MASK_OUT_ABOVE_8(DX());
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_w_9_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_w_9_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_w_9_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_w_9_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_w_9_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_w_9_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_w_9_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t src = MASK_OUT_ABOVE_16(DX());
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_l_10_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_l_10_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_l_10_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_l_10_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_l_10_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_l_10_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_sub_l_10_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t src = DX();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_suba_w_0()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(DY()));


}
void m68000_base_device::m68k_op_suba_w_1()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(AY()));


}
void m68000_base_device::m68k_op_suba_w_2_ai()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_pi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_pd()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_di()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_ix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_aw()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AW_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_al()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AL_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_pcdi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_pcix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_w_2_i()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_I_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_3()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - DY());


}
void m68000_base_device::m68k_op_suba_l_4()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - AY());


}
void m68000_base_device::m68k_op_suba_l_5_ai()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_AI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_pi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_pd()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PD_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_di()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_DI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_ix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_IX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_aw()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AW_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_al()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AL_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_pcdi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCDI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_pcix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCIX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_suba_l_5_i()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_I_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);


}
void m68000_base_device::m68k_op_subi_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_subi_b_1_ai()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_b_1_pi()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_b_1_pi7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_b_1_pd()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_b_1_pd7()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_b_1_di()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_b_1_ix()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_b_1_aw()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AW_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_b_1_al()
{
	uint32_t src = OPER_I_8();
	uint32_t ea = EA_AL_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_w_2()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_subi_w_3_ai()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_AI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_w_3_pi()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_w_3_pd()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_PD_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_w_3_di()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_DI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_w_3_ix()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AY_IX_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_w_3_aw()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AW_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_w_3_al()
{
	uint32_t src = OPER_I_16();
	uint32_t ea = EA_AL_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_l_4()
{
	uint32_t* r_dst = &DY();
	uint32_t src = OPER_I_32();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_subi_l_5_ai()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_AI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_l_5_pi()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_l_5_pd()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_PD_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_l_5_di()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_DI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_l_5_ix()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AY_IX_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_l_5_aw()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AW_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subi_l_5_al()
{
	uint32_t src = OPER_I_32();
	uint32_t ea = EA_AL_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_0()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_subq_b_1_ai()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_1_pi()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_1_pi7()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_1_pd()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_1_pd7()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_1_di()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_1_ix()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_1_aw()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_b_1_al()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	m68ki_write_8(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_w_2()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;


}
void m68000_base_device::m68k_op_subq_w_3()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((m_ir >> 9) - 1) & 7) + 1));


}
void m68000_base_device::m68k_op_subq_w_4_ai()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_w_4_pi()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_w_4_pd()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_w_4_di()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_w_4_ix()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_w_4_aw()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_w_4_al()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	m68ki_write_16(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_l_5()
{
	uint32_t* r_dst = &DY();
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t dst = *r_dst;
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	*r_dst = m_not_z_flag;


}
void m68000_base_device::m68k_op_subq_l_6()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((m_ir >> 9) - 1) & 7) + 1));


}
void m68000_base_device::m68k_op_subq_l_7_ai()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_AI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_l_7_pi()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_l_7_pd()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_PD_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_l_7_di()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_DI_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_l_7_ix()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AY_IX_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_l_7_aw()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AW_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subq_l_7_al()
{
	uint32_t src = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t ea = EA_AL_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	m68ki_write_32(ea, m_not_z_flag);


}
void m68000_base_device::m68k_op_subx_b_0()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;


}
void m68000_base_device::m68k_op_subx_w_1()
{
	uint32_t* r_dst = &DX();
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;


}
void m68000_base_device::m68k_op_subx_l_2()
{
	uint32_t* r_dst = &DX();
	uint32_t src = DY();
	uint32_t dst = *r_dst;
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	*r_dst = res;


}
void m68000_base_device::m68k_op_subx_b_3()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_subx_b_4()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_subx_b_5()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t ea  = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_subx_b_6()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t ea  = EA_AX_PD_8();
	uint32_t dst = m68ki_read_8(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_8(res);
	m_x_flag = m_c_flag = CFLAG_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);

	res = MASK_OUT_ABOVE_8(res);
	m_not_z_flag |= res;

	m68ki_write_8(ea, res);


}
void m68000_base_device::m68k_op_subx_w_7()
{
	uint32_t src = OPER_AY_PD_16();
	uint32_t ea  = EA_AX_PD_16();
	uint32_t dst = m68ki_read_16(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_16(res);
	m_x_flag = m_c_flag = CFLAG_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);

	res = MASK_OUT_ABOVE_16(res);
	m_not_z_flag |= res;

	m68ki_write_16(ea, res);


}
void m68000_base_device::m68k_op_subx_l_8()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t ea  = EA_AX_PD_32();
	uint32_t dst = m68ki_read_32(ea);
	uint32_t res = dst - src - XFLAG_1();

	m_n_flag = NFLAG_32(res);
	m_x_flag = m_c_flag = CFLAG_SUB_32(src, dst, res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	m_not_z_flag |= res;

	m68ki_write_32(ea, res);


}
void m68000_base_device::m68k_op_swap_l_0()
{
	uint32_t* r_dst = &DY();

	m_not_z_flag = MASK_OUT_ABOVE_32(*r_dst<<16);
	*r_dst = (*r_dst>>16) | m_not_z_flag;

	m_not_z_flag = *r_dst;
	m_n_flag = NFLAG_32(*r_dst);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tas_b_0()
{
	uint32_t* r_dst = &DY();

	m_not_z_flag = MASK_OUT_ABOVE_8(*r_dst);
	m_n_flag = NFLAG_8(*r_dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	*r_dst |= 0x80;


}
void m68000_base_device::m68k_op_tas_b_1_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_tas_b_1_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_tas_b_1_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_tas_b_1_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_tas_b_1_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_tas_b_1_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_tas_b_1_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_tas_b_1_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_tas_b_1_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t dst = m68ki_read_8(ea);

	m_not_z_flag = dst;
	m_n_flag = NFLAG_8(dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;

	/* On the 68000 and 68010, the TAS instruction uses a unique bus cycle that may have
	   side effects (e.g. delaying DMA) or may fail to write back at all depending on the
	   bus implementation.
	   In particular, the Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS
	   to fail to write back in order to function properly. */
	if (CPU_TYPE_IS_010_LESS() && !m_tas_write_callback.isnull())
		(m_tas_write_callback)(*m_program, ea, dst | 0x80, 0xff);
	else
		m68ki_write_8(ea, dst | 0x80);


}
void m68000_base_device::m68k_op_trap_0()
{
	/* Trap#n stacks exception frame type 0 */
	m68ki_exception_trapN(EXCEPTION_TRAP_BASE + (m_ir & 0xf));    /* HJB 990403 */


}
void m68000_base_device::m68k_op_trapt_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapt_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapt_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapf_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapf_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapf_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traphi_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_HI())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapls_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LS())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapcc_3()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CC())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapcs_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CS())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapne_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_NE())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapeq_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_EQ())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapvc_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VC())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapvs_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VS())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trappl_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_PL())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapmi_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_MI())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapge_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GE())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traplt_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LT())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapgt_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GT())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traple_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LE())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traphi_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_HI())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapls_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LS())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapcc_w_4()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CC())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapcs_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CS())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapne_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_NE())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapeq_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_EQ())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapvc_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VC())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapvs_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VS())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trappl_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_PL())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapmi_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_MI())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapge_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GE())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traplt_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LT())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapgt_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GT())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traple_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LE())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traphi_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_HI())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapls_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LS())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapcc_l_5()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CC())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapcs_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CS())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapne_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_NE())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapeq_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_EQ())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapvc_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VC())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapvs_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VS())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trappl_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_PL())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapmi_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_MI())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapge_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GE())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traplt_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LT())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapgt_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GT())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_traple_l_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LE())
		{
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
			return;
		}
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_trapv_0()
{
	if(COND_VC())
	{
		return;
	}
	m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */


}
void m68000_base_device::m68k_op_tst_b_0()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_ai()
{
	uint32_t res = OPER_AY_AI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_pi()
{
	uint32_t res = OPER_AY_PI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_pi7()
{
	uint32_t res = OPER_A7_PI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_pd()
{
	uint32_t res = OPER_AY_PD_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_pd7()
{
	uint32_t res = OPER_A7_PD_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_di()
{
	uint32_t res = OPER_AY_DI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_ix()
{
	uint32_t res = OPER_AY_IX_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_aw()
{
	uint32_t res = OPER_AW_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_1_al()
{
	uint32_t res = OPER_AL_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_b_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_PCDI_8();

		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_b_3()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_PCIX_8();

		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_b_4()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_I_8();

		m_n_flag = NFLAG_8(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_w_5()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_w_6()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = MAKE_INT_16(AY());

		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_w_7_ai()
{
	uint32_t res = OPER_AY_AI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_w_7_pi()
{
	uint32_t res = OPER_AY_PI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_w_7_pd()
{
	uint32_t res = OPER_AY_PD_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_w_7_di()
{
	uint32_t res = OPER_AY_DI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_w_7_ix()
{
	uint32_t res = OPER_AY_IX_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_w_7_aw()
{
	uint32_t res = OPER_AW_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_w_7_al()
{
	uint32_t res = OPER_AL_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_w_8()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_PCDI_16();

		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_w_9()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_PCIX_16();

		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_w_10()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_I_16();

		m_n_flag = NFLAG_16(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_l_11()
{
	uint32_t res = DY();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_l_12()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = AY();

		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_l_13_ai()
{
	uint32_t res = OPER_AY_AI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_l_13_pi()
{
	uint32_t res = OPER_AY_PI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_l_13_pd()
{
	uint32_t res = OPER_AY_PD_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_l_13_di()
{
	uint32_t res = OPER_AY_DI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_l_13_ix()
{
	uint32_t res = OPER_AY_IX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_l_13_aw()
{
	uint32_t res = OPER_AW_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_l_13_al()
{
	uint32_t res = OPER_AL_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;


}
void m68000_base_device::m68k_op_tst_l_14()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_PCDI_32();

		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_l_15()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_PCIX_32();

		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_tst_l_16()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t res = OPER_I_32();

		m_n_flag = NFLAG_32(res);
		m_not_z_flag = res;
		m_v_flag = VFLAG_CLEAR;
		m_c_flag = CFLAG_CLEAR;
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_unlk_l_0()
{
	REG_A()[7] = m68ki_read_32(REG_A()[7]);


}
void m68000_base_device::m68k_op_unlk_l_1()
{
	uint32_t* r_dst = &AY();

	REG_A()[7] = *r_dst;
	*r_dst = m68ki_pull_32();


}
void m68000_base_device::m68k_op_unpk_w_0()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		/* Note: DX() and DY() are reversed in Motorola's docs */
		uint32_t src = DY();
		uint32_t* r_dst = &DX();

		*r_dst = MASK_OUT_BELOW_16(*r_dst) | (((((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16()) & 0xffff);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_unpk_w_1()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint32_t src = OPER_AY_PD_8();
		uint32_t ea_dst;

		src = (((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16();
		ea_dst = EA_A7_PD_8();
		m68ki_write_8(ea_dst, src & 0xff);
		ea_dst = EA_A7_PD_8();
		m68ki_write_8(ea_dst, (src >> 8) & 0xff);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_unpk_w_2()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint32_t src = OPER_A7_PD_8();
		uint32_t ea_dst;

		src = (((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16();
		ea_dst = EA_AX_PD_8();
		m68ki_write_8(ea_dst, src & 0xff);
		ea_dst = EA_AX_PD_8();
		m68ki_write_8(ea_dst, (src >> 8) & 0xff);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_unpk_w_3()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t src = OPER_A7_PD_8();
		uint32_t ea_dst;

		src = (((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16();
		ea_dst = EA_A7_PD_8();
		m68ki_write_8(ea_dst, src & 0xff);
		ea_dst = EA_A7_PD_8();
		m68ki_write_8(ea_dst, (src >> 8) & 0xff);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_unpk_w_4()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		/* Note: AX and AY are reversed in Motorola's docs */
		uint32_t src = OPER_AY_PD_8();
		uint32_t ea_dst;

		src = (((src << 4) & 0x0f00) | (src & 0x000f)) + OPER_I_16();
		ea_dst = EA_AX_PD_8();
		m68ki_write_8(ea_dst, src & 0xff);
		ea_dst = EA_AX_PD_8();
		m68ki_write_8(ea_dst, (src >> 8) & 0xff);
		return;
	}
	m68ki_exception_illegal();


}
void m68000_base_device::m68k_op_cinv_l_0()
{
	if(CPU_TYPE_IS_040_PLUS())
	{
		uint16_t ir = m_ir;
		uint8_t cache = (ir >> 6) & 3;
    //  uint8_t scope = (ir >> 3) & 3;
    //  logerror("68040 %s: pc=%08x ir=%04x cache=%d scope=%d register=%d\n", ir & 0x0020 ? "cpush" : "cinv", m_ppc, ir, cache, scope, ir & 7);
		switch (cache)
		{
		case 2:
		case 3:
			// we invalidate/push the whole instruction cache
			m68ki_ic_clear();
		}
		return;
	}
	m68ki_exception_1111();


}
void m68000_base_device::m68k_op_cpush_l_0()
{
	if(CPU_TYPE_IS_040_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpush)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();


}
const m68000_base_device::opcode_handler_ptr m68000_base_device::m68k_handler_table[] =
{

	&m68000_base_device::m68k_op_1010_0,
	&m68000_base_device::m68k_op_1111_0,
	&m68000_base_device::m68k_op_moveq_l_0,
	&m68000_base_device::m68k_op_cpbcc_l_0,
	&m68000_base_device::m68k_op_cpgen_l_0,
	&m68000_base_device::m68k_op_cpscc_l_0,
	&m68000_base_device::m68k_op_pmmu_l_0,
	&m68000_base_device::m68k_op_bra_b_0,
	&m68000_base_device::m68k_op_bsr_b_0,
	&m68000_base_device::m68k_op_bhi_b_0,
	&m68000_base_device::m68k_op_bls_b_0,
	&m68000_base_device::m68k_op_bcc_b_3,
	&m68000_base_device::m68k_op_bcs_b_0,
	&m68000_base_device::m68k_op_bne_b_0,
	&m68000_base_device::m68k_op_beq_b_0,
	&m68000_base_device::m68k_op_bvc_b_0,
	&m68000_base_device::m68k_op_bvs_b_0,
	&m68000_base_device::m68k_op_bpl_b_0,
	&m68000_base_device::m68k_op_bmi_b_0,
	&m68000_base_device::m68k_op_bge_b_0,
	&m68000_base_device::m68k_op_blt_b_0,
	&m68000_base_device::m68k_op_bgt_b_0,
	&m68000_base_device::m68k_op_ble_b_0,
	&m68000_base_device::m68k_op_040fpu0_l_0,
	&m68000_base_device::m68k_op_040fpu1_l_0,
	&m68000_base_device::m68k_op_cinv_l_0,
	&m68000_base_device::m68k_op_cpush_l_0,
	&m68000_base_device::m68k_op_btst_l_0,
	&m68000_base_device::m68k_op_movep_w_2,
	&m68000_base_device::m68k_op_btst_b_1_ai,
	&m68000_base_device::m68k_op_btst_b_1_pi,
	&m68000_base_device::m68k_op_btst_b_1_pd,
	&m68000_base_device::m68k_op_btst_b_1_di,
	&m68000_base_device::m68k_op_btst_b_1_ix,
	&m68000_base_device::m68k_op_bchg_l_0,
	&m68000_base_device::m68k_op_movep_l_3,
	&m68000_base_device::m68k_op_bchg_b_1_ai,
	&m68000_base_device::m68k_op_bchg_b_1_pi,
	&m68000_base_device::m68k_op_bchg_b_1_pd,
	&m68000_base_device::m68k_op_bchg_b_1_di,
	&m68000_base_device::m68k_op_bchg_b_1_ix,
	&m68000_base_device::m68k_op_bclr_l_0,
	&m68000_base_device::m68k_op_movep_w_0,
	&m68000_base_device::m68k_op_bclr_b_1_ai,
	&m68000_base_device::m68k_op_bclr_b_1_pi,
	&m68000_base_device::m68k_op_bclr_b_1_pd,
	&m68000_base_device::m68k_op_bclr_b_1_di,
	&m68000_base_device::m68k_op_bclr_b_1_ix,
	&m68000_base_device::m68k_op_bset_l_0,
	&m68000_base_device::m68k_op_movep_l_1,
	&m68000_base_device::m68k_op_bset_b_1_ai,
	&m68000_base_device::m68k_op_bset_b_1_pi,
	&m68000_base_device::m68k_op_bset_b_1_pd,
	&m68000_base_device::m68k_op_bset_b_1_di,
	&m68000_base_device::m68k_op_bset_b_1_ix,
	&m68000_base_device::m68k_op_move_b_0,
	&m68000_base_device::m68k_op_move_b_1_ai,
	&m68000_base_device::m68k_op_move_b_1_pi,
	&m68000_base_device::m68k_op_move_b_1_pd,
	&m68000_base_device::m68k_op_move_b_1_di,
	&m68000_base_device::m68k_op_move_b_1_ix,
	&m68000_base_device::m68k_op_move_b_2,
	&m68000_base_device::m68k_op_move_b_3_ai,
	&m68000_base_device::m68k_op_move_b_3_pi,
	&m68000_base_device::m68k_op_move_b_3_pd,
	&m68000_base_device::m68k_op_move_b_3_di,
	&m68000_base_device::m68k_op_move_b_3_ix,
	&m68000_base_device::m68k_op_move_b_5,
	&m68000_base_device::m68k_op_move_b_7_ai,
	&m68000_base_device::m68k_op_move_b_7_pi,
	&m68000_base_device::m68k_op_move_b_7_pd,
	&m68000_base_device::m68k_op_move_b_7_di,
	&m68000_base_device::m68k_op_move_b_7_ix,
	&m68000_base_device::m68k_op_move_b_9,
	&m68000_base_device::m68k_op_move_b_11_ai,
	&m68000_base_device::m68k_op_move_b_11_pi,
	&m68000_base_device::m68k_op_move_b_11_pd,
	&m68000_base_device::m68k_op_move_b_11_di,
	&m68000_base_device::m68k_op_move_b_11_ix,
	&m68000_base_device::m68k_op_move_b_12,
	&m68000_base_device::m68k_op_move_b_13_ai,
	&m68000_base_device::m68k_op_move_b_13_pi,
	&m68000_base_device::m68k_op_move_b_13_pd,
	&m68000_base_device::m68k_op_move_b_13_di,
	&m68000_base_device::m68k_op_move_b_13_ix,
	&m68000_base_device::m68k_op_move_b_14,
	&m68000_base_device::m68k_op_move_b_15_ai,
	&m68000_base_device::m68k_op_move_b_15_pi,
	&m68000_base_device::m68k_op_move_b_15_pd,
	&m68000_base_device::m68k_op_move_b_15_di,
	&m68000_base_device::m68k_op_move_b_15_ix,
	&m68000_base_device::m68k_op_move_l_44,
	&m68000_base_device::m68k_op_move_l_45,
	&m68000_base_device::m68k_op_move_l_46_ai,
	&m68000_base_device::m68k_op_move_l_46_pi,
	&m68000_base_device::m68k_op_move_l_46_pd,
	&m68000_base_device::m68k_op_move_l_46_di,
	&m68000_base_device::m68k_op_move_l_46_ix,
	&m68000_base_device::m68k_op_movea_l_3,
	&m68000_base_device::m68k_op_movea_l_4,
	&m68000_base_device::m68k_op_movea_l_5_ai,
	&m68000_base_device::m68k_op_movea_l_5_pi,
	&m68000_base_device::m68k_op_movea_l_5_pd,
	&m68000_base_device::m68k_op_movea_l_5_di,
	&m68000_base_device::m68k_op_movea_l_5_ix,
	&m68000_base_device::m68k_op_move_l_47,
	&m68000_base_device::m68k_op_move_l_48,
	&m68000_base_device::m68k_op_move_l_49_ai,
	&m68000_base_device::m68k_op_move_l_49_pi,
	&m68000_base_device::m68k_op_move_l_49_pd,
	&m68000_base_device::m68k_op_move_l_49_di,
	&m68000_base_device::m68k_op_move_l_49_ix,
	&m68000_base_device::m68k_op_move_l_50,
	&m68000_base_device::m68k_op_move_l_51,
	&m68000_base_device::m68k_op_move_l_52_ai,
	&m68000_base_device::m68k_op_move_l_52_pi,
	&m68000_base_device::m68k_op_move_l_52_pd,
	&m68000_base_device::m68k_op_move_l_52_di,
	&m68000_base_device::m68k_op_move_l_52_ix,
	&m68000_base_device::m68k_op_move_l_53,
	&m68000_base_device::m68k_op_move_l_54,
	&m68000_base_device::m68k_op_move_l_55_ai,
	&m68000_base_device::m68k_op_move_l_55_pi,
	&m68000_base_device::m68k_op_move_l_55_pd,
	&m68000_base_device::m68k_op_move_l_55_di,
	&m68000_base_device::m68k_op_move_l_55_ix,
	&m68000_base_device::m68k_op_move_l_56,
	&m68000_base_device::m68k_op_move_l_57,
	&m68000_base_device::m68k_op_move_l_58_ai,
	&m68000_base_device::m68k_op_move_l_58_pi,
	&m68000_base_device::m68k_op_move_l_58_pd,
	&m68000_base_device::m68k_op_move_l_58_di,
	&m68000_base_device::m68k_op_move_l_58_ix,
	&m68000_base_device::m68k_op_move_l_59,
	&m68000_base_device::m68k_op_move_l_60,
	&m68000_base_device::m68k_op_move_l_61_ai,
	&m68000_base_device::m68k_op_move_l_61_pi,
	&m68000_base_device::m68k_op_move_l_61_pd,
	&m68000_base_device::m68k_op_move_l_61_di,
	&m68000_base_device::m68k_op_move_l_61_ix,
	&m68000_base_device::m68k_op_move_w_20,
	&m68000_base_device::m68k_op_move_w_21,
	&m68000_base_device::m68k_op_move_w_22_ai,
	&m68000_base_device::m68k_op_move_w_22_pi,
	&m68000_base_device::m68k_op_move_w_22_pd,
	&m68000_base_device::m68k_op_move_w_22_di,
	&m68000_base_device::m68k_op_move_w_22_ix,
	&m68000_base_device::m68k_op_movea_w_0,
	&m68000_base_device::m68k_op_movea_w_1,
	&m68000_base_device::m68k_op_movea_w_2_ai,
	&m68000_base_device::m68k_op_movea_w_2_pi,
	&m68000_base_device::m68k_op_movea_w_2_pd,
	&m68000_base_device::m68k_op_movea_w_2_di,
	&m68000_base_device::m68k_op_movea_w_2_ix,
	&m68000_base_device::m68k_op_move_w_23,
	&m68000_base_device::m68k_op_move_w_24,
	&m68000_base_device::m68k_op_move_w_25_ai,
	&m68000_base_device::m68k_op_move_w_25_pi,
	&m68000_base_device::m68k_op_move_w_25_pd,
	&m68000_base_device::m68k_op_move_w_25_di,
	&m68000_base_device::m68k_op_move_w_25_ix,
	&m68000_base_device::m68k_op_move_w_26,
	&m68000_base_device::m68k_op_move_w_27,
	&m68000_base_device::m68k_op_move_w_28_ai,
	&m68000_base_device::m68k_op_move_w_28_pi,
	&m68000_base_device::m68k_op_move_w_28_pd,
	&m68000_base_device::m68k_op_move_w_28_di,
	&m68000_base_device::m68k_op_move_w_28_ix,
	&m68000_base_device::m68k_op_move_w_29,
	&m68000_base_device::m68k_op_move_w_30,
	&m68000_base_device::m68k_op_move_w_31_ai,
	&m68000_base_device::m68k_op_move_w_31_pi,
	&m68000_base_device::m68k_op_move_w_31_pd,
	&m68000_base_device::m68k_op_move_w_31_di,
	&m68000_base_device::m68k_op_move_w_31_ix,
	&m68000_base_device::m68k_op_move_w_32,
	&m68000_base_device::m68k_op_move_w_33,
	&m68000_base_device::m68k_op_move_w_34_ai,
	&m68000_base_device::m68k_op_move_w_34_pi,
	&m68000_base_device::m68k_op_move_w_34_pd,
	&m68000_base_device::m68k_op_move_w_34_di,
	&m68000_base_device::m68k_op_move_w_34_ix,
	&m68000_base_device::m68k_op_move_w_35,
	&m68000_base_device::m68k_op_move_w_36,
	&m68000_base_device::m68k_op_move_w_37_ai,
	&m68000_base_device::m68k_op_move_w_37_pi,
	&m68000_base_device::m68k_op_move_w_37_pd,
	&m68000_base_device::m68k_op_move_w_37_di,
	&m68000_base_device::m68k_op_move_w_37_ix,
	&m68000_base_device::m68k_op_chk_l_2,
	&m68000_base_device::m68k_op_chk_l_3_ai,
	&m68000_base_device::m68k_op_chk_l_3_pi,
	&m68000_base_device::m68k_op_chk_l_3_pd,
	&m68000_base_device::m68k_op_chk_l_3_di,
	&m68000_base_device::m68k_op_chk_l_3_ix,
	&m68000_base_device::m68k_op_chk_w_0,
	&m68000_base_device::m68k_op_chk_w_1_ai,
	&m68000_base_device::m68k_op_chk_w_1_pi,
	&m68000_base_device::m68k_op_chk_w_1_pd,
	&m68000_base_device::m68k_op_chk_w_1_di,
	&m68000_base_device::m68k_op_chk_w_1_ix,
	&m68000_base_device::m68k_op_lea_l_0_ai,
	&m68000_base_device::m68k_op_lea_l_0_di,
	&m68000_base_device::m68k_op_lea_l_0_ix,
	&m68000_base_device::m68k_op_addq_b_0,
	&m68000_base_device::m68k_op_addq_b_1_ai,
	&m68000_base_device::m68k_op_addq_b_1_pi,
	&m68000_base_device::m68k_op_addq_b_1_pd,
	&m68000_base_device::m68k_op_addq_b_1_di,
	&m68000_base_device::m68k_op_addq_b_1_ix,
	&m68000_base_device::m68k_op_addq_w_2,
	&m68000_base_device::m68k_op_addq_w_3,
	&m68000_base_device::m68k_op_addq_w_4_ai,
	&m68000_base_device::m68k_op_addq_w_4_pi,
	&m68000_base_device::m68k_op_addq_w_4_pd,
	&m68000_base_device::m68k_op_addq_w_4_di,
	&m68000_base_device::m68k_op_addq_w_4_ix,
	&m68000_base_device::m68k_op_addq_l_5,
	&m68000_base_device::m68k_op_addq_l_6,
	&m68000_base_device::m68k_op_addq_l_7_ai,
	&m68000_base_device::m68k_op_addq_l_7_pi,
	&m68000_base_device::m68k_op_addq_l_7_pd,
	&m68000_base_device::m68k_op_addq_l_7_di,
	&m68000_base_device::m68k_op_addq_l_7_ix,
	&m68000_base_device::m68k_op_subq_b_0,
	&m68000_base_device::m68k_op_subq_b_1_ai,
	&m68000_base_device::m68k_op_subq_b_1_pi,
	&m68000_base_device::m68k_op_subq_b_1_pd,
	&m68000_base_device::m68k_op_subq_b_1_di,
	&m68000_base_device::m68k_op_subq_b_1_ix,
	&m68000_base_device::m68k_op_subq_w_2,
	&m68000_base_device::m68k_op_subq_w_3,
	&m68000_base_device::m68k_op_subq_w_4_ai,
	&m68000_base_device::m68k_op_subq_w_4_pi,
	&m68000_base_device::m68k_op_subq_w_4_pd,
	&m68000_base_device::m68k_op_subq_w_4_di,
	&m68000_base_device::m68k_op_subq_w_4_ix,
	&m68000_base_device::m68k_op_subq_l_5,
	&m68000_base_device::m68k_op_subq_l_6,
	&m68000_base_device::m68k_op_subq_l_7_ai,
	&m68000_base_device::m68k_op_subq_l_7_pi,
	&m68000_base_device::m68k_op_subq_l_7_pd,
	&m68000_base_device::m68k_op_subq_l_7_di,
	&m68000_base_device::m68k_op_subq_l_7_ix,
	&m68000_base_device::m68k_op_or_b_0,
	&m68000_base_device::m68k_op_or_b_1_ai,
	&m68000_base_device::m68k_op_or_b_1_pi,
	&m68000_base_device::m68k_op_or_b_1_pd,
	&m68000_base_device::m68k_op_or_b_1_di,
	&m68000_base_device::m68k_op_or_b_1_ix,
	&m68000_base_device::m68k_op_or_w_2,
	&m68000_base_device::m68k_op_or_w_3_ai,
	&m68000_base_device::m68k_op_or_w_3_pi,
	&m68000_base_device::m68k_op_or_w_3_pd,
	&m68000_base_device::m68k_op_or_w_3_di,
	&m68000_base_device::m68k_op_or_w_3_ix,
	&m68000_base_device::m68k_op_or_l_4,
	&m68000_base_device::m68k_op_or_l_5_ai,
	&m68000_base_device::m68k_op_or_l_5_pi,
	&m68000_base_device::m68k_op_or_l_5_pd,
	&m68000_base_device::m68k_op_or_l_5_di,
	&m68000_base_device::m68k_op_or_l_5_ix,
	&m68000_base_device::m68k_op_divu_w_0,
	&m68000_base_device::m68k_op_divu_w_1_ai,
	&m68000_base_device::m68k_op_divu_w_1_pi,
	&m68000_base_device::m68k_op_divu_w_1_pd,
	&m68000_base_device::m68k_op_divu_w_1_di,
	&m68000_base_device::m68k_op_divu_w_1_ix,
	&m68000_base_device::m68k_op_sbcd_b_0,
	&m68000_base_device::m68k_op_sbcd_b_4,
	&m68000_base_device::m68k_op_or_b_6_ai,
	&m68000_base_device::m68k_op_or_b_6_pi,
	&m68000_base_device::m68k_op_or_b_6_pd,
	&m68000_base_device::m68k_op_or_b_6_di,
	&m68000_base_device::m68k_op_or_b_6_ix,
	&m68000_base_device::m68k_op_pack_w_0,
	&m68000_base_device::m68k_op_pack_w_4,
	&m68000_base_device::m68k_op_or_w_7_ai,
	&m68000_base_device::m68k_op_or_w_7_pi,
	&m68000_base_device::m68k_op_or_w_7_pd,
	&m68000_base_device::m68k_op_or_w_7_di,
	&m68000_base_device::m68k_op_or_w_7_ix,
	&m68000_base_device::m68k_op_unpk_w_0,
	&m68000_base_device::m68k_op_unpk_w_4,
	&m68000_base_device::m68k_op_or_l_8_ai,
	&m68000_base_device::m68k_op_or_l_8_pi,
	&m68000_base_device::m68k_op_or_l_8_pd,
	&m68000_base_device::m68k_op_or_l_8_di,
	&m68000_base_device::m68k_op_or_l_8_ix,
	&m68000_base_device::m68k_op_divs_w_0,
	&m68000_base_device::m68k_op_divs_w_1_ai,
	&m68000_base_device::m68k_op_divs_w_1_pi,
	&m68000_base_device::m68k_op_divs_w_1_pd,
	&m68000_base_device::m68k_op_divs_w_1_di,
	&m68000_base_device::m68k_op_divs_w_1_ix,
	&m68000_base_device::m68k_op_sub_b_0,
	&m68000_base_device::m68k_op_sub_b_1_ai,
	&m68000_base_device::m68k_op_sub_b_1_pi,
	&m68000_base_device::m68k_op_sub_b_1_pd,
	&m68000_base_device::m68k_op_sub_b_1_di,
	&m68000_base_device::m68k_op_sub_b_1_ix,
	&m68000_base_device::m68k_op_sub_w_2,
	&m68000_base_device::m68k_op_sub_w_3,
	&m68000_base_device::m68k_op_sub_w_4_ai,
	&m68000_base_device::m68k_op_sub_w_4_pi,
	&m68000_base_device::m68k_op_sub_w_4_pd,
	&m68000_base_device::m68k_op_sub_w_4_di,
	&m68000_base_device::m68k_op_sub_w_4_ix,
	&m68000_base_device::m68k_op_sub_l_5,
	&m68000_base_device::m68k_op_sub_l_6,
	&m68000_base_device::m68k_op_sub_l_7_ai,
	&m68000_base_device::m68k_op_sub_l_7_pi,
	&m68000_base_device::m68k_op_sub_l_7_pd,
	&m68000_base_device::m68k_op_sub_l_7_di,
	&m68000_base_device::m68k_op_sub_l_7_ix,
	&m68000_base_device::m68k_op_suba_w_0,
	&m68000_base_device::m68k_op_suba_w_1,
	&m68000_base_device::m68k_op_suba_w_2_ai,
	&m68000_base_device::m68k_op_suba_w_2_pi,
	&m68000_base_device::m68k_op_suba_w_2_pd,
	&m68000_base_device::m68k_op_suba_w_2_di,
	&m68000_base_device::m68k_op_suba_w_2_ix,
	&m68000_base_device::m68k_op_subx_b_0,
	&m68000_base_device::m68k_op_subx_b_6,
	&m68000_base_device::m68k_op_sub_b_8_ai,
	&m68000_base_device::m68k_op_sub_b_8_pi,
	&m68000_base_device::m68k_op_sub_b_8_pd,
	&m68000_base_device::m68k_op_sub_b_8_di,
	&m68000_base_device::m68k_op_sub_b_8_ix,
	&m68000_base_device::m68k_op_subx_w_1,
	&m68000_base_device::m68k_op_subx_w_7,
	&m68000_base_device::m68k_op_sub_w_9_ai,
	&m68000_base_device::m68k_op_sub_w_9_pi,
	&m68000_base_device::m68k_op_sub_w_9_pd,
	&m68000_base_device::m68k_op_sub_w_9_di,
	&m68000_base_device::m68k_op_sub_w_9_ix,
	&m68000_base_device::m68k_op_subx_l_2,
	&m68000_base_device::m68k_op_subx_l_8,
	&m68000_base_device::m68k_op_sub_l_10_ai,
	&m68000_base_device::m68k_op_sub_l_10_pi,
	&m68000_base_device::m68k_op_sub_l_10_pd,
	&m68000_base_device::m68k_op_sub_l_10_di,
	&m68000_base_device::m68k_op_sub_l_10_ix,
	&m68000_base_device::m68k_op_suba_l_3,
	&m68000_base_device::m68k_op_suba_l_4,
	&m68000_base_device::m68k_op_suba_l_5_ai,
	&m68000_base_device::m68k_op_suba_l_5_pi,
	&m68000_base_device::m68k_op_suba_l_5_pd,
	&m68000_base_device::m68k_op_suba_l_5_di,
	&m68000_base_device::m68k_op_suba_l_5_ix,
	&m68000_base_device::m68k_op_cmp_b_0,
	&m68000_base_device::m68k_op_cmp_b_1_ai,
	&m68000_base_device::m68k_op_cmp_b_1_pi,
	&m68000_base_device::m68k_op_cmp_b_1_pd,
	&m68000_base_device::m68k_op_cmp_b_1_di,
	&m68000_base_device::m68k_op_cmp_b_1_ix,
	&m68000_base_device::m68k_op_cmp_w_2,
	&m68000_base_device::m68k_op_cmp_w_3,
	&m68000_base_device::m68k_op_cmp_w_4_ai,
	&m68000_base_device::m68k_op_cmp_w_4_pi,
	&m68000_base_device::m68k_op_cmp_w_4_pd,
	&m68000_base_device::m68k_op_cmp_w_4_di,
	&m68000_base_device::m68k_op_cmp_w_4_ix,
	&m68000_base_device::m68k_op_cmp_l_5,
	&m68000_base_device::m68k_op_cmp_l_6,
	&m68000_base_device::m68k_op_cmp_l_7_ai,
	&m68000_base_device::m68k_op_cmp_l_7_pi,
	&m68000_base_device::m68k_op_cmp_l_7_pd,
	&m68000_base_device::m68k_op_cmp_l_7_di,
	&m68000_base_device::m68k_op_cmp_l_7_ix,
	&m68000_base_device::m68k_op_cmpa_w_0,
	&m68000_base_device::m68k_op_cmpa_w_1,
	&m68000_base_device::m68k_op_cmpa_w_2_ai,
	&m68000_base_device::m68k_op_cmpa_w_2_pi,
	&m68000_base_device::m68k_op_cmpa_w_2_pd,
	&m68000_base_device::m68k_op_cmpa_w_2_di,
	&m68000_base_device::m68k_op_cmpa_w_2_ix,
	&m68000_base_device::m68k_op_eor_b_0,
	&m68000_base_device::m68k_op_cmpm_b_3,
	&m68000_base_device::m68k_op_eor_b_1_ai,
	&m68000_base_device::m68k_op_eor_b_1_pi,
	&m68000_base_device::m68k_op_eor_b_1_pd,
	&m68000_base_device::m68k_op_eor_b_1_di,
	&m68000_base_device::m68k_op_eor_b_1_ix,
	&m68000_base_device::m68k_op_eor_w_2,
	&m68000_base_device::m68k_op_cmpm_w_4,
	&m68000_base_device::m68k_op_eor_w_3_ai,
	&m68000_base_device::m68k_op_eor_w_3_pi,
	&m68000_base_device::m68k_op_eor_w_3_pd,
	&m68000_base_device::m68k_op_eor_w_3_di,
	&m68000_base_device::m68k_op_eor_w_3_ix,
	&m68000_base_device::m68k_op_eor_l_4,
	&m68000_base_device::m68k_op_cmpm_l_5,
	&m68000_base_device::m68k_op_eor_l_5_ai,
	&m68000_base_device::m68k_op_eor_l_5_pi,
	&m68000_base_device::m68k_op_eor_l_5_pd,
	&m68000_base_device::m68k_op_eor_l_5_di,
	&m68000_base_device::m68k_op_eor_l_5_ix,
	&m68000_base_device::m68k_op_cmpa_l_3,
	&m68000_base_device::m68k_op_cmpa_l_4,
	&m68000_base_device::m68k_op_cmpa_l_5_ai,
	&m68000_base_device::m68k_op_cmpa_l_5_pi,
	&m68000_base_device::m68k_op_cmpa_l_5_pd,
	&m68000_base_device::m68k_op_cmpa_l_5_di,
	&m68000_base_device::m68k_op_cmpa_l_5_ix,
	&m68000_base_device::m68k_op_and_b_0,
	&m68000_base_device::m68k_op_and_b_1_ai,
	&m68000_base_device::m68k_op_and_b_1_pi,
	&m68000_base_device::m68k_op_and_b_1_pd,
	&m68000_base_device::m68k_op_and_b_1_di,
	&m68000_base_device::m68k_op_and_b_1_ix,
	&m68000_base_device::m68k_op_and_w_2,
	&m68000_base_device::m68k_op_and_w_3_ai,
	&m68000_base_device::m68k_op_and_w_3_pi,
	&m68000_base_device::m68k_op_and_w_3_pd,
	&m68000_base_device::m68k_op_and_w_3_di,
	&m68000_base_device::m68k_op_and_w_3_ix,
	&m68000_base_device::m68k_op_and_l_4,
	&m68000_base_device::m68k_op_and_l_5_ai,
	&m68000_base_device::m68k_op_and_l_5_pi,
	&m68000_base_device::m68k_op_and_l_5_pd,
	&m68000_base_device::m68k_op_and_l_5_di,
	&m68000_base_device::m68k_op_and_l_5_ix,
	&m68000_base_device::m68k_op_mulu_w_0,
	&m68000_base_device::m68k_op_mulu_w_1_ai,
	&m68000_base_device::m68k_op_mulu_w_1_pi,
	&m68000_base_device::m68k_op_mulu_w_1_pd,
	&m68000_base_device::m68k_op_mulu_w_1_di,
	&m68000_base_device::m68k_op_mulu_w_1_ix,
	&m68000_base_device::m68k_op_abcd_b_0,
	&m68000_base_device::m68k_op_abcd_b_4,
	&m68000_base_device::m68k_op_and_b_6_ai,
	&m68000_base_device::m68k_op_and_b_6_pi,
	&m68000_base_device::m68k_op_and_b_6_pd,
	&m68000_base_device::m68k_op_and_b_6_di,
	&m68000_base_device::m68k_op_and_b_6_ix,
	&m68000_base_device::m68k_op_exg_l_0,
	&m68000_base_device::m68k_op_exg_l_1,
	&m68000_base_device::m68k_op_and_w_7_ai,
	&m68000_base_device::m68k_op_and_w_7_pi,
	&m68000_base_device::m68k_op_and_w_7_pd,
	&m68000_base_device::m68k_op_and_w_7_di,
	&m68000_base_device::m68k_op_and_w_7_ix,
	&m68000_base_device::m68k_op_exg_l_2,
	&m68000_base_device::m68k_op_and_l_8_ai,
	&m68000_base_device::m68k_op_and_l_8_pi,
	&m68000_base_device::m68k_op_and_l_8_pd,
	&m68000_base_device::m68k_op_and_l_8_di,
	&m68000_base_device::m68k_op_and_l_8_ix,
	&m68000_base_device::m68k_op_muls_w_0,
	&m68000_base_device::m68k_op_muls_w_1_ai,
	&m68000_base_device::m68k_op_muls_w_1_pi,
	&m68000_base_device::m68k_op_muls_w_1_pd,
	&m68000_base_device::m68k_op_muls_w_1_di,
	&m68000_base_device::m68k_op_muls_w_1_ix,
	&m68000_base_device::m68k_op_add_b_0,
	&m68000_base_device::m68k_op_add_b_1_ai,
	&m68000_base_device::m68k_op_add_b_1_pi,
	&m68000_base_device::m68k_op_add_b_1_pd,
	&m68000_base_device::m68k_op_add_b_1_di,
	&m68000_base_device::m68k_op_add_b_1_ix,
	&m68000_base_device::m68k_op_add_w_2,
	&m68000_base_device::m68k_op_add_w_3,
	&m68000_base_device::m68k_op_add_w_4_ai,
	&m68000_base_device::m68k_op_add_w_4_pi,
	&m68000_base_device::m68k_op_add_w_4_pd,
	&m68000_base_device::m68k_op_add_w_4_di,
	&m68000_base_device::m68k_op_add_w_4_ix,
	&m68000_base_device::m68k_op_add_l_5,
	&m68000_base_device::m68k_op_add_l_6,
	&m68000_base_device::m68k_op_add_l_7_ai,
	&m68000_base_device::m68k_op_add_l_7_pi,
	&m68000_base_device::m68k_op_add_l_7_pd,
	&m68000_base_device::m68k_op_add_l_7_di,
	&m68000_base_device::m68k_op_add_l_7_ix,
	&m68000_base_device::m68k_op_adda_w_0,
	&m68000_base_device::m68k_op_adda_w_1,
	&m68000_base_device::m68k_op_adda_w_2_ai,
	&m68000_base_device::m68k_op_adda_w_2_pi,
	&m68000_base_device::m68k_op_adda_w_2_pd,
	&m68000_base_device::m68k_op_adda_w_2_di,
	&m68000_base_device::m68k_op_adda_w_2_ix,
	&m68000_base_device::m68k_op_addx_b_0,
	&m68000_base_device::m68k_op_addx_b_6,
	&m68000_base_device::m68k_op_add_b_8_ai,
	&m68000_base_device::m68k_op_add_b_8_pi,
	&m68000_base_device::m68k_op_add_b_8_pd,
	&m68000_base_device::m68k_op_add_b_8_di,
	&m68000_base_device::m68k_op_add_b_8_ix,
	&m68000_base_device::m68k_op_addx_w_1,
	&m68000_base_device::m68k_op_addx_w_7,
	&m68000_base_device::m68k_op_add_w_9_ai,
	&m68000_base_device::m68k_op_add_w_9_pi,
	&m68000_base_device::m68k_op_add_w_9_pd,
	&m68000_base_device::m68k_op_add_w_9_di,
	&m68000_base_device::m68k_op_add_w_9_ix,
	&m68000_base_device::m68k_op_addx_l_2,
	&m68000_base_device::m68k_op_addx_l_8,
	&m68000_base_device::m68k_op_add_l_10_ai,
	&m68000_base_device::m68k_op_add_l_10_pi,
	&m68000_base_device::m68k_op_add_l_10_pd,
	&m68000_base_device::m68k_op_add_l_10_di,
	&m68000_base_device::m68k_op_add_l_10_ix,
	&m68000_base_device::m68k_op_adda_l_3,
	&m68000_base_device::m68k_op_adda_l_4,
	&m68000_base_device::m68k_op_adda_l_5_ai,
	&m68000_base_device::m68k_op_adda_l_5_pi,
	&m68000_base_device::m68k_op_adda_l_5_pd,
	&m68000_base_device::m68k_op_adda_l_5_di,
	&m68000_base_device::m68k_op_adda_l_5_ix,
	&m68000_base_device::m68k_op_asr_b_0,
	&m68000_base_device::m68k_op_lsr_b_0,
	&m68000_base_device::m68k_op_roxr_b_0,
	&m68000_base_device::m68k_op_ror_b_0,
	&m68000_base_device::m68k_op_asr_b_3,
	&m68000_base_device::m68k_op_lsr_b_3,
	&m68000_base_device::m68k_op_roxr_b_3,
	&m68000_base_device::m68k_op_ror_b_3,
	&m68000_base_device::m68k_op_asr_w_1,
	&m68000_base_device::m68k_op_lsr_w_1,
	&m68000_base_device::m68k_op_roxr_w_1,
	&m68000_base_device::m68k_op_ror_w_1,
	&m68000_base_device::m68k_op_asr_w_4,
	&m68000_base_device::m68k_op_lsr_w_4,
	&m68000_base_device::m68k_op_roxr_w_4,
	&m68000_base_device::m68k_op_ror_w_4,
	&m68000_base_device::m68k_op_asr_l_2,
	&m68000_base_device::m68k_op_lsr_l_2,
	&m68000_base_device::m68k_op_roxr_l_2,
	&m68000_base_device::m68k_op_ror_l_2,
	&m68000_base_device::m68k_op_asr_l_5,
	&m68000_base_device::m68k_op_lsr_l_5,
	&m68000_base_device::m68k_op_roxr_l_5,
	&m68000_base_device::m68k_op_ror_l_5,
	&m68000_base_device::m68k_op_asl_b_0,
	&m68000_base_device::m68k_op_lsl_b_0,
	&m68000_base_device::m68k_op_roxl_b_0,
	&m68000_base_device::m68k_op_rol_b_0,
	&m68000_base_device::m68k_op_asl_b_3,
	&m68000_base_device::m68k_op_lsl_b_3,
	&m68000_base_device::m68k_op_roxl_b_3,
	&m68000_base_device::m68k_op_rol_b_3,
	&m68000_base_device::m68k_op_asl_w_1,
	&m68000_base_device::m68k_op_lsl_w_1,
	&m68000_base_device::m68k_op_roxl_w_1,
	&m68000_base_device::m68k_op_rol_w_1,
	&m68000_base_device::m68k_op_asl_w_4,
	&m68000_base_device::m68k_op_lsl_w_4,
	&m68000_base_device::m68k_op_roxl_w_4,
	&m68000_base_device::m68k_op_rol_w_4,
	&m68000_base_device::m68k_op_asl_l_2,
	&m68000_base_device::m68k_op_lsl_l_2,
	&m68000_base_device::m68k_op_roxl_l_2,
	&m68000_base_device::m68k_op_rol_l_2,
	&m68000_base_device::m68k_op_asl_l_5,
	&m68000_base_device::m68k_op_lsl_l_5,
	&m68000_base_device::m68k_op_roxl_l_5,
	&m68000_base_device::m68k_op_rol_l_5,
	&m68000_base_device::m68k_op_cpdbcc_l_0,
	&m68000_base_device::m68k_op_cptrapcc_l_0,
	&m68000_base_device::m68k_op_ptest_l_0,
	&m68000_base_device::m68k_op_rtm_l_0,
	&m68000_base_device::m68k_op_trap_0,
	&m68000_base_device::m68k_op_btst_b_1_pi7,
	&m68000_base_device::m68k_op_btst_b_1_pd7,
	&m68000_base_device::m68k_op_btst_b_1_aw,
	&m68000_base_device::m68k_op_btst_b_1_al,
	&m68000_base_device::m68k_op_btst_b_1_pcdi,
	&m68000_base_device::m68k_op_btst_b_1_pcix,
	&m68000_base_device::m68k_op_btst_b_1_i,
	&m68000_base_device::m68k_op_bchg_b_1_pi7,
	&m68000_base_device::m68k_op_bchg_b_1_pd7,
	&m68000_base_device::m68k_op_bchg_b_1_aw,
	&m68000_base_device::m68k_op_bchg_b_1_al,
	&m68000_base_device::m68k_op_bclr_b_1_pi7,
	&m68000_base_device::m68k_op_bclr_b_1_pd7,
	&m68000_base_device::m68k_op_bclr_b_1_aw,
	&m68000_base_device::m68k_op_bclr_b_1_al,
	&m68000_base_device::m68k_op_bset_b_1_pi7,
	&m68000_base_device::m68k_op_bset_b_1_pd7,
	&m68000_base_device::m68k_op_bset_b_1_aw,
	&m68000_base_device::m68k_op_bset_b_1_al,
	&m68000_base_device::m68k_op_move_b_1_pi7,
	&m68000_base_device::m68k_op_move_b_1_pd7,
	&m68000_base_device::m68k_op_move_b_1_aw,
	&m68000_base_device::m68k_op_move_b_1_al,
	&m68000_base_device::m68k_op_move_b_1_pcdi,
	&m68000_base_device::m68k_op_move_b_1_pcix,
	&m68000_base_device::m68k_op_move_b_1_i,
	&m68000_base_device::m68k_op_move_b_3_pi7,
	&m68000_base_device::m68k_op_move_b_3_pd7,
	&m68000_base_device::m68k_op_move_b_3_aw,
	&m68000_base_device::m68k_op_move_b_3_al,
	&m68000_base_device::m68k_op_move_b_3_pcdi,
	&m68000_base_device::m68k_op_move_b_3_pcix,
	&m68000_base_device::m68k_op_move_b_3_i,
	&m68000_base_device::m68k_op_move_b_7_pi7,
	&m68000_base_device::m68k_op_move_b_7_pd7,
	&m68000_base_device::m68k_op_move_b_7_aw,
	&m68000_base_device::m68k_op_move_b_7_al,
	&m68000_base_device::m68k_op_move_b_7_pcdi,
	&m68000_base_device::m68k_op_move_b_7_pcix,
	&m68000_base_device::m68k_op_move_b_7_i,
	&m68000_base_device::m68k_op_move_b_11_pi7,
	&m68000_base_device::m68k_op_move_b_11_pd7,
	&m68000_base_device::m68k_op_move_b_11_aw,
	&m68000_base_device::m68k_op_move_b_11_al,
	&m68000_base_device::m68k_op_move_b_11_pcdi,
	&m68000_base_device::m68k_op_move_b_11_pcix,
	&m68000_base_device::m68k_op_move_b_11_i,
	&m68000_base_device::m68k_op_move_b_13_pi7,
	&m68000_base_device::m68k_op_move_b_13_pd7,
	&m68000_base_device::m68k_op_move_b_13_aw,
	&m68000_base_device::m68k_op_move_b_13_al,
	&m68000_base_device::m68k_op_move_b_13_pcdi,
	&m68000_base_device::m68k_op_move_b_13_pcix,
	&m68000_base_device::m68k_op_move_b_13_i,
	&m68000_base_device::m68k_op_move_b_15_pi7,
	&m68000_base_device::m68k_op_move_b_15_pd7,
	&m68000_base_device::m68k_op_move_b_15_aw,
	&m68000_base_device::m68k_op_move_b_15_al,
	&m68000_base_device::m68k_op_move_b_15_pcdi,
	&m68000_base_device::m68k_op_move_b_15_pcix,
	&m68000_base_device::m68k_op_move_b_15_i,
	&m68000_base_device::m68k_op_move_l_46_aw,
	&m68000_base_device::m68k_op_move_l_46_al,
	&m68000_base_device::m68k_op_move_l_46_pcdi,
	&m68000_base_device::m68k_op_move_l_46_pcix,
	&m68000_base_device::m68k_op_move_l_46_i,
	&m68000_base_device::m68k_op_movea_l_5_aw,
	&m68000_base_device::m68k_op_movea_l_5_al,
	&m68000_base_device::m68k_op_movea_l_5_pcdi,
	&m68000_base_device::m68k_op_movea_l_5_pcix,
	&m68000_base_device::m68k_op_movea_l_5_i,
	&m68000_base_device::m68k_op_move_l_49_aw,
	&m68000_base_device::m68k_op_move_l_49_al,
	&m68000_base_device::m68k_op_move_l_49_pcdi,
	&m68000_base_device::m68k_op_move_l_49_pcix,
	&m68000_base_device::m68k_op_move_l_49_i,
	&m68000_base_device::m68k_op_move_l_52_aw,
	&m68000_base_device::m68k_op_move_l_52_al,
	&m68000_base_device::m68k_op_move_l_52_pcdi,
	&m68000_base_device::m68k_op_move_l_52_pcix,
	&m68000_base_device::m68k_op_move_l_52_i,
	&m68000_base_device::m68k_op_move_l_55_aw,
	&m68000_base_device::m68k_op_move_l_55_al,
	&m68000_base_device::m68k_op_move_l_55_pcdi,
	&m68000_base_device::m68k_op_move_l_55_pcix,
	&m68000_base_device::m68k_op_move_l_55_i,
	&m68000_base_device::m68k_op_move_l_58_aw,
	&m68000_base_device::m68k_op_move_l_58_al,
	&m68000_base_device::m68k_op_move_l_58_pcdi,
	&m68000_base_device::m68k_op_move_l_58_pcix,
	&m68000_base_device::m68k_op_move_l_58_i,
	&m68000_base_device::m68k_op_move_l_61_aw,
	&m68000_base_device::m68k_op_move_l_61_al,
	&m68000_base_device::m68k_op_move_l_61_pcdi,
	&m68000_base_device::m68k_op_move_l_61_pcix,
	&m68000_base_device::m68k_op_move_l_61_i,
	&m68000_base_device::m68k_op_move_w_22_aw,
	&m68000_base_device::m68k_op_move_w_22_al,
	&m68000_base_device::m68k_op_move_w_22_pcdi,
	&m68000_base_device::m68k_op_move_w_22_pcix,
	&m68000_base_device::m68k_op_move_w_22_i,
	&m68000_base_device::m68k_op_movea_w_2_aw,
	&m68000_base_device::m68k_op_movea_w_2_al,
	&m68000_base_device::m68k_op_movea_w_2_pcdi,
	&m68000_base_device::m68k_op_movea_w_2_pcix,
	&m68000_base_device::m68k_op_movea_w_2_i,
	&m68000_base_device::m68k_op_move_w_25_aw,
	&m68000_base_device::m68k_op_move_w_25_al,
	&m68000_base_device::m68k_op_move_w_25_pcdi,
	&m68000_base_device::m68k_op_move_w_25_pcix,
	&m68000_base_device::m68k_op_move_w_25_i,
	&m68000_base_device::m68k_op_move_w_28_aw,
	&m68000_base_device::m68k_op_move_w_28_al,
	&m68000_base_device::m68k_op_move_w_28_pcdi,
	&m68000_base_device::m68k_op_move_w_28_pcix,
	&m68000_base_device::m68k_op_move_w_28_i,
	&m68000_base_device::m68k_op_move_w_31_aw,
	&m68000_base_device::m68k_op_move_w_31_al,
	&m68000_base_device::m68k_op_move_w_31_pcdi,
	&m68000_base_device::m68k_op_move_w_31_pcix,
	&m68000_base_device::m68k_op_move_w_31_i,
	&m68000_base_device::m68k_op_move_w_34_aw,
	&m68000_base_device::m68k_op_move_w_34_al,
	&m68000_base_device::m68k_op_move_w_34_pcdi,
	&m68000_base_device::m68k_op_move_w_34_pcix,
	&m68000_base_device::m68k_op_move_w_34_i,
	&m68000_base_device::m68k_op_move_w_37_aw,
	&m68000_base_device::m68k_op_move_w_37_al,
	&m68000_base_device::m68k_op_move_w_37_pcdi,
	&m68000_base_device::m68k_op_move_w_37_pcix,
	&m68000_base_device::m68k_op_move_w_37_i,
	&m68000_base_device::m68k_op_chk_l_3_aw,
	&m68000_base_device::m68k_op_chk_l_3_al,
	&m68000_base_device::m68k_op_chk_l_3_pcdi,
	&m68000_base_device::m68k_op_chk_l_3_pcix,
	&m68000_base_device::m68k_op_chk_l_3_i,
	&m68000_base_device::m68k_op_chk_w_1_aw,
	&m68000_base_device::m68k_op_chk_w_1_al,
	&m68000_base_device::m68k_op_chk_w_1_pcdi,
	&m68000_base_device::m68k_op_chk_w_1_pcix,
	&m68000_base_device::m68k_op_chk_w_1_i,
	&m68000_base_device::m68k_op_lea_l_0_aw,
	&m68000_base_device::m68k_op_lea_l_0_al,
	&m68000_base_device::m68k_op_lea_l_0_pcdi,
	&m68000_base_device::m68k_op_lea_l_0_pcix,
	&m68000_base_device::m68k_op_addq_b_1_pi7,
	&m68000_base_device::m68k_op_addq_b_1_pd7,
	&m68000_base_device::m68k_op_addq_b_1_aw,
	&m68000_base_device::m68k_op_addq_b_1_al,
	&m68000_base_device::m68k_op_addq_w_4_aw,
	&m68000_base_device::m68k_op_addq_w_4_al,
	&m68000_base_device::m68k_op_addq_l_7_aw,
	&m68000_base_device::m68k_op_addq_l_7_al,
	&m68000_base_device::m68k_op_subq_b_1_pi7,
	&m68000_base_device::m68k_op_subq_b_1_pd7,
	&m68000_base_device::m68k_op_subq_b_1_aw,
	&m68000_base_device::m68k_op_subq_b_1_al,
	&m68000_base_device::m68k_op_subq_w_4_aw,
	&m68000_base_device::m68k_op_subq_w_4_al,
	&m68000_base_device::m68k_op_subq_l_7_aw,
	&m68000_base_device::m68k_op_subq_l_7_al,
	&m68000_base_device::m68k_op_or_b_1_pi7,
	&m68000_base_device::m68k_op_or_b_1_pd7,
	&m68000_base_device::m68k_op_or_b_1_aw,
	&m68000_base_device::m68k_op_or_b_1_al,
	&m68000_base_device::m68k_op_or_b_1_pcdi,
	&m68000_base_device::m68k_op_or_b_1_pcix,
	&m68000_base_device::m68k_op_or_b_1_i,
	&m68000_base_device::m68k_op_or_w_3_aw,
	&m68000_base_device::m68k_op_or_w_3_al,
	&m68000_base_device::m68k_op_or_w_3_pcdi,
	&m68000_base_device::m68k_op_or_w_3_pcix,
	&m68000_base_device::m68k_op_or_w_3_i,
	&m68000_base_device::m68k_op_or_l_5_aw,
	&m68000_base_device::m68k_op_or_l_5_al,
	&m68000_base_device::m68k_op_or_l_5_pcdi,
	&m68000_base_device::m68k_op_or_l_5_pcix,
	&m68000_base_device::m68k_op_or_l_5_i,
	&m68000_base_device::m68k_op_divu_w_1_aw,
	&m68000_base_device::m68k_op_divu_w_1_al,
	&m68000_base_device::m68k_op_divu_w_1_pcdi,
	&m68000_base_device::m68k_op_divu_w_1_pcix,
	&m68000_base_device::m68k_op_divu_w_1_i,
	&m68000_base_device::m68k_op_sbcd_b_2,
	&m68000_base_device::m68k_op_or_b_6_pi7,
	&m68000_base_device::m68k_op_or_b_6_pd7,
	&m68000_base_device::m68k_op_or_b_6_aw,
	&m68000_base_device::m68k_op_or_b_6_al,
	&m68000_base_device::m68k_op_pack_w_2,
	&m68000_base_device::m68k_op_or_w_7_aw,
	&m68000_base_device::m68k_op_or_w_7_al,
	&m68000_base_device::m68k_op_unpk_w_2,
	&m68000_base_device::m68k_op_or_l_8_aw,
	&m68000_base_device::m68k_op_or_l_8_al,
	&m68000_base_device::m68k_op_divs_w_1_aw,
	&m68000_base_device::m68k_op_divs_w_1_al,
	&m68000_base_device::m68k_op_divs_w_1_pcdi,
	&m68000_base_device::m68k_op_divs_w_1_pcix,
	&m68000_base_device::m68k_op_divs_w_1_i,
	&m68000_base_device::m68k_op_sub_b_1_pi7,
	&m68000_base_device::m68k_op_sub_b_1_pd7,
	&m68000_base_device::m68k_op_sub_b_1_aw,
	&m68000_base_device::m68k_op_sub_b_1_al,
	&m68000_base_device::m68k_op_sub_b_1_pcdi,
	&m68000_base_device::m68k_op_sub_b_1_pcix,
	&m68000_base_device::m68k_op_sub_b_1_i,
	&m68000_base_device::m68k_op_sub_w_4_aw,
	&m68000_base_device::m68k_op_sub_w_4_al,
	&m68000_base_device::m68k_op_sub_w_4_pcdi,
	&m68000_base_device::m68k_op_sub_w_4_pcix,
	&m68000_base_device::m68k_op_sub_w_4_i,
	&m68000_base_device::m68k_op_sub_l_7_aw,
	&m68000_base_device::m68k_op_sub_l_7_al,
	&m68000_base_device::m68k_op_sub_l_7_pcdi,
	&m68000_base_device::m68k_op_sub_l_7_pcix,
	&m68000_base_device::m68k_op_sub_l_7_i,
	&m68000_base_device::m68k_op_suba_w_2_aw,
	&m68000_base_device::m68k_op_suba_w_2_al,
	&m68000_base_device::m68k_op_suba_w_2_pcdi,
	&m68000_base_device::m68k_op_suba_w_2_pcix,
	&m68000_base_device::m68k_op_suba_w_2_i,
	&m68000_base_device::m68k_op_subx_b_4,
	&m68000_base_device::m68k_op_sub_b_8_pi7,
	&m68000_base_device::m68k_op_sub_b_8_pd7,
	&m68000_base_device::m68k_op_sub_b_8_aw,
	&m68000_base_device::m68k_op_sub_b_8_al,
	&m68000_base_device::m68k_op_sub_w_9_aw,
	&m68000_base_device::m68k_op_sub_w_9_al,
	&m68000_base_device::m68k_op_sub_l_10_aw,
	&m68000_base_device::m68k_op_sub_l_10_al,
	&m68000_base_device::m68k_op_suba_l_5_aw,
	&m68000_base_device::m68k_op_suba_l_5_al,
	&m68000_base_device::m68k_op_suba_l_5_pcdi,
	&m68000_base_device::m68k_op_suba_l_5_pcix,
	&m68000_base_device::m68k_op_suba_l_5_i,
	&m68000_base_device::m68k_op_cmp_b_1_pi7,
	&m68000_base_device::m68k_op_cmp_b_1_pd7,
	&m68000_base_device::m68k_op_cmp_b_1_aw,
	&m68000_base_device::m68k_op_cmp_b_1_al,
	&m68000_base_device::m68k_op_cmp_b_1_pcdi,
	&m68000_base_device::m68k_op_cmp_b_1_pcix,
	&m68000_base_device::m68k_op_cmp_b_1_i,
	&m68000_base_device::m68k_op_cmp_w_4_aw,
	&m68000_base_device::m68k_op_cmp_w_4_al,
	&m68000_base_device::m68k_op_cmp_w_4_pcdi,
	&m68000_base_device::m68k_op_cmp_w_4_pcix,
	&m68000_base_device::m68k_op_cmp_w_4_i,
	&m68000_base_device::m68k_op_cmp_l_7_aw,
	&m68000_base_device::m68k_op_cmp_l_7_al,
	&m68000_base_device::m68k_op_cmp_l_7_pcdi,
	&m68000_base_device::m68k_op_cmp_l_7_pcix,
	&m68000_base_device::m68k_op_cmp_l_7_i,
	&m68000_base_device::m68k_op_cmpa_w_2_aw,
	&m68000_base_device::m68k_op_cmpa_w_2_al,
	&m68000_base_device::m68k_op_cmpa_w_2_pcdi,
	&m68000_base_device::m68k_op_cmpa_w_2_pcix,
	&m68000_base_device::m68k_op_cmpa_w_2_i,
	&m68000_base_device::m68k_op_cmpm_b_1,
	&m68000_base_device::m68k_op_eor_b_1_pi7,
	&m68000_base_device::m68k_op_eor_b_1_pd7,
	&m68000_base_device::m68k_op_eor_b_1_aw,
	&m68000_base_device::m68k_op_eor_b_1_al,
	&m68000_base_device::m68k_op_eor_w_3_aw,
	&m68000_base_device::m68k_op_eor_w_3_al,
	&m68000_base_device::m68k_op_eor_l_5_aw,
	&m68000_base_device::m68k_op_eor_l_5_al,
	&m68000_base_device::m68k_op_cmpa_l_5_aw,
	&m68000_base_device::m68k_op_cmpa_l_5_al,
	&m68000_base_device::m68k_op_cmpa_l_5_pcdi,
	&m68000_base_device::m68k_op_cmpa_l_5_pcix,
	&m68000_base_device::m68k_op_cmpa_l_5_i,
	&m68000_base_device::m68k_op_and_b_1_pi7,
	&m68000_base_device::m68k_op_and_b_1_pd7,
	&m68000_base_device::m68k_op_and_b_1_aw,
	&m68000_base_device::m68k_op_and_b_1_al,
	&m68000_base_device::m68k_op_and_b_1_pcdi,
	&m68000_base_device::m68k_op_and_b_1_pcix,
	&m68000_base_device::m68k_op_and_b_1_i,
	&m68000_base_device::m68k_op_and_w_3_aw,
	&m68000_base_device::m68k_op_and_w_3_al,
	&m68000_base_device::m68k_op_and_w_3_pcdi,
	&m68000_base_device::m68k_op_and_w_3_pcix,
	&m68000_base_device::m68k_op_and_w_3_i,
	&m68000_base_device::m68k_op_and_l_5_aw,
	&m68000_base_device::m68k_op_and_l_5_al,
	&m68000_base_device::m68k_op_and_l_5_pcdi,
	&m68000_base_device::m68k_op_and_l_5_pcix,
	&m68000_base_device::m68k_op_and_l_5_i,
	&m68000_base_device::m68k_op_mulu_w_1_aw,
	&m68000_base_device::m68k_op_mulu_w_1_al,
	&m68000_base_device::m68k_op_mulu_w_1_pcdi,
	&m68000_base_device::m68k_op_mulu_w_1_pcix,
	&m68000_base_device::m68k_op_mulu_w_1_i,
	&m68000_base_device::m68k_op_abcd_b_2,
	&m68000_base_device::m68k_op_and_b_6_pi7,
	&m68000_base_device::m68k_op_and_b_6_pd7,
	&m68000_base_device::m68k_op_and_b_6_aw,
	&m68000_base_device::m68k_op_and_b_6_al,
	&m68000_base_device::m68k_op_and_w_7_aw,
	&m68000_base_device::m68k_op_and_w_7_al,
	&m68000_base_device::m68k_op_and_l_8_aw,
	&m68000_base_device::m68k_op_and_l_8_al,
	&m68000_base_device::m68k_op_muls_w_1_aw,
	&m68000_base_device::m68k_op_muls_w_1_al,
	&m68000_base_device::m68k_op_muls_w_1_pcdi,
	&m68000_base_device::m68k_op_muls_w_1_pcix,
	&m68000_base_device::m68k_op_muls_w_1_i,
	&m68000_base_device::m68k_op_add_b_1_pi7,
	&m68000_base_device::m68k_op_add_b_1_pd7,
	&m68000_base_device::m68k_op_add_b_1_aw,
	&m68000_base_device::m68k_op_add_b_1_al,
	&m68000_base_device::m68k_op_add_b_1_pcdi,
	&m68000_base_device::m68k_op_add_b_1_pcix,
	&m68000_base_device::m68k_op_add_b_1_i,
	&m68000_base_device::m68k_op_add_w_4_aw,
	&m68000_base_device::m68k_op_add_w_4_al,
	&m68000_base_device::m68k_op_add_w_4_pcdi,
	&m68000_base_device::m68k_op_add_w_4_pcix,
	&m68000_base_device::m68k_op_add_w_4_i,
	&m68000_base_device::m68k_op_add_l_7_aw,
	&m68000_base_device::m68k_op_add_l_7_al,
	&m68000_base_device::m68k_op_add_l_7_pcdi,
	&m68000_base_device::m68k_op_add_l_7_pcix,
	&m68000_base_device::m68k_op_add_l_7_i,
	&m68000_base_device::m68k_op_adda_w_2_aw,
	&m68000_base_device::m68k_op_adda_w_2_al,
	&m68000_base_device::m68k_op_adda_w_2_pcdi,
	&m68000_base_device::m68k_op_adda_w_2_pcix,
	&m68000_base_device::m68k_op_adda_w_2_i,
	&m68000_base_device::m68k_op_addx_b_4,
	&m68000_base_device::m68k_op_add_b_8_pi7,
	&m68000_base_device::m68k_op_add_b_8_pd7,
	&m68000_base_device::m68k_op_add_b_8_aw,
	&m68000_base_device::m68k_op_add_b_8_al,
	&m68000_base_device::m68k_op_add_w_9_aw,
	&m68000_base_device::m68k_op_add_w_9_al,
	&m68000_base_device::m68k_op_add_l_10_aw,
	&m68000_base_device::m68k_op_add_l_10_al,
	&m68000_base_device::m68k_op_adda_l_5_aw,
	&m68000_base_device::m68k_op_adda_l_5_al,
	&m68000_base_device::m68k_op_adda_l_5_pcdi,
	&m68000_base_device::m68k_op_adda_l_5_pcix,
	&m68000_base_device::m68k_op_adda_l_5_i,
	&m68000_base_device::m68k_op_ori_b_0,
	&m68000_base_device::m68k_op_ori_b_1_ai,
	&m68000_base_device::m68k_op_ori_b_1_pi,
	&m68000_base_device::m68k_op_ori_b_1_pd,
	&m68000_base_device::m68k_op_ori_b_1_di,
	&m68000_base_device::m68k_op_ori_b_1_ix,
	&m68000_base_device::m68k_op_ori_w_2,
	&m68000_base_device::m68k_op_ori_w_3_ai,
	&m68000_base_device::m68k_op_ori_w_3_pi,
	&m68000_base_device::m68k_op_ori_w_3_pd,
	&m68000_base_device::m68k_op_ori_w_3_di,
	&m68000_base_device::m68k_op_ori_w_3_ix,
	&m68000_base_device::m68k_op_ori_l_4,
	&m68000_base_device::m68k_op_ori_l_5_ai,
	&m68000_base_device::m68k_op_ori_l_5_pi,
	&m68000_base_device::m68k_op_ori_l_5_pd,
	&m68000_base_device::m68k_op_ori_l_5_di,
	&m68000_base_device::m68k_op_ori_l_5_ix,
	&m68000_base_device::m68k_op_chk2cmp2_b_2_ai,
	&m68000_base_device::m68k_op_chk2cmp2_b_2_di,
	&m68000_base_device::m68k_op_chk2cmp2_b_2_ix,
	&m68000_base_device::m68k_op_andi_b_0,
	&m68000_base_device::m68k_op_andi_b_1_ai,
	&m68000_base_device::m68k_op_andi_b_1_pi,
	&m68000_base_device::m68k_op_andi_b_1_pd,
	&m68000_base_device::m68k_op_andi_b_1_di,
	&m68000_base_device::m68k_op_andi_b_1_ix,
	&m68000_base_device::m68k_op_andi_w_2,
	&m68000_base_device::m68k_op_andi_w_3_ai,
	&m68000_base_device::m68k_op_andi_w_3_pi,
	&m68000_base_device::m68k_op_andi_w_3_pd,
	&m68000_base_device::m68k_op_andi_w_3_di,
	&m68000_base_device::m68k_op_andi_w_3_ix,
	&m68000_base_device::m68k_op_andi_l_4,
	&m68000_base_device::m68k_op_andi_l_5_ai,
	&m68000_base_device::m68k_op_andi_l_5_pi,
	&m68000_base_device::m68k_op_andi_l_5_pd,
	&m68000_base_device::m68k_op_andi_l_5_di,
	&m68000_base_device::m68k_op_andi_l_5_ix,
	&m68000_base_device::m68k_op_chk2cmp2_w_5_ai,
	&m68000_base_device::m68k_op_chk2cmp2_w_5_di,
	&m68000_base_device::m68k_op_chk2cmp2_w_5_ix,
	&m68000_base_device::m68k_op_subi_b_0,
	&m68000_base_device::m68k_op_subi_b_1_ai,
	&m68000_base_device::m68k_op_subi_b_1_pi,
	&m68000_base_device::m68k_op_subi_b_1_pd,
	&m68000_base_device::m68k_op_subi_b_1_di,
	&m68000_base_device::m68k_op_subi_b_1_ix,
	&m68000_base_device::m68k_op_subi_w_2,
	&m68000_base_device::m68k_op_subi_w_3_ai,
	&m68000_base_device::m68k_op_subi_w_3_pi,
	&m68000_base_device::m68k_op_subi_w_3_pd,
	&m68000_base_device::m68k_op_subi_w_3_di,
	&m68000_base_device::m68k_op_subi_w_3_ix,
	&m68000_base_device::m68k_op_subi_l_4,
	&m68000_base_device::m68k_op_subi_l_5_ai,
	&m68000_base_device::m68k_op_subi_l_5_pi,
	&m68000_base_device::m68k_op_subi_l_5_pd,
	&m68000_base_device::m68k_op_subi_l_5_di,
	&m68000_base_device::m68k_op_subi_l_5_ix,
	&m68000_base_device::m68k_op_chk2cmp2_l_8_ai,
	&m68000_base_device::m68k_op_chk2cmp2_l_8_di,
	&m68000_base_device::m68k_op_chk2cmp2_l_8_ix,
	&m68000_base_device::m68k_op_addi_b_0,
	&m68000_base_device::m68k_op_addi_b_1_ai,
	&m68000_base_device::m68k_op_addi_b_1_pi,
	&m68000_base_device::m68k_op_addi_b_1_pd,
	&m68000_base_device::m68k_op_addi_b_1_di,
	&m68000_base_device::m68k_op_addi_b_1_ix,
	&m68000_base_device::m68k_op_addi_w_2,
	&m68000_base_device::m68k_op_addi_w_3_ai,
	&m68000_base_device::m68k_op_addi_w_3_pi,
	&m68000_base_device::m68k_op_addi_w_3_pd,
	&m68000_base_device::m68k_op_addi_w_3_di,
	&m68000_base_device::m68k_op_addi_w_3_ix,
	&m68000_base_device::m68k_op_addi_l_4,
	&m68000_base_device::m68k_op_addi_l_5_ai,
	&m68000_base_device::m68k_op_addi_l_5_pi,
	&m68000_base_device::m68k_op_addi_l_5_pd,
	&m68000_base_device::m68k_op_addi_l_5_di,
	&m68000_base_device::m68k_op_addi_l_5_ix,
	&m68000_base_device::m68k_op_callm_l_0_ai,
	&m68000_base_device::m68k_op_callm_l_0_di,
	&m68000_base_device::m68k_op_callm_l_0_ix,
	&m68000_base_device::m68k_op_btst_l_2,
	&m68000_base_device::m68k_op_btst_b_3_ai,
	&m68000_base_device::m68k_op_btst_b_3_pi,
	&m68000_base_device::m68k_op_btst_b_3_pd,
	&m68000_base_device::m68k_op_btst_b_3_di,
	&m68000_base_device::m68k_op_btst_b_3_ix,
	&m68000_base_device::m68k_op_bchg_l_2,
	&m68000_base_device::m68k_op_bchg_b_3_ai,
	&m68000_base_device::m68k_op_bchg_b_3_pi,
	&m68000_base_device::m68k_op_bchg_b_3_pd,
	&m68000_base_device::m68k_op_bchg_b_3_di,
	&m68000_base_device::m68k_op_bchg_b_3_ix,
	&m68000_base_device::m68k_op_bclr_l_2,
	&m68000_base_device::m68k_op_bclr_b_3_ai,
	&m68000_base_device::m68k_op_bclr_b_3_pi,
	&m68000_base_device::m68k_op_bclr_b_3_pd,
	&m68000_base_device::m68k_op_bclr_b_3_di,
	&m68000_base_device::m68k_op_bclr_b_3_ix,
	&m68000_base_device::m68k_op_bset_l_2,
	&m68000_base_device::m68k_op_bset_b_3_ai,
	&m68000_base_device::m68k_op_bset_b_3_pi,
	&m68000_base_device::m68k_op_bset_b_3_pd,
	&m68000_base_device::m68k_op_bset_b_3_di,
	&m68000_base_device::m68k_op_bset_b_3_ix,
	&m68000_base_device::m68k_op_eori_b_0,
	&m68000_base_device::m68k_op_eori_b_1_ai,
	&m68000_base_device::m68k_op_eori_b_1_pi,
	&m68000_base_device::m68k_op_eori_b_1_pd,
	&m68000_base_device::m68k_op_eori_b_1_di,
	&m68000_base_device::m68k_op_eori_b_1_ix,
	&m68000_base_device::m68k_op_eori_w_2,
	&m68000_base_device::m68k_op_eori_w_3_ai,
	&m68000_base_device::m68k_op_eori_w_3_pi,
	&m68000_base_device::m68k_op_eori_w_3_pd,
	&m68000_base_device::m68k_op_eori_w_3_di,
	&m68000_base_device::m68k_op_eori_w_3_ix,
	&m68000_base_device::m68k_op_eori_l_4,
	&m68000_base_device::m68k_op_eori_l_5_ai,
	&m68000_base_device::m68k_op_eori_l_5_pi,
	&m68000_base_device::m68k_op_eori_l_5_pd,
	&m68000_base_device::m68k_op_eori_l_5_di,
	&m68000_base_device::m68k_op_eori_l_5_ix,
	&m68000_base_device::m68k_op_cas_b_0_ai,
	&m68000_base_device::m68k_op_cas_b_0_pi,
	&m68000_base_device::m68k_op_cas_b_0_pd,
	&m68000_base_device::m68k_op_cas_b_0_di,
	&m68000_base_device::m68k_op_cas_b_0_ix,
	&m68000_base_device::m68k_op_cmpi_b_0,
	&m68000_base_device::m68k_op_cmpi_b_1_ai,
	&m68000_base_device::m68k_op_cmpi_b_1_pi,
	&m68000_base_device::m68k_op_cmpi_b_1_pd,
	&m68000_base_device::m68k_op_cmpi_b_1_di,
	&m68000_base_device::m68k_op_cmpi_b_1_ix,
	&m68000_base_device::m68k_op_cmpi_w_4,
	&m68000_base_device::m68k_op_cmpi_w_5_ai,
	&m68000_base_device::m68k_op_cmpi_w_5_pi,
	&m68000_base_device::m68k_op_cmpi_w_5_pd,
	&m68000_base_device::m68k_op_cmpi_w_5_di,
	&m68000_base_device::m68k_op_cmpi_w_5_ix,
	&m68000_base_device::m68k_op_cmpi_l_8,
	&m68000_base_device::m68k_op_cmpi_l_9_ai,
	&m68000_base_device::m68k_op_cmpi_l_9_pi,
	&m68000_base_device::m68k_op_cmpi_l_9_pd,
	&m68000_base_device::m68k_op_cmpi_l_9_di,
	&m68000_base_device::m68k_op_cmpi_l_9_ix,
	&m68000_base_device::m68k_op_cas_w_1_ai,
	&m68000_base_device::m68k_op_cas_w_1_pi,
	&m68000_base_device::m68k_op_cas_w_1_pd,
	&m68000_base_device::m68k_op_cas_w_1_di,
	&m68000_base_device::m68k_op_cas_w_1_ix,
	&m68000_base_device::m68k_op_moves_b_0_ai,
	&m68000_base_device::m68k_op_moves_b_0_pi,
	&m68000_base_device::m68k_op_moves_b_0_pd,
	&m68000_base_device::m68k_op_moves_b_0_di,
	&m68000_base_device::m68k_op_moves_b_0_ix,
	&m68000_base_device::m68k_op_moves_w_1_ai,
	&m68000_base_device::m68k_op_moves_w_1_pi,
	&m68000_base_device::m68k_op_moves_w_1_pd,
	&m68000_base_device::m68k_op_moves_w_1_di,
	&m68000_base_device::m68k_op_moves_w_1_ix,
	&m68000_base_device::m68k_op_moves_l_2_ai,
	&m68000_base_device::m68k_op_moves_l_2_pi,
	&m68000_base_device::m68k_op_moves_l_2_pd,
	&m68000_base_device::m68k_op_moves_l_2_di,
	&m68000_base_device::m68k_op_moves_l_2_ix,
	&m68000_base_device::m68k_op_cas_l_2_ai,
	&m68000_base_device::m68k_op_cas_l_2_pi,
	&m68000_base_device::m68k_op_cas_l_2_pd,
	&m68000_base_device::m68k_op_cas_l_2_di,
	&m68000_base_device::m68k_op_cas_l_2_ix,
	&m68000_base_device::m68k_op_move_b_16,
	&m68000_base_device::m68k_op_move_b_17_ai,
	&m68000_base_device::m68k_op_move_b_17_pi,
	&m68000_base_device::m68k_op_move_b_17_pd,
	&m68000_base_device::m68k_op_move_b_17_di,
	&m68000_base_device::m68k_op_move_b_17_ix,
	&m68000_base_device::m68k_op_move_b_18,
	&m68000_base_device::m68k_op_move_b_19_ai,
	&m68000_base_device::m68k_op_move_b_19_pi,
	&m68000_base_device::m68k_op_move_b_19_pd,
	&m68000_base_device::m68k_op_move_b_19_di,
	&m68000_base_device::m68k_op_move_b_19_ix,
	&m68000_base_device::m68k_op_move_b_4,
	&m68000_base_device::m68k_op_move_b_6_ai,
	&m68000_base_device::m68k_op_move_b_6_pi,
	&m68000_base_device::m68k_op_move_b_6_pd,
	&m68000_base_device::m68k_op_move_b_6_di,
	&m68000_base_device::m68k_op_move_b_6_ix,
	&m68000_base_device::m68k_op_move_b_8,
	&m68000_base_device::m68k_op_move_b_10_ai,
	&m68000_base_device::m68k_op_move_b_10_pi,
	&m68000_base_device::m68k_op_move_b_10_pd,
	&m68000_base_device::m68k_op_move_b_10_di,
	&m68000_base_device::m68k_op_move_b_10_ix,
	&m68000_base_device::m68k_op_move_l_62,
	&m68000_base_device::m68k_op_move_l_63,
	&m68000_base_device::m68k_op_move_l_64_ai,
	&m68000_base_device::m68k_op_move_l_64_pi,
	&m68000_base_device::m68k_op_move_l_64_pd,
	&m68000_base_device::m68k_op_move_l_64_di,
	&m68000_base_device::m68k_op_move_l_64_ix,
	&m68000_base_device::m68k_op_move_l_65,
	&m68000_base_device::m68k_op_move_l_66,
	&m68000_base_device::m68k_op_move_l_67_ai,
	&m68000_base_device::m68k_op_move_l_67_pi,
	&m68000_base_device::m68k_op_move_l_67_pd,
	&m68000_base_device::m68k_op_move_l_67_di,
	&m68000_base_device::m68k_op_move_l_67_ix,
	&m68000_base_device::m68k_op_move_w_38,
	&m68000_base_device::m68k_op_move_w_39,
	&m68000_base_device::m68k_op_move_w_40_ai,
	&m68000_base_device::m68k_op_move_w_40_pi,
	&m68000_base_device::m68k_op_move_w_40_pd,
	&m68000_base_device::m68k_op_move_w_40_di,
	&m68000_base_device::m68k_op_move_w_40_ix,
	&m68000_base_device::m68k_op_move_w_41,
	&m68000_base_device::m68k_op_move_w_42,
	&m68000_base_device::m68k_op_move_w_43_ai,
	&m68000_base_device::m68k_op_move_w_43_pi,
	&m68000_base_device::m68k_op_move_w_43_pd,
	&m68000_base_device::m68k_op_move_w_43_di,
	&m68000_base_device::m68k_op_move_w_43_ix,
	&m68000_base_device::m68k_op_negx_b_0,
	&m68000_base_device::m68k_op_negx_b_1_ai,
	&m68000_base_device::m68k_op_negx_b_1_pi,
	&m68000_base_device::m68k_op_negx_b_1_pd,
	&m68000_base_device::m68k_op_negx_b_1_di,
	&m68000_base_device::m68k_op_negx_b_1_ix,
	&m68000_base_device::m68k_op_negx_w_2,
	&m68000_base_device::m68k_op_negx_w_3_ai,
	&m68000_base_device::m68k_op_negx_w_3_pi,
	&m68000_base_device::m68k_op_negx_w_3_pd,
	&m68000_base_device::m68k_op_negx_w_3_di,
	&m68000_base_device::m68k_op_negx_w_3_ix,
	&m68000_base_device::m68k_op_negx_l_4,
	&m68000_base_device::m68k_op_negx_l_5_ai,
	&m68000_base_device::m68k_op_negx_l_5_pi,
	&m68000_base_device::m68k_op_negx_l_5_pd,
	&m68000_base_device::m68k_op_negx_l_5_di,
	&m68000_base_device::m68k_op_negx_l_5_ix,
	&m68000_base_device::m68k_op_move_w_72,
	&m68000_base_device::m68k_op_move_w_73_ai,
	&m68000_base_device::m68k_op_move_w_73_pi,
	&m68000_base_device::m68k_op_move_w_73_pd,
	&m68000_base_device::m68k_op_move_w_73_di,
	&m68000_base_device::m68k_op_move_w_73_ix,
	&m68000_base_device::m68k_op_clr_b_0,
	&m68000_base_device::m68k_op_clr_b_1_ai,
	&m68000_base_device::m68k_op_clr_b_1_pi,
	&m68000_base_device::m68k_op_clr_b_1_pd,
	&m68000_base_device::m68k_op_clr_b_1_di,
	&m68000_base_device::m68k_op_clr_b_1_ix,
	&m68000_base_device::m68k_op_clr_w_2,
	&m68000_base_device::m68k_op_clr_w_3_ai,
	&m68000_base_device::m68k_op_clr_w_3_pi,
	&m68000_base_device::m68k_op_clr_w_3_pd,
	&m68000_base_device::m68k_op_clr_w_3_di,
	&m68000_base_device::m68k_op_clr_w_3_ix,
	&m68000_base_device::m68k_op_clr_l_4,
	&m68000_base_device::m68k_op_clr_l_5_ai,
	&m68000_base_device::m68k_op_clr_l_5_pi,
	&m68000_base_device::m68k_op_clr_l_5_pd,
	&m68000_base_device::m68k_op_clr_l_5_di,
	&m68000_base_device::m68k_op_clr_l_5_ix,
	&m68000_base_device::m68k_op_move_w_68,
	&m68000_base_device::m68k_op_move_w_69_ai,
	&m68000_base_device::m68k_op_move_w_69_pi,
	&m68000_base_device::m68k_op_move_w_69_pd,
	&m68000_base_device::m68k_op_move_w_69_di,
	&m68000_base_device::m68k_op_move_w_69_ix,
	&m68000_base_device::m68k_op_neg_b_0,
	&m68000_base_device::m68k_op_neg_b_1_ai,
	&m68000_base_device::m68k_op_neg_b_1_pi,
	&m68000_base_device::m68k_op_neg_b_1_pd,
	&m68000_base_device::m68k_op_neg_b_1_di,
	&m68000_base_device::m68k_op_neg_b_1_ix,
	&m68000_base_device::m68k_op_neg_w_2,
	&m68000_base_device::m68k_op_neg_w_3_ai,
	&m68000_base_device::m68k_op_neg_w_3_pi,
	&m68000_base_device::m68k_op_neg_w_3_pd,
	&m68000_base_device::m68k_op_neg_w_3_di,
	&m68000_base_device::m68k_op_neg_w_3_ix,
	&m68000_base_device::m68k_op_neg_l_4,
	&m68000_base_device::m68k_op_neg_l_5_ai,
	&m68000_base_device::m68k_op_neg_l_5_pi,
	&m68000_base_device::m68k_op_neg_l_5_pd,
	&m68000_base_device::m68k_op_neg_l_5_di,
	&m68000_base_device::m68k_op_neg_l_5_ix,
	&m68000_base_device::m68k_op_move_w_70,
	&m68000_base_device::m68k_op_move_w_71_ai,
	&m68000_base_device::m68k_op_move_w_71_pi,
	&m68000_base_device::m68k_op_move_w_71_pd,
	&m68000_base_device::m68k_op_move_w_71_di,
	&m68000_base_device::m68k_op_move_w_71_ix,
	&m68000_base_device::m68k_op_not_b_0,
	&m68000_base_device::m68k_op_not_b_1_ai,
	&m68000_base_device::m68k_op_not_b_1_pi,
	&m68000_base_device::m68k_op_not_b_1_pd,
	&m68000_base_device::m68k_op_not_b_1_di,
	&m68000_base_device::m68k_op_not_b_1_ix,
	&m68000_base_device::m68k_op_not_w_2,
	&m68000_base_device::m68k_op_not_w_3_ai,
	&m68000_base_device::m68k_op_not_w_3_pi,
	&m68000_base_device::m68k_op_not_w_3_pd,
	&m68000_base_device::m68k_op_not_w_3_di,
	&m68000_base_device::m68k_op_not_w_3_ix,
	&m68000_base_device::m68k_op_not_l_4,
	&m68000_base_device::m68k_op_not_l_5_ai,
	&m68000_base_device::m68k_op_not_l_5_pi,
	&m68000_base_device::m68k_op_not_l_5_pd,
	&m68000_base_device::m68k_op_not_l_5_di,
	&m68000_base_device::m68k_op_not_l_5_ix,
	&m68000_base_device::m68k_op_move_w_74,
	&m68000_base_device::m68k_op_move_w_75_ai,
	&m68000_base_device::m68k_op_move_w_75_pi,
	&m68000_base_device::m68k_op_move_w_75_pd,
	&m68000_base_device::m68k_op_move_w_75_di,
	&m68000_base_device::m68k_op_move_w_75_ix,
	&m68000_base_device::m68k_op_nbcd_b_0,
	&m68000_base_device::m68k_op_link_l_3,
	&m68000_base_device::m68k_op_nbcd_b_1_ai,
	&m68000_base_device::m68k_op_nbcd_b_1_pi,
	&m68000_base_device::m68k_op_nbcd_b_1_pd,
	&m68000_base_device::m68k_op_nbcd_b_1_di,
	&m68000_base_device::m68k_op_nbcd_b_1_ix,
	&m68000_base_device::m68k_op_swap_l_0,
	&m68000_base_device::m68k_op_bkpt_0,
	&m68000_base_device::m68k_op_pea_l_0_ai,
	&m68000_base_device::m68k_op_pea_l_0_di,
	&m68000_base_device::m68k_op_pea_l_0_ix,
	&m68000_base_device::m68k_op_ext_w_0,
	&m68000_base_device::m68k_op_movem_w_1_ai,
	&m68000_base_device::m68k_op_movem_w_0,
	&m68000_base_device::m68k_op_movem_w_1_di,
	&m68000_base_device::m68k_op_movem_w_1_ix,
	&m68000_base_device::m68k_op_ext_l_1,
	&m68000_base_device::m68k_op_movem_l_3_ai,
	&m68000_base_device::m68k_op_movem_l_2,
	&m68000_base_device::m68k_op_movem_l_3_di,
	&m68000_base_device::m68k_op_movem_l_3_ix,
	&m68000_base_device::m68k_op_extb_l_0,
	&m68000_base_device::m68k_op_tst_b_0,
	&m68000_base_device::m68k_op_tst_b_1_ai,
	&m68000_base_device::m68k_op_tst_b_1_pi,
	&m68000_base_device::m68k_op_tst_b_1_pd,
	&m68000_base_device::m68k_op_tst_b_1_di,
	&m68000_base_device::m68k_op_tst_b_1_ix,
	&m68000_base_device::m68k_op_tst_w_5,
	&m68000_base_device::m68k_op_tst_w_6,
	&m68000_base_device::m68k_op_tst_w_7_ai,
	&m68000_base_device::m68k_op_tst_w_7_pi,
	&m68000_base_device::m68k_op_tst_w_7_pd,
	&m68000_base_device::m68k_op_tst_w_7_di,
	&m68000_base_device::m68k_op_tst_w_7_ix,
	&m68000_base_device::m68k_op_tst_l_11,
	&m68000_base_device::m68k_op_tst_l_12,
	&m68000_base_device::m68k_op_tst_l_13_ai,
	&m68000_base_device::m68k_op_tst_l_13_pi,
	&m68000_base_device::m68k_op_tst_l_13_pd,
	&m68000_base_device::m68k_op_tst_l_13_di,
	&m68000_base_device::m68k_op_tst_l_13_ix,
	&m68000_base_device::m68k_op_tas_b_0,
	&m68000_base_device::m68k_op_tas_b_1_ai,
	&m68000_base_device::m68k_op_tas_b_1_pi,
	&m68000_base_device::m68k_op_tas_b_1_pd,
	&m68000_base_device::m68k_op_tas_b_1_di,
	&m68000_base_device::m68k_op_tas_b_1_ix,
	&m68000_base_device::m68k_op_mull_l_0,
	&m68000_base_device::m68k_op_mull_l_1_ai,
	&m68000_base_device::m68k_op_mull_l_1_pi,
	&m68000_base_device::m68k_op_mull_l_1_pd,
	&m68000_base_device::m68k_op_mull_l_1_di,
	&m68000_base_device::m68k_op_mull_l_1_ix,
	&m68000_base_device::m68k_op_divl_l_0,
	&m68000_base_device::m68k_op_divl_l_1_ai,
	&m68000_base_device::m68k_op_divl_l_1_pi,
	&m68000_base_device::m68k_op_divl_l_1_pd,
	&m68000_base_device::m68k_op_divl_l_1_di,
	&m68000_base_device::m68k_op_divl_l_1_ix,
	&m68000_base_device::m68k_op_movem_w_7_ai,
	&m68000_base_device::m68k_op_movem_w_4,
	&m68000_base_device::m68k_op_movem_w_7_di,
	&m68000_base_device::m68k_op_movem_w_7_ix,
	&m68000_base_device::m68k_op_movem_l_11_ai,
	&m68000_base_device::m68k_op_movem_l_8,
	&m68000_base_device::m68k_op_movem_l_11_di,
	&m68000_base_device::m68k_op_movem_l_11_ix,
	&m68000_base_device::m68k_op_link_w_1,
	&m68000_base_device::m68k_op_unlk_l_1,
	&m68000_base_device::m68k_op_move_l_77,
	&m68000_base_device::m68k_op_move_l_76,
	&m68000_base_device::m68k_op_jsr_l_0_ai,
	&m68000_base_device::m68k_op_jsr_l_0_di,
	&m68000_base_device::m68k_op_jsr_l_0_ix,
	&m68000_base_device::m68k_op_jmp_l_0_ai,
	&m68000_base_device::m68k_op_jmp_l_0_di,
	&m68000_base_device::m68k_op_jmp_l_0_ix,
	&m68000_base_device::m68k_op_st_b_0,
	&m68000_base_device::m68k_op_dbt_w_0,
	&m68000_base_device::m68k_op_st_b_1_ai,
	&m68000_base_device::m68k_op_st_b_1_pi,
	&m68000_base_device::m68k_op_st_b_1_pd,
	&m68000_base_device::m68k_op_st_b_1_di,
	&m68000_base_device::m68k_op_st_b_1_ix,
	&m68000_base_device::m68k_op_sf_b_0,
	&m68000_base_device::m68k_op_dbf_w_0,
	&m68000_base_device::m68k_op_sf_b_1_ai,
	&m68000_base_device::m68k_op_sf_b_1_pi,
	&m68000_base_device::m68k_op_sf_b_1_pd,
	&m68000_base_device::m68k_op_sf_b_1_di,
	&m68000_base_device::m68k_op_sf_b_1_ix,
	&m68000_base_device::m68k_op_shi_b_0,
	&m68000_base_device::m68k_op_dbhi_w_0,
	&m68000_base_device::m68k_op_shi_b_1_ai,
	&m68000_base_device::m68k_op_shi_b_1_pi,
	&m68000_base_device::m68k_op_shi_b_1_pd,
	&m68000_base_device::m68k_op_shi_b_1_di,
	&m68000_base_device::m68k_op_shi_b_1_ix,
	&m68000_base_device::m68k_op_sls_b_0,
	&m68000_base_device::m68k_op_dbls_w_0,
	&m68000_base_device::m68k_op_sls_b_1_ai,
	&m68000_base_device::m68k_op_sls_b_1_pi,
	&m68000_base_device::m68k_op_sls_b_1_pd,
	&m68000_base_device::m68k_op_sls_b_1_di,
	&m68000_base_device::m68k_op_sls_b_1_ix,
	&m68000_base_device::m68k_op_scc_b_2,
	&m68000_base_device::m68k_op_dbcc_w_1,
	&m68000_base_device::m68k_op_scc_b_3_ai,
	&m68000_base_device::m68k_op_scc_b_3_pi,
	&m68000_base_device::m68k_op_scc_b_3_pd,
	&m68000_base_device::m68k_op_scc_b_3_di,
	&m68000_base_device::m68k_op_scc_b_3_ix,
	&m68000_base_device::m68k_op_scs_b_0,
	&m68000_base_device::m68k_op_dbcs_w_0,
	&m68000_base_device::m68k_op_scs_b_1_ai,
	&m68000_base_device::m68k_op_scs_b_1_pi,
	&m68000_base_device::m68k_op_scs_b_1_pd,
	&m68000_base_device::m68k_op_scs_b_1_di,
	&m68000_base_device::m68k_op_scs_b_1_ix,
	&m68000_base_device::m68k_op_sne_b_0,
	&m68000_base_device::m68k_op_dbne_w_0,
	&m68000_base_device::m68k_op_sne_b_1_ai,
	&m68000_base_device::m68k_op_sne_b_1_pi,
	&m68000_base_device::m68k_op_sne_b_1_pd,
	&m68000_base_device::m68k_op_sne_b_1_di,
	&m68000_base_device::m68k_op_sne_b_1_ix,
	&m68000_base_device::m68k_op_seq_b_0,
	&m68000_base_device::m68k_op_dbeq_w_0,
	&m68000_base_device::m68k_op_seq_b_1_ai,
	&m68000_base_device::m68k_op_seq_b_1_pi,
	&m68000_base_device::m68k_op_seq_b_1_pd,
	&m68000_base_device::m68k_op_seq_b_1_di,
	&m68000_base_device::m68k_op_seq_b_1_ix,
	&m68000_base_device::m68k_op_svc_b_0,
	&m68000_base_device::m68k_op_dbvc_w_0,
	&m68000_base_device::m68k_op_svc_b_1_ai,
	&m68000_base_device::m68k_op_svc_b_1_pi,
	&m68000_base_device::m68k_op_svc_b_1_pd,
	&m68000_base_device::m68k_op_svc_b_1_di,
	&m68000_base_device::m68k_op_svc_b_1_ix,
	&m68000_base_device::m68k_op_svs_b_0,
	&m68000_base_device::m68k_op_dbvs_w_0,
	&m68000_base_device::m68k_op_svs_b_1_ai,
	&m68000_base_device::m68k_op_svs_b_1_pi,
	&m68000_base_device::m68k_op_svs_b_1_pd,
	&m68000_base_device::m68k_op_svs_b_1_di,
	&m68000_base_device::m68k_op_svs_b_1_ix,
	&m68000_base_device::m68k_op_spl_b_0,
	&m68000_base_device::m68k_op_dbpl_w_0,
	&m68000_base_device::m68k_op_spl_b_1_ai,
	&m68000_base_device::m68k_op_spl_b_1_pi,
	&m68000_base_device::m68k_op_spl_b_1_pd,
	&m68000_base_device::m68k_op_spl_b_1_di,
	&m68000_base_device::m68k_op_spl_b_1_ix,
	&m68000_base_device::m68k_op_smi_b_0,
	&m68000_base_device::m68k_op_dbmi_w_0,
	&m68000_base_device::m68k_op_smi_b_1_ai,
	&m68000_base_device::m68k_op_smi_b_1_pi,
	&m68000_base_device::m68k_op_smi_b_1_pd,
	&m68000_base_device::m68k_op_smi_b_1_di,
	&m68000_base_device::m68k_op_smi_b_1_ix,
	&m68000_base_device::m68k_op_sge_b_0,
	&m68000_base_device::m68k_op_dbge_w_0,
	&m68000_base_device::m68k_op_sge_b_1_ai,
	&m68000_base_device::m68k_op_sge_b_1_pi,
	&m68000_base_device::m68k_op_sge_b_1_pd,
	&m68000_base_device::m68k_op_sge_b_1_di,
	&m68000_base_device::m68k_op_sge_b_1_ix,
	&m68000_base_device::m68k_op_slt_b_0,
	&m68000_base_device::m68k_op_dblt_w_0,
	&m68000_base_device::m68k_op_slt_b_1_ai,
	&m68000_base_device::m68k_op_slt_b_1_pi,
	&m68000_base_device::m68k_op_slt_b_1_pd,
	&m68000_base_device::m68k_op_slt_b_1_di,
	&m68000_base_device::m68k_op_slt_b_1_ix,
	&m68000_base_device::m68k_op_sgt_b_0,
	&m68000_base_device::m68k_op_dbgt_w_0,
	&m68000_base_device::m68k_op_sgt_b_1_ai,
	&m68000_base_device::m68k_op_sgt_b_1_pi,
	&m68000_base_device::m68k_op_sgt_b_1_pd,
	&m68000_base_device::m68k_op_sgt_b_1_di,
	&m68000_base_device::m68k_op_sgt_b_1_ix,
	&m68000_base_device::m68k_op_sle_b_0,
	&m68000_base_device::m68k_op_dble_w_0,
	&m68000_base_device::m68k_op_sle_b_1_ai,
	&m68000_base_device::m68k_op_sle_b_1_pi,
	&m68000_base_device::m68k_op_sle_b_1_pd,
	&m68000_base_device::m68k_op_sle_b_1_di,
	&m68000_base_device::m68k_op_sle_b_1_ix,
	&m68000_base_device::m68k_op_sbcd_b_1,
	&m68000_base_device::m68k_op_pack_w_1,
	&m68000_base_device::m68k_op_unpk_w_1,
	&m68000_base_device::m68k_op_subx_b_3,
	&m68000_base_device::m68k_op_cmpm_b_0,
	&m68000_base_device::m68k_op_abcd_b_1,
	&m68000_base_device::m68k_op_addx_b_3,
	&m68000_base_device::m68k_op_asr_w_6_ai,
	&m68000_base_device::m68k_op_asr_w_6_pi,
	&m68000_base_device::m68k_op_asr_w_6_pd,
	&m68000_base_device::m68k_op_asr_w_6_di,
	&m68000_base_device::m68k_op_asr_w_6_ix,
	&m68000_base_device::m68k_op_asl_w_6_ai,
	&m68000_base_device::m68k_op_asl_w_6_pi,
	&m68000_base_device::m68k_op_asl_w_6_pd,
	&m68000_base_device::m68k_op_asl_w_6_di,
	&m68000_base_device::m68k_op_asl_w_6_ix,
	&m68000_base_device::m68k_op_lsr_w_6_ai,
	&m68000_base_device::m68k_op_lsr_w_6_pi,
	&m68000_base_device::m68k_op_lsr_w_6_pd,
	&m68000_base_device::m68k_op_lsr_w_6_di,
	&m68000_base_device::m68k_op_lsr_w_6_ix,
	&m68000_base_device::m68k_op_lsl_w_6_ai,
	&m68000_base_device::m68k_op_lsl_w_6_pi,
	&m68000_base_device::m68k_op_lsl_w_6_pd,
	&m68000_base_device::m68k_op_lsl_w_6_di,
	&m68000_base_device::m68k_op_lsl_w_6_ix,
	&m68000_base_device::m68k_op_roxr_w_6_ai,
	&m68000_base_device::m68k_op_roxr_w_6_pi,
	&m68000_base_device::m68k_op_roxr_w_6_pd,
	&m68000_base_device::m68k_op_roxr_w_6_di,
	&m68000_base_device::m68k_op_roxr_w_6_ix,
	&m68000_base_device::m68k_op_roxl_w_6_ai,
	&m68000_base_device::m68k_op_roxl_w_6_pi,
	&m68000_base_device::m68k_op_roxl_w_6_pd,
	&m68000_base_device::m68k_op_roxl_w_6_di,
	&m68000_base_device::m68k_op_roxl_w_6_ix,
	&m68000_base_device::m68k_op_ror_w_6_ai,
	&m68000_base_device::m68k_op_ror_w_6_pi,
	&m68000_base_device::m68k_op_ror_w_6_pd,
	&m68000_base_device::m68k_op_ror_w_6_di,
	&m68000_base_device::m68k_op_ror_w_6_ix,
	&m68000_base_device::m68k_op_rol_w_6_ai,
	&m68000_base_device::m68k_op_rol_w_6_pi,
	&m68000_base_device::m68k_op_rol_w_6_pd,
	&m68000_base_device::m68k_op_rol_w_6_di,
	&m68000_base_device::m68k_op_rol_w_6_ix,
	&m68000_base_device::m68k_op_bftst_l_0,
	&m68000_base_device::m68k_op_bftst_l_1_ai,
	&m68000_base_device::m68k_op_bftst_l_1_di,
	&m68000_base_device::m68k_op_bftst_l_1_ix,
	&m68000_base_device::m68k_op_bfextu_l_0,
	&m68000_base_device::m68k_op_bfextu_l_1_ai,
	&m68000_base_device::m68k_op_bfextu_l_1_di,
	&m68000_base_device::m68k_op_bfextu_l_1_ix,
	&m68000_base_device::m68k_op_bfchg_l_0,
	&m68000_base_device::m68k_op_bfchg_l_1_ai,
	&m68000_base_device::m68k_op_bfchg_l_1_di,
	&m68000_base_device::m68k_op_bfchg_l_1_ix,
	&m68000_base_device::m68k_op_bfexts_l_0,
	&m68000_base_device::m68k_op_bfexts_l_1_ai,
	&m68000_base_device::m68k_op_bfexts_l_1_di,
	&m68000_base_device::m68k_op_bfexts_l_1_ix,
	&m68000_base_device::m68k_op_bfclr_l_0,
	&m68000_base_device::m68k_op_bfclr_l_1_ai,
	&m68000_base_device::m68k_op_bfclr_l_1_di,
	&m68000_base_device::m68k_op_bfclr_l_1_ix,
	&m68000_base_device::m68k_op_bfffo_l_0,
	&m68000_base_device::m68k_op_bfffo_l_1_ai,
	&m68000_base_device::m68k_op_bfffo_l_1_di,
	&m68000_base_device::m68k_op_bfffo_l_1_ix,
	&m68000_base_device::m68k_op_bfset_l_0,
	&m68000_base_device::m68k_op_bfset_l_1_ai,
	&m68000_base_device::m68k_op_bfset_l_1_di,
	&m68000_base_device::m68k_op_bfset_l_1_ix,
	&m68000_base_device::m68k_op_bfins_l_0,
	&m68000_base_device::m68k_op_bfins_l_1_ai,
	&m68000_base_device::m68k_op_bfins_l_1_di,
	&m68000_base_device::m68k_op_bfins_l_1_ix,
	&m68000_base_device::m68k_op_ftrapcc_l_0,
	&m68000_base_device::m68k_op_pflushan_l_0,
	&m68000_base_device::m68k_op_pflusha_l_0,
	&m68000_base_device::m68k_op_move16_l_0,
	&m68000_base_device::m68k_op_ori_b_1_pi7,
	&m68000_base_device::m68k_op_ori_b_1_pd7,
	&m68000_base_device::m68k_op_ori_b_1_aw,
	&m68000_base_device::m68k_op_ori_b_1_al,
	&m68000_base_device::m68k_op_ori_w_6,
	&m68000_base_device::m68k_op_ori_w_3_aw,
	&m68000_base_device::m68k_op_ori_w_3_al,
	&m68000_base_device::m68k_op_ori_w_7,
	&m68000_base_device::m68k_op_ori_l_5_aw,
	&m68000_base_device::m68k_op_ori_l_5_al,
	&m68000_base_device::m68k_op_chk2cmp2_b_2_aw,
	&m68000_base_device::m68k_op_chk2cmp2_b_2_al,
	&m68000_base_device::m68k_op_chk2cmp2_b_0,
	&m68000_base_device::m68k_op_chk2cmp2_b_1,
	&m68000_base_device::m68k_op_andi_b_1_pi7,
	&m68000_base_device::m68k_op_andi_b_1_pd7,
	&m68000_base_device::m68k_op_andi_b_1_aw,
	&m68000_base_device::m68k_op_andi_b_1_al,
	&m68000_base_device::m68k_op_andi_w_6,
	&m68000_base_device::m68k_op_andi_w_3_aw,
	&m68000_base_device::m68k_op_andi_w_3_al,
	&m68000_base_device::m68k_op_andi_w_7,
	&m68000_base_device::m68k_op_andi_l_5_aw,
	&m68000_base_device::m68k_op_andi_l_5_al,
	&m68000_base_device::m68k_op_chk2cmp2_w_5_aw,
	&m68000_base_device::m68k_op_chk2cmp2_w_5_al,
	&m68000_base_device::m68k_op_chk2cmp2_w_3,
	&m68000_base_device::m68k_op_chk2cmp2_w_4,
	&m68000_base_device::m68k_op_subi_b_1_pi7,
	&m68000_base_device::m68k_op_subi_b_1_pd7,
	&m68000_base_device::m68k_op_subi_b_1_aw,
	&m68000_base_device::m68k_op_subi_b_1_al,
	&m68000_base_device::m68k_op_subi_w_3_aw,
	&m68000_base_device::m68k_op_subi_w_3_al,
	&m68000_base_device::m68k_op_subi_l_5_aw,
	&m68000_base_device::m68k_op_subi_l_5_al,
	&m68000_base_device::m68k_op_chk2cmp2_l_8_aw,
	&m68000_base_device::m68k_op_chk2cmp2_l_8_al,
	&m68000_base_device::m68k_op_chk2cmp2_l_6,
	&m68000_base_device::m68k_op_chk2cmp2_l_7,
	&m68000_base_device::m68k_op_addi_b_1_pi7,
	&m68000_base_device::m68k_op_addi_b_1_pd7,
	&m68000_base_device::m68k_op_addi_b_1_aw,
	&m68000_base_device::m68k_op_addi_b_1_al,
	&m68000_base_device::m68k_op_addi_w_3_aw,
	&m68000_base_device::m68k_op_addi_w_3_al,
	&m68000_base_device::m68k_op_addi_l_5_aw,
	&m68000_base_device::m68k_op_addi_l_5_al,
	&m68000_base_device::m68k_op_callm_l_0_aw,
	&m68000_base_device::m68k_op_callm_l_0_al,
	&m68000_base_device::m68k_op_callm_l_0_pcdi,
	&m68000_base_device::m68k_op_callm_l_0_pcix,
	&m68000_base_device::m68k_op_btst_b_3_pi7,
	&m68000_base_device::m68k_op_btst_b_3_pd7,
	&m68000_base_device::m68k_op_btst_b_3_aw,
	&m68000_base_device::m68k_op_btst_b_3_al,
	&m68000_base_device::m68k_op_btst_b_3_pcdi,
	&m68000_base_device::m68k_op_btst_b_3_pcix,
	&m68000_base_device::m68k_op_bchg_b_3_pi7,
	&m68000_base_device::m68k_op_bchg_b_3_pd7,
	&m68000_base_device::m68k_op_bchg_b_3_aw,
	&m68000_base_device::m68k_op_bchg_b_3_al,
	&m68000_base_device::m68k_op_bclr_b_3_pi7,
	&m68000_base_device::m68k_op_bclr_b_3_pd7,
	&m68000_base_device::m68k_op_bclr_b_3_aw,
	&m68000_base_device::m68k_op_bclr_b_3_al,
	&m68000_base_device::m68k_op_bset_b_3_pi7,
	&m68000_base_device::m68k_op_bset_b_3_pd7,
	&m68000_base_device::m68k_op_bset_b_3_aw,
	&m68000_base_device::m68k_op_bset_b_3_al,
	&m68000_base_device::m68k_op_eori_b_1_pi7,
	&m68000_base_device::m68k_op_eori_b_1_pd7,
	&m68000_base_device::m68k_op_eori_b_1_aw,
	&m68000_base_device::m68k_op_eori_b_1_al,
	&m68000_base_device::m68k_op_eori_w_6,
	&m68000_base_device::m68k_op_eori_w_3_aw,
	&m68000_base_device::m68k_op_eori_w_3_al,
	&m68000_base_device::m68k_op_eori_w_7,
	&m68000_base_device::m68k_op_eori_l_5_aw,
	&m68000_base_device::m68k_op_eori_l_5_al,
	&m68000_base_device::m68k_op_cas_b_0_pi7,
	&m68000_base_device::m68k_op_cas_b_0_pd7,
	&m68000_base_device::m68k_op_cas_b_0_aw,
	&m68000_base_device::m68k_op_cas_b_0_al,
	&m68000_base_device::m68k_op_cmpi_b_1_pi7,
	&m68000_base_device::m68k_op_cmpi_b_1_pd7,
	&m68000_base_device::m68k_op_cmpi_b_1_aw,
	&m68000_base_device::m68k_op_cmpi_b_1_al,
	&m68000_base_device::m68k_op_cmpi_b_2,
	&m68000_base_device::m68k_op_cmpi_b_3,
	&m68000_base_device::m68k_op_cmpi_w_5_aw,
	&m68000_base_device::m68k_op_cmpi_w_5_al,
	&m68000_base_device::m68k_op_cmpi_w_6,
	&m68000_base_device::m68k_op_cmpi_w_7,
	&m68000_base_device::m68k_op_cmpi_l_9_aw,
	&m68000_base_device::m68k_op_cmpi_l_9_al,
	&m68000_base_device::m68k_op_cmpi_l_10,
	&m68000_base_device::m68k_op_cmpi_l_11,
	&m68000_base_device::m68k_op_cas_w_1_aw,
	&m68000_base_device::m68k_op_cas_w_1_al,
	&m68000_base_device::m68k_op_cas2_w_0,
	&m68000_base_device::m68k_op_moves_b_0_pi7,
	&m68000_base_device::m68k_op_moves_b_0_pd7,
	&m68000_base_device::m68k_op_moves_b_0_aw,
	&m68000_base_device::m68k_op_moves_b_0_al,
	&m68000_base_device::m68k_op_moves_w_1_aw,
	&m68000_base_device::m68k_op_moves_w_1_al,
	&m68000_base_device::m68k_op_moves_l_2_aw,
	&m68000_base_device::m68k_op_moves_l_2_al,
	&m68000_base_device::m68k_op_cas_l_2_aw,
	&m68000_base_device::m68k_op_cas_l_2_al,
	&m68000_base_device::m68k_op_cas2_l_1,
	&m68000_base_device::m68k_op_move_b_17_pi7,
	&m68000_base_device::m68k_op_move_b_17_pd7,
	&m68000_base_device::m68k_op_move_b_17_aw,
	&m68000_base_device::m68k_op_move_b_17_al,
	&m68000_base_device::m68k_op_move_b_17_pcdi,
	&m68000_base_device::m68k_op_move_b_17_pcix,
	&m68000_base_device::m68k_op_move_b_17_i,
	&m68000_base_device::m68k_op_move_b_19_pi7,
	&m68000_base_device::m68k_op_move_b_19_pd7,
	&m68000_base_device::m68k_op_move_b_19_aw,
	&m68000_base_device::m68k_op_move_b_19_al,
	&m68000_base_device::m68k_op_move_b_19_pcdi,
	&m68000_base_device::m68k_op_move_b_19_pcix,
	&m68000_base_device::m68k_op_move_b_19_i,
	&m68000_base_device::m68k_op_move_b_6_pi7,
	&m68000_base_device::m68k_op_move_b_6_pd7,
	&m68000_base_device::m68k_op_move_b_6_aw,
	&m68000_base_device::m68k_op_move_b_6_al,
	&m68000_base_device::m68k_op_move_b_6_pcdi,
	&m68000_base_device::m68k_op_move_b_6_pcix,
	&m68000_base_device::m68k_op_move_b_6_i,
	&m68000_base_device::m68k_op_move_b_10_pi7,
	&m68000_base_device::m68k_op_move_b_10_pd7,
	&m68000_base_device::m68k_op_move_b_10_aw,
	&m68000_base_device::m68k_op_move_b_10_al,
	&m68000_base_device::m68k_op_move_b_10_pcdi,
	&m68000_base_device::m68k_op_move_b_10_pcix,
	&m68000_base_device::m68k_op_move_b_10_i,
	&m68000_base_device::m68k_op_move_l_64_aw,
	&m68000_base_device::m68k_op_move_l_64_al,
	&m68000_base_device::m68k_op_move_l_64_pcdi,
	&m68000_base_device::m68k_op_move_l_64_pcix,
	&m68000_base_device::m68k_op_move_l_64_i,
	&m68000_base_device::m68k_op_move_l_67_aw,
	&m68000_base_device::m68k_op_move_l_67_al,
	&m68000_base_device::m68k_op_move_l_67_pcdi,
	&m68000_base_device::m68k_op_move_l_67_pcix,
	&m68000_base_device::m68k_op_move_l_67_i,
	&m68000_base_device::m68k_op_move_w_40_aw,
	&m68000_base_device::m68k_op_move_w_40_al,
	&m68000_base_device::m68k_op_move_w_40_pcdi,
	&m68000_base_device::m68k_op_move_w_40_pcix,
	&m68000_base_device::m68k_op_move_w_40_i,
	&m68000_base_device::m68k_op_move_w_43_aw,
	&m68000_base_device::m68k_op_move_w_43_al,
	&m68000_base_device::m68k_op_move_w_43_pcdi,
	&m68000_base_device::m68k_op_move_w_43_pcix,
	&m68000_base_device::m68k_op_move_w_43_i,
	&m68000_base_device::m68k_op_negx_b_1_pi7,
	&m68000_base_device::m68k_op_negx_b_1_pd7,
	&m68000_base_device::m68k_op_negx_b_1_aw,
	&m68000_base_device::m68k_op_negx_b_1_al,
	&m68000_base_device::m68k_op_negx_w_3_aw,
	&m68000_base_device::m68k_op_negx_w_3_al,
	&m68000_base_device::m68k_op_negx_l_5_aw,
	&m68000_base_device::m68k_op_negx_l_5_al,
	&m68000_base_device::m68k_op_move_w_73_aw,
	&m68000_base_device::m68k_op_move_w_73_al,
	&m68000_base_device::m68k_op_clr_b_1_pi7,
	&m68000_base_device::m68k_op_clr_b_1_pd7,
	&m68000_base_device::m68k_op_clr_b_1_aw,
	&m68000_base_device::m68k_op_clr_b_1_al,
	&m68000_base_device::m68k_op_clr_w_3_aw,
	&m68000_base_device::m68k_op_clr_w_3_al,
	&m68000_base_device::m68k_op_clr_l_5_aw,
	&m68000_base_device::m68k_op_clr_l_5_al,
	&m68000_base_device::m68k_op_move_w_69_aw,
	&m68000_base_device::m68k_op_move_w_69_al,
	&m68000_base_device::m68k_op_neg_b_1_pi7,
	&m68000_base_device::m68k_op_neg_b_1_pd7,
	&m68000_base_device::m68k_op_neg_b_1_aw,
	&m68000_base_device::m68k_op_neg_b_1_al,
	&m68000_base_device::m68k_op_neg_w_3_aw,
	&m68000_base_device::m68k_op_neg_w_3_al,
	&m68000_base_device::m68k_op_neg_l_5_aw,
	&m68000_base_device::m68k_op_neg_l_5_al,
	&m68000_base_device::m68k_op_move_w_71_aw,
	&m68000_base_device::m68k_op_move_w_71_al,
	&m68000_base_device::m68k_op_move_w_71_pcdi,
	&m68000_base_device::m68k_op_move_w_71_pcix,
	&m68000_base_device::m68k_op_move_w_71_i,
	&m68000_base_device::m68k_op_not_b_1_pi7,
	&m68000_base_device::m68k_op_not_b_1_pd7,
	&m68000_base_device::m68k_op_not_b_1_aw,
	&m68000_base_device::m68k_op_not_b_1_al,
	&m68000_base_device::m68k_op_not_w_3_aw,
	&m68000_base_device::m68k_op_not_w_3_al,
	&m68000_base_device::m68k_op_not_l_5_aw,
	&m68000_base_device::m68k_op_not_l_5_al,
	&m68000_base_device::m68k_op_move_w_75_aw,
	&m68000_base_device::m68k_op_move_w_75_al,
	&m68000_base_device::m68k_op_move_w_75_pcdi,
	&m68000_base_device::m68k_op_move_w_75_pcix,
	&m68000_base_device::m68k_op_move_w_75_i,
	&m68000_base_device::m68k_op_link_l_2,
	&m68000_base_device::m68k_op_nbcd_b_1_pi7,
	&m68000_base_device::m68k_op_nbcd_b_1_pd7,
	&m68000_base_device::m68k_op_nbcd_b_1_aw,
	&m68000_base_device::m68k_op_nbcd_b_1_al,
	&m68000_base_device::m68k_op_pea_l_0_aw,
	&m68000_base_device::m68k_op_pea_l_0_al,
	&m68000_base_device::m68k_op_pea_l_0_pcdi,
	&m68000_base_device::m68k_op_pea_l_0_pcix,
	&m68000_base_device::m68k_op_movem_w_1_aw,
	&m68000_base_device::m68k_op_movem_w_1_al,
	&m68000_base_device::m68k_op_movem_l_3_aw,
	&m68000_base_device::m68k_op_movem_l_3_al,
	&m68000_base_device::m68k_op_tst_b_1_pi7,
	&m68000_base_device::m68k_op_tst_b_1_pd7,
	&m68000_base_device::m68k_op_tst_b_1_aw,
	&m68000_base_device::m68k_op_tst_b_1_al,
	&m68000_base_device::m68k_op_tst_b_2,
	&m68000_base_device::m68k_op_tst_b_3,
	&m68000_base_device::m68k_op_tst_b_4,
	&m68000_base_device::m68k_op_tst_w_7_aw,
	&m68000_base_device::m68k_op_tst_w_7_al,
	&m68000_base_device::m68k_op_tst_w_8,
	&m68000_base_device::m68k_op_tst_w_9,
	&m68000_base_device::m68k_op_tst_w_10,
	&m68000_base_device::m68k_op_tst_l_13_aw,
	&m68000_base_device::m68k_op_tst_l_13_al,
	&m68000_base_device::m68k_op_tst_l_14,
	&m68000_base_device::m68k_op_tst_l_15,
	&m68000_base_device::m68k_op_tst_l_16,
	&m68000_base_device::m68k_op_tas_b_1_pi7,
	&m68000_base_device::m68k_op_tas_b_1_pd7,
	&m68000_base_device::m68k_op_tas_b_1_aw,
	&m68000_base_device::m68k_op_tas_b_1_al,
	&m68000_base_device::m68k_op_illegal_0,
	&m68000_base_device::m68k_op_mull_l_1_aw,
	&m68000_base_device::m68k_op_mull_l_1_al,
	&m68000_base_device::m68k_op_mull_l_1_pcdi,
	&m68000_base_device::m68k_op_mull_l_1_pcix,
	&m68000_base_device::m68k_op_mull_l_1_i,
	&m68000_base_device::m68k_op_divl_l_1_aw,
	&m68000_base_device::m68k_op_divl_l_1_al,
	&m68000_base_device::m68k_op_divl_l_1_pcdi,
	&m68000_base_device::m68k_op_divl_l_1_pcix,
	&m68000_base_device::m68k_op_divl_l_1_i,
	&m68000_base_device::m68k_op_movem_w_7_aw,
	&m68000_base_device::m68k_op_movem_w_7_al,
	&m68000_base_device::m68k_op_movem_w_5,
	&m68000_base_device::m68k_op_movem_w_6,
	&m68000_base_device::m68k_op_movem_l_11_aw,
	&m68000_base_device::m68k_op_movem_l_11_al,
	&m68000_base_device::m68k_op_movem_l_9,
	&m68000_base_device::m68k_op_movem_l_10,
	&m68000_base_device::m68k_op_link_w_0,
	&m68000_base_device::m68k_op_unlk_l_0,
	&m68000_base_device::m68k_op_reset_0,
	&m68000_base_device::m68k_op_nop_0,
	&m68000_base_device::m68k_op_stop_0,
	&m68000_base_device::m68k_op_rte_l_0,
	&m68000_base_device::m68k_op_rtd_l_0,
	&m68000_base_device::m68k_op_rts_l_0,
	&m68000_base_device::m68k_op_trapv_0,
	&m68000_base_device::m68k_op_rtr_l_0,
	&m68000_base_device::m68k_op_movec_l_0,
	&m68000_base_device::m68k_op_movec_l_1,
	&m68000_base_device::m68k_op_jsr_l_0_aw,
	&m68000_base_device::m68k_op_jsr_l_0_al,
	&m68000_base_device::m68k_op_jsr_l_0_pcdi,
	&m68000_base_device::m68k_op_jsr_l_0_pcix,
	&m68000_base_device::m68k_op_jmp_l_0_aw,
	&m68000_base_device::m68k_op_jmp_l_0_al,
	&m68000_base_device::m68k_op_jmp_l_0_pcdi,
	&m68000_base_device::m68k_op_jmp_l_0_pcix,
	&m68000_base_device::m68k_op_st_b_1_pi7,
	&m68000_base_device::m68k_op_st_b_1_pd7,
	&m68000_base_device::m68k_op_st_b_1_aw,
	&m68000_base_device::m68k_op_st_b_1_al,
	&m68000_base_device::m68k_op_trapt_w_1,
	&m68000_base_device::m68k_op_trapt_l_2,
	&m68000_base_device::m68k_op_trapt_0,
	&m68000_base_device::m68k_op_sf_b_1_pi7,
	&m68000_base_device::m68k_op_sf_b_1_pd7,
	&m68000_base_device::m68k_op_sf_b_1_aw,
	&m68000_base_device::m68k_op_sf_b_1_al,
	&m68000_base_device::m68k_op_trapf_w_1,
	&m68000_base_device::m68k_op_trapf_l_2,
	&m68000_base_device::m68k_op_trapf_0,
	&m68000_base_device::m68k_op_shi_b_1_pi7,
	&m68000_base_device::m68k_op_shi_b_1_pd7,
	&m68000_base_device::m68k_op_shi_b_1_aw,
	&m68000_base_device::m68k_op_shi_b_1_al,
	&m68000_base_device::m68k_op_traphi_w_1,
	&m68000_base_device::m68k_op_traphi_l_2,
	&m68000_base_device::m68k_op_traphi_0,
	&m68000_base_device::m68k_op_sls_b_1_pi7,
	&m68000_base_device::m68k_op_sls_b_1_pd7,
	&m68000_base_device::m68k_op_sls_b_1_aw,
	&m68000_base_device::m68k_op_sls_b_1_al,
	&m68000_base_device::m68k_op_trapls_w_1,
	&m68000_base_device::m68k_op_trapls_l_2,
	&m68000_base_device::m68k_op_trapls_0,
	&m68000_base_device::m68k_op_scc_b_3_pi7,
	&m68000_base_device::m68k_op_scc_b_3_pd7,
	&m68000_base_device::m68k_op_scc_b_3_aw,
	&m68000_base_device::m68k_op_scc_b_3_al,
	&m68000_base_device::m68k_op_trapcc_w_4,
	&m68000_base_device::m68k_op_trapcc_l_5,
	&m68000_base_device::m68k_op_trapcc_3,
	&m68000_base_device::m68k_op_scs_b_1_pi7,
	&m68000_base_device::m68k_op_scs_b_1_pd7,
	&m68000_base_device::m68k_op_scs_b_1_aw,
	&m68000_base_device::m68k_op_scs_b_1_al,
	&m68000_base_device::m68k_op_trapcs_w_1,
	&m68000_base_device::m68k_op_trapcs_l_2,
	&m68000_base_device::m68k_op_trapcs_0,
	&m68000_base_device::m68k_op_sne_b_1_pi7,
	&m68000_base_device::m68k_op_sne_b_1_pd7,
	&m68000_base_device::m68k_op_sne_b_1_aw,
	&m68000_base_device::m68k_op_sne_b_1_al,
	&m68000_base_device::m68k_op_trapne_w_1,
	&m68000_base_device::m68k_op_trapne_l_2,
	&m68000_base_device::m68k_op_trapne_0,
	&m68000_base_device::m68k_op_seq_b_1_pi7,
	&m68000_base_device::m68k_op_seq_b_1_pd7,
	&m68000_base_device::m68k_op_seq_b_1_aw,
	&m68000_base_device::m68k_op_seq_b_1_al,
	&m68000_base_device::m68k_op_trapeq_w_1,
	&m68000_base_device::m68k_op_trapeq_l_2,
	&m68000_base_device::m68k_op_trapeq_0,
	&m68000_base_device::m68k_op_svc_b_1_pi7,
	&m68000_base_device::m68k_op_svc_b_1_pd7,
	&m68000_base_device::m68k_op_svc_b_1_aw,
	&m68000_base_device::m68k_op_svc_b_1_al,
	&m68000_base_device::m68k_op_trapvc_w_1,
	&m68000_base_device::m68k_op_trapvc_l_2,
	&m68000_base_device::m68k_op_trapvc_0,
	&m68000_base_device::m68k_op_svs_b_1_pi7,
	&m68000_base_device::m68k_op_svs_b_1_pd7,
	&m68000_base_device::m68k_op_svs_b_1_aw,
	&m68000_base_device::m68k_op_svs_b_1_al,
	&m68000_base_device::m68k_op_trapvs_w_1,
	&m68000_base_device::m68k_op_trapvs_l_2,
	&m68000_base_device::m68k_op_trapvs_0,
	&m68000_base_device::m68k_op_spl_b_1_pi7,
	&m68000_base_device::m68k_op_spl_b_1_pd7,
	&m68000_base_device::m68k_op_spl_b_1_aw,
	&m68000_base_device::m68k_op_spl_b_1_al,
	&m68000_base_device::m68k_op_trappl_w_1,
	&m68000_base_device::m68k_op_trappl_l_2,
	&m68000_base_device::m68k_op_trappl_0,
	&m68000_base_device::m68k_op_smi_b_1_pi7,
	&m68000_base_device::m68k_op_smi_b_1_pd7,
	&m68000_base_device::m68k_op_smi_b_1_aw,
	&m68000_base_device::m68k_op_smi_b_1_al,
	&m68000_base_device::m68k_op_trapmi_w_1,
	&m68000_base_device::m68k_op_trapmi_l_2,
	&m68000_base_device::m68k_op_trapmi_0,
	&m68000_base_device::m68k_op_sge_b_1_pi7,
	&m68000_base_device::m68k_op_sge_b_1_pd7,
	&m68000_base_device::m68k_op_sge_b_1_aw,
	&m68000_base_device::m68k_op_sge_b_1_al,
	&m68000_base_device::m68k_op_trapge_w_1,
	&m68000_base_device::m68k_op_trapge_l_2,
	&m68000_base_device::m68k_op_trapge_0,
	&m68000_base_device::m68k_op_slt_b_1_pi7,
	&m68000_base_device::m68k_op_slt_b_1_pd7,
	&m68000_base_device::m68k_op_slt_b_1_aw,
	&m68000_base_device::m68k_op_slt_b_1_al,
	&m68000_base_device::m68k_op_traplt_w_1,
	&m68000_base_device::m68k_op_traplt_l_2,
	&m68000_base_device::m68k_op_traplt_0,
	&m68000_base_device::m68k_op_sgt_b_1_pi7,
	&m68000_base_device::m68k_op_sgt_b_1_pd7,
	&m68000_base_device::m68k_op_sgt_b_1_aw,
	&m68000_base_device::m68k_op_sgt_b_1_al,
	&m68000_base_device::m68k_op_trapgt_w_1,
	&m68000_base_device::m68k_op_trapgt_l_2,
	&m68000_base_device::m68k_op_trapgt_0,
	&m68000_base_device::m68k_op_sle_b_1_pi7,
	&m68000_base_device::m68k_op_sle_b_1_pd7,
	&m68000_base_device::m68k_op_sle_b_1_aw,
	&m68000_base_device::m68k_op_sle_b_1_al,
	&m68000_base_device::m68k_op_traple_w_1,
	&m68000_base_device::m68k_op_traple_l_2,
	&m68000_base_device::m68k_op_traple_0,
	&m68000_base_device::m68k_op_bra_w_1,
	&m68000_base_device::m68k_op_bra_l_2,
	&m68000_base_device::m68k_op_bsr_w_1,
	&m68000_base_device::m68k_op_bsr_l_2,
	&m68000_base_device::m68k_op_bhi_w_1,
	&m68000_base_device::m68k_op_bhi_l_2,
	&m68000_base_device::m68k_op_bls_w_1,
	&m68000_base_device::m68k_op_bls_l_2,
	&m68000_base_device::m68k_op_bcc_w_4,
	&m68000_base_device::m68k_op_bcc_l_5,
	&m68000_base_device::m68k_op_bcs_w_1,
	&m68000_base_device::m68k_op_bcs_l_2,
	&m68000_base_device::m68k_op_bne_w_1,
	&m68000_base_device::m68k_op_bne_l_2,
	&m68000_base_device::m68k_op_beq_w_1,
	&m68000_base_device::m68k_op_beq_l_2,
	&m68000_base_device::m68k_op_bvc_w_1,
	&m68000_base_device::m68k_op_bvc_l_2,
	&m68000_base_device::m68k_op_bvs_w_1,
	&m68000_base_device::m68k_op_bvs_l_2,
	&m68000_base_device::m68k_op_bpl_w_1,
	&m68000_base_device::m68k_op_bpl_l_2,
	&m68000_base_device::m68k_op_bmi_w_1,
	&m68000_base_device::m68k_op_bmi_l_2,
	&m68000_base_device::m68k_op_bge_w_1,
	&m68000_base_device::m68k_op_bge_l_2,
	&m68000_base_device::m68k_op_blt_w_1,
	&m68000_base_device::m68k_op_blt_l_2,
	&m68000_base_device::m68k_op_bgt_w_1,
	&m68000_base_device::m68k_op_bgt_l_2,
	&m68000_base_device::m68k_op_ble_w_1,
	&m68000_base_device::m68k_op_ble_l_2,
	&m68000_base_device::m68k_op_sbcd_b_3,
	&m68000_base_device::m68k_op_pack_w_3,
	&m68000_base_device::m68k_op_unpk_w_3,
	&m68000_base_device::m68k_op_subx_b_5,
	&m68000_base_device::m68k_op_cmpm_b_2,
	&m68000_base_device::m68k_op_abcd_b_3,
	&m68000_base_device::m68k_op_addx_b_5,
	&m68000_base_device::m68k_op_asr_w_6_aw,
	&m68000_base_device::m68k_op_asr_w_6_al,
	&m68000_base_device::m68k_op_asl_w_6_aw,
	&m68000_base_device::m68k_op_asl_w_6_al,
	&m68000_base_device::m68k_op_lsr_w_6_aw,
	&m68000_base_device::m68k_op_lsr_w_6_al,
	&m68000_base_device::m68k_op_lsl_w_6_aw,
	&m68000_base_device::m68k_op_lsl_w_6_al,
	&m68000_base_device::m68k_op_roxr_w_6_aw,
	&m68000_base_device::m68k_op_roxr_w_6_al,
	&m68000_base_device::m68k_op_roxl_w_6_aw,
	&m68000_base_device::m68k_op_roxl_w_6_al,
	&m68000_base_device::m68k_op_ror_w_6_aw,
	&m68000_base_device::m68k_op_ror_w_6_al,
	&m68000_base_device::m68k_op_rol_w_6_aw,
	&m68000_base_device::m68k_op_rol_w_6_al,
	&m68000_base_device::m68k_op_bftst_l_1_aw,
	&m68000_base_device::m68k_op_bftst_l_1_al,
	&m68000_base_device::m68k_op_bftst_l_1_pcdi,
	&m68000_base_device::m68k_op_bftst_l_1_pcix,
	&m68000_base_device::m68k_op_bfextu_l_1_aw,
	&m68000_base_device::m68k_op_bfextu_l_1_al,
	&m68000_base_device::m68k_op_bfextu_l_1_pcdi,
	&m68000_base_device::m68k_op_bfextu_l_1_pcix,
	&m68000_base_device::m68k_op_bfchg_l_1_aw,
	&m68000_base_device::m68k_op_bfchg_l_1_al,
	&m68000_base_device::m68k_op_bfexts_l_1_aw,
	&m68000_base_device::m68k_op_bfexts_l_1_al,
	&m68000_base_device::m68k_op_bfexts_l_1_pcdi,
	&m68000_base_device::m68k_op_bfexts_l_1_pcix,
	&m68000_base_device::m68k_op_bfclr_l_1_aw,
	&m68000_base_device::m68k_op_bfclr_l_1_al,
	&m68000_base_device::m68k_op_bfffo_l_1_aw,
	&m68000_base_device::m68k_op_bfffo_l_1_al,
	&m68000_base_device::m68k_op_bfffo_l_1_pcdi,
	&m68000_base_device::m68k_op_bfffo_l_1_pcix,
	&m68000_base_device::m68k_op_bfset_l_1_aw,
	&m68000_base_device::m68k_op_bfset_l_1_al,
	&m68000_base_device::m68k_op_bfins_l_1_aw,
	&m68000_base_device::m68k_op_bfins_l_1_al,
};

const u16 m68000_base_device::m68k_state_illegal = 1742;

const m68000_base_device::opcode_handler_struct m68000_base_device::m68k_opcode_table[] =
{

	{ 0xa000, 0xf000, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0xf000, 0xf000, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x7000, 0xf100, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xf080, 0xf180, {255, 255,   4,   4, 255, 255, 255}},
	{ 0xf000, 0xf1c0, {255, 255,   4,   4, 255, 255, 255}},
	{ 0xf040, 0xf1c0, {255, 255,   4,   4, 255, 255, 255}},
	{ 0xf000, 0xfe00, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x6000, 0xff00, { 10,  10,  10,  10,  10,  10,  10}},
	{ 0x6100, 0xff00, { 18,  18,   7,   7,   7,   7,   7}},
	{ 0x6200, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6300, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6400, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6500, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6600, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6700, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6800, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6900, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6a00, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6b00, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6c00, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6d00, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6e00, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6f00, 0xff00, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0xf200, 0xff00, {255, 255,   0,   0,   0,   0, 255}},
	{ 0xf300, 0xff00, {255, 255,   0,   0,   0,   0, 255}},
	{ 0xf400, 0xff20, {255, 255, 255, 255,  16, 255, 255}},
	{ 0xf420, 0xff20, {255, 255, 255, 255,  16, 255, 255}},
	{ 0x0100, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0x0108, 0xf1f8, { 16,  16,  12,  12,  12,  12,  12}},
	{ 0x0110, 0xf1f8, {  8,   8,   8,   8,   8,   8,   4}},
	{ 0x0118, 0xf1f8, {  8,   8,   8,   8,   8,   8,   4}},
	{ 0x0120, 0xf1f8, { 10,  10,   9,   9,   9,   9,   4}},
	{ 0x0128, 0xf1f8, { 12,  12,   9,   9,   9,   9,   4}},
	{ 0x0130, 0xf1f8, { 14,  14,  11,  11,  11,  11,   4}},
	{ 0x0140, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x0148, 0xf1f8, { 24,  24,  18,  18,  18,  18,  18}},
	{ 0x0150, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x0158, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x0160, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x0168, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x0170, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x0180, 0xf1f8, { 10,  10,   4,   4,   4,   4,   4}},
	{ 0x0188, 0xf1f8, { 16,  16,  11,  11,  11,  11,  11}},
	{ 0x0190, 0xf1f8, { 12,  14,   8,   8,   8,   8,   4}},
	{ 0x0198, 0xf1f8, { 12,  14,   8,   8,   8,   8,   4}},
	{ 0x01a0, 0xf1f8, { 14,  16,   9,   9,   9,   9,   4}},
	{ 0x01a8, 0xf1f8, { 16,  18,   9,   9,   9,   9,   4}},
	{ 0x01b0, 0xf1f8, { 18,  20,  11,  11,  11,  11,   4}},
	{ 0x01c0, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x01c8, 0xf1f8, { 24,  24,  17,  17,  17,  17,  17}},
	{ 0x01d0, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x01d8, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x01e0, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x01e8, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x01f0, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x1000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x1010, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x1018, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x1020, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x1028, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x1030, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x1080, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x1090, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x1098, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x10a0, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x10a8, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x10b0, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x10c0, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x10d0, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x10d8, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x10e0, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x10e8, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x10f0, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x1100, 0xf1f8, {  8,   8,   5,   5,   5,   5,   5}},
	{ 0x1110, 0xf1f8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x1118, 0xf1f8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x1120, 0xf1f8, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0x1128, 0xf1f8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0x1130, 0xf1f8, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0x1140, 0xf1f8, { 12,  12,   5,   5,   5,   5,   5}},
	{ 0x1150, 0xf1f8, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x1158, 0xf1f8, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x1160, 0xf1f8, { 18,  18,  10,  10,  10,  10,   5}},
	{ 0x1168, 0xf1f8, { 20,  20,  10,  10,  10,  10,   5}},
	{ 0x1170, 0xf1f8, { 22,  22,  12,  12,  12,  12,   5}},
	{ 0x1180, 0xf1f8, { 14,  14,   7,   7,   7,   7,   7}},
	{ 0x1190, 0xf1f8, { 18,  18,  11,  11,  11,  11,   7}},
	{ 0x1198, 0xf1f8, { 18,  18,  11,  11,  11,  11,   7}},
	{ 0x11a0, 0xf1f8, { 20,  20,  12,  12,  12,  12,   7}},
	{ 0x11a8, 0xf1f8, { 22,  22,  12,  12,  12,  12,   7}},
	{ 0x11b0, 0xf1f8, { 24,  24,  14,  14,  14,  14,   7}},
	{ 0x2000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x2008, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x2010, 0xf1f8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x2018, 0xf1f8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x2020, 0xf1f8, { 14,  14,   7,   7,   7,   7,   2}},
	{ 0x2028, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x2030, 0xf1f8, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x2040, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x2048, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x2050, 0xf1f8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x2058, 0xf1f8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x2060, 0xf1f8, { 14,  14,   7,   7,   7,   7,   2}},
	{ 0x2068, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x2070, 0xf1f8, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x2080, 0xf1f8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x2088, 0xf1f8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x2090, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x2098, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x20a0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x20a8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x20b0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x20c0, 0xf1f8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x20c8, 0xf1f8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x20d0, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x20d8, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x20e0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x20e8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x20f0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x2100, 0xf1f8, { 12,  14,   5,   5,   5,   5,   5}},
	{ 0x2108, 0xf1f8, { 12,  14,   5,   5,   5,   5,   5}},
	{ 0x2110, 0xf1f8, { 20,  22,   9,   9,   9,   9,   5}},
	{ 0x2118, 0xf1f8, { 20,  22,   9,   9,   9,   9,   5}},
	{ 0x2120, 0xf1f8, { 22,  24,  10,  10,  10,  10,   5}},
	{ 0x2128, 0xf1f8, { 24,  26,  10,  10,  10,  10,   5}},
	{ 0x2130, 0xf1f8, { 26,  28,  12,  12,  12,  12,   5}},
	{ 0x2140, 0xf1f8, { 16,  16,   5,   5,   5,   5,   5}},
	{ 0x2148, 0xf1f8, { 16,  16,   5,   5,   5,   5,   5}},
	{ 0x2150, 0xf1f8, { 24,  24,   9,   9,   9,   9,   5}},
	{ 0x2158, 0xf1f8, { 24,  24,   9,   9,   9,   9,   5}},
	{ 0x2160, 0xf1f8, { 26,  26,  10,  10,  10,  10,   5}},
	{ 0x2168, 0xf1f8, { 28,  28,  10,  10,  10,  10,   5}},
	{ 0x2170, 0xf1f8, { 30,  30,  12,  12,  12,  12,   5}},
	{ 0x2180, 0xf1f8, { 18,  18,   7,   7,   7,   7,   7}},
	{ 0x2188, 0xf1f8, { 18,  18,   7,   7,   7,   7,   7}},
	{ 0x2190, 0xf1f8, { 26,  26,  11,  11,  11,  11,   7}},
	{ 0x2198, 0xf1f8, { 26,  26,  11,  11,  11,  11,   7}},
	{ 0x21a0, 0xf1f8, { 28,  28,  12,  12,  12,  12,   7}},
	{ 0x21a8, 0xf1f8, { 30,  30,  12,  12,  12,  12,   7}},
	{ 0x21b0, 0xf1f8, { 32,  32,  14,  14,  14,  14,   7}},
	{ 0x3000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x3008, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x3010, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x3018, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x3020, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x3028, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x3030, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x3040, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x3048, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x3050, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x3058, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x3060, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x3068, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x3070, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x3080, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x3088, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x3090, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x3098, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x30a0, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x30a8, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x30b0, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x30c0, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x30c8, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x30d0, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x30d8, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x30e0, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x30e8, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x30f0, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x3100, 0xf1f8, {  8,   8,   5,   5,   5,   5,   5}},
	{ 0x3108, 0xf1f8, {  8,   8,   5,   5,   5,   5,   5}},
	{ 0x3110, 0xf1f8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x3118, 0xf1f8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x3120, 0xf1f8, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0x3128, 0xf1f8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0x3130, 0xf1f8, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0x3140, 0xf1f8, { 12,  12,   5,   5,   5,   5,   5}},
	{ 0x3148, 0xf1f8, { 12,  12,   5,   5,   5,   5,   5}},
	{ 0x3150, 0xf1f8, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x3158, 0xf1f8, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x3160, 0xf1f8, { 18,  18,  10,  10,  10,  10,   5}},
	{ 0x3168, 0xf1f8, { 20,  20,  10,  10,  10,  10,   5}},
	{ 0x3170, 0xf1f8, { 22,  22,  12,  12,  12,  12,   5}},
	{ 0x3180, 0xf1f8, { 14,  14,   7,   7,   7,   7,   7}},
	{ 0x3188, 0xf1f8, { 14,  14,   7,   7,   7,   7,   7}},
	{ 0x3190, 0xf1f8, { 18,  18,  11,  11,  11,  11,   7}},
	{ 0x3198, 0xf1f8, { 18,  18,  11,  11,  11,  11,   7}},
	{ 0x31a0, 0xf1f8, { 20,  20,  12,  12,  12,  12,   7}},
	{ 0x31a8, 0xf1f8, { 22,  22,  12,  12,  12,  12,   7}},
	{ 0x31b0, 0xf1f8, { 24,  24,  14,  14,  14,  14,   7}},
	{ 0x4100, 0xf1f8, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x4110, 0xf1f8, {255, 255,  12,  12,  12,  12,   8}},
	{ 0x4118, 0xf1f8, {255, 255,  12,  12,  12,  12,   8}},
	{ 0x4120, 0xf1f8, {255, 255,  13,  13,  13,  13,   8}},
	{ 0x4128, 0xf1f8, {255, 255,  13,  13,  13,  13,   8}},
	{ 0x4130, 0xf1f8, {255, 255,  15,  15,  15,  15,   8}},
	{ 0x4180, 0xf1f8, { 10,   8,   8,   8,   8,   8,   8}},
	{ 0x4190, 0xf1f8, { 14,  12,  12,  12,  12,  12,   8}},
	{ 0x4198, 0xf1f8, { 14,  12,  12,  12,  12,  12,   8}},
	{ 0x41a0, 0xf1f8, { 16,  14,  13,  13,  13,  13,   8}},
	{ 0x41a8, 0xf1f8, { 18,  16,  13,  13,  13,  13,   8}},
	{ 0x41b0, 0xf1f8, { 20,  18,  15,  15,  15,  15,   8}},
	{ 0x41d0, 0xf1f8, {  4,   4,   6,   6,   6,   6,   2}},
	{ 0x41e8, 0xf1f8, {  8,   8,   7,   7,   7,   7,   2}},
	{ 0x41f0, 0xf1f8, { 12,  12,   9,   9,   9,   9,   2}},
	{ 0x5000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x5010, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5018, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5020, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x5028, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x5030, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x5040, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x5048, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x5050, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5058, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5060, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x5068, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x5070, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x5080, 0xf1f8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x5088, 0xf1f8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x5090, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x5098, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x50a0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x50a8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x50b0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x5100, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x5110, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5118, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5120, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x5128, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x5130, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x5140, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x5148, 0xf1f8, {  8,   4,   2,   2,   2,   2,   2}},
	{ 0x5150, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5158, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5160, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x5168, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x5170, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x5180, 0xf1f8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x5188, 0xf1f8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x5190, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x5198, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x51a0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x51a8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x51b0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x8000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x8010, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x8018, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x8020, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x8028, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x8030, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x8040, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x8050, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x8058, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x8060, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x8068, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x8070, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x8080, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0x8090, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0x8098, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0x80a0, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x80a8, 0xf1f8, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0x80b0, 0xf1f8, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0x80c0, 0xf1f8, {140, 108,  44,  44,  44,  44,  44}},
	{ 0x80d0, 0xf1f8, {144, 112,  48,  48,  48,  48,  44}},
	{ 0x80d8, 0xf1f8, {144, 112,  48,  48,  48,  48,  44}},
	{ 0x80e0, 0xf1f8, {146, 114,  49,  49,  49,  49,  44}},
	{ 0x80e8, 0xf1f8, {148, 116,  49,  49,  49,  49,  44}},
	{ 0x80f0, 0xf1f8, {150, 118,  51,  51,  51,  51,  44}},
	{ 0x8100, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0x8108, 0xf1f8, { 18,  18,  16,  16,  16,  16,  16}},
	{ 0x8110, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x8118, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x8120, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x8128, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x8130, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x8140, 0xf1f8, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x8148, 0xf1f8, {255, 255,  13,  13,  13,  13,  13}},
	{ 0x8150, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x8158, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x8160, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x8168, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x8170, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x8180, 0xf1f8, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x8188, 0xf1f8, {255, 255,  13,  13,  13,  13,  13}},
	{ 0x8190, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x8198, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x81a0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x81a8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x81b0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x81c0, 0xf1f8, {158, 122,  56,  56,  56,  56,  56}},
	{ 0x81d0, 0xf1f8, {162, 126,  60,  60,  60,  60,  56}},
	{ 0x81d8, 0xf1f8, {162, 126,  60,  60,  60,  60,  56}},
	{ 0x81e0, 0xf1f8, {164, 128,  61,  61,  61,  61,  56}},
	{ 0x81e8, 0xf1f8, {166, 130,  61,  61,  61,  61,  56}},
	{ 0x81f0, 0xf1f8, {168, 132,  63,  63,  63,  63,  56}},
	{ 0x9000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x9010, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x9018, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x9020, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x9028, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x9030, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x9040, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x9048, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x9050, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x9058, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x9060, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x9068, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x9070, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x9080, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0x9088, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0x9090, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0x9098, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0x90a0, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x90a8, 0xf1f8, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0x90b0, 0xf1f8, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0x90c0, 0xf1f8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x90c8, 0xf1f8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x90d0, 0xf1f8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x90d8, 0xf1f8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x90e0, 0xf1f8, { 14,  14,   7,   7,   7,   7,   2}},
	{ 0x90e8, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x90f0, 0xf1f8, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x9100, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x9108, 0xf1f8, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0x9110, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x9118, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x9120, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x9128, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x9130, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x9140, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x9148, 0xf1f8, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0x9150, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x9158, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x9160, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x9168, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x9170, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x9180, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0x9188, 0xf1f8, { 30,  30,  12,  12,  12,  12,  12}},
	{ 0x9190, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x9198, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x91a0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x91a8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x91b0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x91c0, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0x91c8, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0x91d0, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0x91d8, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0x91e0, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x91e8, 0xf1f8, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0x91f0, 0xf1f8, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xb000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xb010, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xb018, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xb020, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xb028, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xb030, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xb040, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xb048, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xb050, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xb058, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xb060, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xb068, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xb070, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xb080, 0xf1f8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0xb088, 0xf1f8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0xb090, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xb098, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xb0a0, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0xb0a8, 0xf1f8, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0xb0b0, 0xf1f8, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xb0c0, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xb0c8, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xb0d0, 0xf1f8, { 10,  10,   8,   8,   8,   8,   4}},
	{ 0xb0d8, 0xf1f8, { 10,  10,   8,   8,   8,   8,   4}},
	{ 0xb0e0, 0xf1f8, { 12,  12,   9,   9,   9,   9,   4}},
	{ 0xb0e8, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xb0f0, 0xf1f8, { 16,  16,  11,  11,  11,  11,   4}},
	{ 0xb100, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xb108, 0xf1f8, { 12,  12,   9,   9,   9,   9,   9}},
	{ 0xb110, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xb118, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xb120, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xb128, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0xb130, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0xb140, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xb148, 0xf1f8, { 12,  12,   9,   9,   9,   9,   9}},
	{ 0xb150, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xb158, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xb160, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xb168, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0xb170, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0xb180, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0xb188, 0xf1f8, { 20,  20,   9,   9,   9,   9,   9}},
	{ 0xb190, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xb198, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xb1a0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0xb1a8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0xb1b0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0xb1c0, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xb1c8, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xb1d0, 0xf1f8, { 14,  14,   8,   8,   8,   8,   4}},
	{ 0xb1d8, 0xf1f8, { 14,  14,   8,   8,   8,   8,   4}},
	{ 0xb1e0, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0xb1e8, 0xf1f8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0xb1f0, 0xf1f8, { 20,  20,  11,  11,  11,  11,   4}},
	{ 0xc000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xc010, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xc018, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xc020, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xc028, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xc030, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xc040, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xc050, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xc058, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xc060, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xc068, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xc070, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xc080, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0xc090, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xc098, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xc0a0, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0xc0a8, 0xf1f8, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0xc0b0, 0xf1f8, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xc0c0, 0xf1f8, { 54,  30,  27,  27,  27,  27,  27}},
	{ 0xc0d0, 0xf1f8, { 58,  34,  31,  31,  31,  31,  27}},
	{ 0xc0d8, 0xf1f8, { 58,  34,  31,  31,  31,  31,  27}},
	{ 0xc0e0, 0xf1f8, { 60,  36,  32,  32,  32,  32,  27}},
	{ 0xc0e8, 0xf1f8, { 62,  38,  32,  32,  32,  32,  27}},
	{ 0xc0f0, 0xf1f8, { 64,  40,  34,  34,  34,  34,  27}},
	{ 0xc100, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xc108, 0xf1f8, { 18,  18,  16,  16,  16,  16,  16}},
	{ 0xc110, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xc118, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xc120, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xc128, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0xc130, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0xc140, 0xf1f8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0xc148, 0xf1f8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0xc150, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xc158, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xc160, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xc168, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0xc170, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0xc188, 0xf1f8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0xc190, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xc198, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xc1a0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0xc1a8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0xc1b0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0xc1c0, 0xf1f8, { 54,  32,  27,  27,  27,  27,  27}},
	{ 0xc1d0, 0xf1f8, { 58,  36,  31,  31,  31,  31,  27}},
	{ 0xc1d8, 0xf1f8, { 58,  36,  31,  31,  31,  31,  27}},
	{ 0xc1e0, 0xf1f8, { 60,  38,  32,  32,  32,  32,  27}},
	{ 0xc1e8, 0xf1f8, { 62,  40,  32,  32,  32,  32,  27}},
	{ 0xc1f0, 0xf1f8, { 64,  42,  34,  34,  34,  34,  27}},
	{ 0xd000, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xd010, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xd018, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xd020, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xd028, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xd030, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xd040, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xd048, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xd050, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xd058, 0xf1f8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xd060, 0xf1f8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xd068, 0xf1f8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xd070, 0xf1f8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xd080, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0xd088, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0xd090, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xd098, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xd0a0, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0xd0a8, 0xf1f8, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0xd0b0, 0xf1f8, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xd0c0, 0xf1f8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0xd0c8, 0xf1f8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0xd0d0, 0xf1f8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0xd0d8, 0xf1f8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0xd0e0, 0xf1f8, { 14,  14,   7,   7,   7,   7,   2}},
	{ 0xd0e8, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0xd0f0, 0xf1f8, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0xd100, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xd108, 0xf1f8, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0xd110, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xd118, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xd120, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xd128, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0xd130, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0xd140, 0xf1f8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0xd148, 0xf1f8, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0xd150, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xd158, 0xf1f8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xd160, 0xf1f8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xd168, 0xf1f8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0xd170, 0xf1f8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0xd180, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0xd188, 0xf1f8, { 30,  30,  12,  12,  12,  12,  12}},
	{ 0xd190, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xd198, 0xf1f8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xd1a0, 0xf1f8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0xd1a8, 0xf1f8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0xd1b0, 0xf1f8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0xd1c0, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0xd1c8, 0xf1f8, {  8,   6,   2,   2,   2,   2,   2}},
	{ 0xd1d0, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xd1d8, 0xf1f8, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xd1e0, 0xf1f8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0xd1e8, 0xf1f8, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0xd1f0, 0xf1f8, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xe000, 0xf1f8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0xe008, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xe010, 0xf1f8, {  6,   6,  12,  12,  12,  12,  12}},
	{ 0xe018, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe020, 0xf1f8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0xe028, 0xf1f8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0xe030, 0xf1f8, {  6,   6,  12,  12,  12,  12,  12}},
	{ 0xe038, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe040, 0xf1f8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0xe048, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xe050, 0xf1f8, {  6,   6,  12,  12,  12,  12,  12}},
	{ 0xe058, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe060, 0xf1f8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0xe068, 0xf1f8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0xe070, 0xf1f8, {  6,   6,  12,  12,  12,  12,  12}},
	{ 0xe078, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe080, 0xf1f8, {  8,   8,   6,   6,   6,   6,   6}},
	{ 0xe088, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0xe090, 0xf1f8, {  8,   8,  12,  12,  12,  12,  12}},
	{ 0xe098, 0xf1f8, {  8,   8,   8,   8,   8,   8,   8}},
	{ 0xe0a0, 0xf1f8, {  8,   8,   6,   6,   6,   6,   6}},
	{ 0xe0a8, 0xf1f8, {  8,   8,   6,   6,   6,   6,   6}},
	{ 0xe0b0, 0xf1f8, {  8,   8,  12,  12,  12,  12,  12}},
	{ 0xe0b8, 0xf1f8, {  8,   8,   8,   8,   8,   8,   8}},
	{ 0xe100, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe108, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xe110, 0xf1f8, {  6,   6,  12,  12,  12,  12,  12}},
	{ 0xe118, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe120, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe128, 0xf1f8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0xe130, 0xf1f8, {  6,   6,  12,  12,  12,  12,  12}},
	{ 0xe138, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe140, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe148, 0xf1f8, {  6,   6,   4,   4,   4,   4,   4}},
	{ 0xe150, 0xf1f8, {  6,   6,  12,  12,  12,  12,  12}},
	{ 0xe158, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe160, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe168, 0xf1f8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0xe170, 0xf1f8, {  6,   6,  12,  12,  12,  12,  12}},
	{ 0xe178, 0xf1f8, {  6,   6,   8,   8,   8,   8,   8}},
	{ 0xe180, 0xf1f8, {  8,   8,   8,   8,   8,   8,   8}},
	{ 0xe188, 0xf1f8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0xe190, 0xf1f8, {  8,   8,  12,  12,  12,  12,  12}},
	{ 0xe198, 0xf1f8, {  8,   8,   8,   8,   8,   8,   8}},
	{ 0xe1a0, 0xf1f8, {  8,   8,   8,   8,   8,   8,   8}},
	{ 0xe1a8, 0xf1f8, {  8,   8,   6,   6,   6,   6,   6}},
	{ 0xe1b0, 0xf1f8, {  8,   8,  12,  12,  12,  12,  12}},
	{ 0xe1b8, 0xf1f8, {  8,   8,   8,   8,   8,   8,   8}},
	{ 0xf048, 0xf1f8, {255, 255,   4,   4, 255, 255, 255}},
	{ 0xf078, 0xf1f8, {255, 255,   4,   4, 255, 255, 255}},
	{ 0xf548, 0xffd8, {255, 255, 255, 255,   8, 255, 255}},
	{ 0x06c0, 0xfff0, {255, 255,  19,  19,  19,  19,  19}},
	{ 0x4e40, 0xfff0, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x011f, 0xf1ff, {  8,   8,   8,   8,   8,   8,   4}},
	{ 0x0127, 0xf1ff, { 10,  10,   9,   9,   9,   9,   4}},
	{ 0x0138, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x0139, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x013a, 0xf1ff, { 12,  12,   9,   9,   9,   9,   4}},
	{ 0x013b, 0xf1ff, { 14,  14,  11,  11,  11,  11,   4}},
	{ 0x013c, 0xf1ff, {  8,   8,   6,   6,   6,   6,   4}},
	{ 0x015f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x0167, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x0178, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0179, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x019f, 0xf1ff, { 12,  14,   8,   8,   8,   8,   4}},
	{ 0x01a7, 0xf1ff, { 14,  16,   9,   9,   9,   9,   4}},
	{ 0x01b8, 0xf1ff, { 16,  18,   8,   8,   8,   8,   4}},
	{ 0x01b9, 0xf1ff, { 20,  22,   8,   8,   8,   8,   4}},
	{ 0x01df, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x01e7, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x01f8, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x01f9, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x101f, 0xf1ff, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x1027, 0xf1ff, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x1038, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x1039, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x103a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x103b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x103c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0x109f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x10a7, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x10b8, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x10b9, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x10ba, 0xf1ff, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x10bb, 0xf1ff, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x10bc, 0xf1ff, { 12,  12,   6,   6,   6,   6,   4}},
	{ 0x10df, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x10e7, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x10f8, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x10f9, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x10fa, 0xf1ff, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x10fb, 0xf1ff, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x10fc, 0xf1ff, { 12,  12,   6,   6,   6,   6,   4}},
	{ 0x111f, 0xf1ff, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x1127, 0xf1ff, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0x1138, 0xf1ff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x1139, 0xf1ff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0x113a, 0xf1ff, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0x113b, 0xf1ff, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0x113c, 0xf1ff, { 12,  12,   7,   7,   7,   7,   5}},
	{ 0x115f, 0xf1ff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x1167, 0xf1ff, { 18,  18,  10,  10,  10,  10,   5}},
	{ 0x1178, 0xf1ff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0x1179, 0xf1ff, { 24,  24,   9,   9,   9,   9,   5}},
	{ 0x117a, 0xf1ff, { 20,  20,  10,  10,  10,  10,   5}},
	{ 0x117b, 0xf1ff, { 22,  22,  12,  12,  12,  12,   5}},
	{ 0x117c, 0xf1ff, { 16,  16,   7,   7,   7,   7,   5}},
	{ 0x119f, 0xf1ff, { 18,  18,  11,  11,  11,  11,   7}},
	{ 0x11a7, 0xf1ff, { 20,  20,  12,  12,  12,  12,   7}},
	{ 0x11b8, 0xf1ff, { 22,  22,  11,  11,  11,  11,   7}},
	{ 0x11b9, 0xf1ff, { 26,  26,  11,  11,  11,  11,   7}},
	{ 0x11ba, 0xf1ff, { 22,  22,  12,  12,  12,  12,   7}},
	{ 0x11bb, 0xf1ff, { 24,  24,  14,  14,  14,  14,   7}},
	{ 0x11bc, 0xf1ff, { 18,  18,   9,   9,   9,   9,   7}},
	{ 0x2038, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x2039, 0xf1ff, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0x203a, 0xf1ff, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x203b, 0xf1ff, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x203c, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x2078, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x2079, 0xf1ff, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0x207a, 0xf1ff, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x207b, 0xf1ff, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x207c, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x20b8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x20b9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x20ba, 0xf1ff, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x20bb, 0xf1ff, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x20bc, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x20f8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x20f9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x20fa, 0xf1ff, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x20fb, 0xf1ff, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x20fc, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x2138, 0xf1ff, { 24,  26,   9,   9,   9,   9,   5}},
	{ 0x2139, 0xf1ff, { 28,  30,   9,   9,   9,   9,   5}},
	{ 0x213a, 0xf1ff, { 24,  26,  10,  10,  10,  10,   5}},
	{ 0x213b, 0xf1ff, { 26,  28,  12,  12,  12,  12,   5}},
	{ 0x213c, 0xf1ff, { 20,  22,   9,   9,   9,   9,   5}},
	{ 0x2178, 0xf1ff, { 28,  28,   9,   9,   9,   9,   5}},
	{ 0x2179, 0xf1ff, { 32,  32,   9,   9,   9,   9,   5}},
	{ 0x217a, 0xf1ff, { 28,  28,  10,  10,  10,  10,   5}},
	{ 0x217b, 0xf1ff, { 30,  30,  12,  12,  12,  12,   5}},
	{ 0x217c, 0xf1ff, { 24,  24,   9,   9,   9,   9,   5}},
	{ 0x21b8, 0xf1ff, { 30,  30,  11,  11,  11,  11,   7}},
	{ 0x21b9, 0xf1ff, { 34,  34,  11,  11,  11,  11,   7}},
	{ 0x21ba, 0xf1ff, { 30,  30,  12,  12,  12,  12,   7}},
	{ 0x21bb, 0xf1ff, { 32,  32,  14,  14,  14,  14,   7}},
	{ 0x21bc, 0xf1ff, { 26,  26,  11,  11,  11,  11,   7}},
	{ 0x3038, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x3039, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x303a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x303b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x303c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0x3078, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x3079, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x307a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x307b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x307c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0x30b8, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x30b9, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x30ba, 0xf1ff, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x30bb, 0xf1ff, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x30bc, 0xf1ff, { 12,  12,   6,   6,   6,   6,   4}},
	{ 0x30f8, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x30f9, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x30fa, 0xf1ff, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x30fb, 0xf1ff, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x30fc, 0xf1ff, { 12,  12,   6,   6,   6,   6,   4}},
	{ 0x3138, 0xf1ff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x3139, 0xf1ff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0x313a, 0xf1ff, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0x313b, 0xf1ff, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0x313c, 0xf1ff, { 12,  12,   7,   7,   7,   7,   5}},
	{ 0x3178, 0xf1ff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0x3179, 0xf1ff, { 24,  24,   9,   9,   9,   9,   5}},
	{ 0x317a, 0xf1ff, { 20,  20,  10,  10,  10,  10,   5}},
	{ 0x317b, 0xf1ff, { 22,  22,  12,  12,  12,  12,   5}},
	{ 0x317c, 0xf1ff, { 16,  16,   7,   7,   7,   7,   5}},
	{ 0x31b8, 0xf1ff, { 22,  22,  11,  11,  11,  11,   7}},
	{ 0x31b9, 0xf1ff, { 26,  26,  11,  11,  11,  11,   7}},
	{ 0x31ba, 0xf1ff, { 22,  22,  12,  12,  12,  12,   7}},
	{ 0x31bb, 0xf1ff, { 24,  24,  14,  14,  14,  14,   7}},
	{ 0x31bc, 0xf1ff, { 18,  18,   9,   9,   9,   9,   7}},
	{ 0x4138, 0xf1ff, {255, 255,  12,  12,  12,  12,   8}},
	{ 0x4139, 0xf1ff, {255, 255,  12,  12,  12,  12,   8}},
	{ 0x413a, 0xf1ff, {255, 255,  13,  13,  13,  13,   8}},
	{ 0x413b, 0xf1ff, {255, 255,  15,  15,  15,  15,   8}},
	{ 0x413c, 0xf1ff, {255, 255,  12,  12,  12,  12,   8}},
	{ 0x41b8, 0xf1ff, { 18,  16,  12,  12,  12,  12,   8}},
	{ 0x41b9, 0xf1ff, { 22,  20,  12,  12,  12,  12,   8}},
	{ 0x41ba, 0xf1ff, { 18,  16,  13,  13,  13,  13,   8}},
	{ 0x41bb, 0xf1ff, { 20,  18,  15,  15,  15,  15,   8}},
	{ 0x41bc, 0xf1ff, { 14,  12,  10,  10,  10,  10,   8}},
	{ 0x41f8, 0xf1ff, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x41f9, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x41fa, 0xf1ff, {  8,   8,   7,   7,   7,   7,   2}},
	{ 0x41fb, 0xf1ff, { 12,  12,   9,   9,   9,   9,   2}},
	{ 0x501f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5027, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x5038, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x5039, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x5078, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x5079, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x50b8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x50b9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x511f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x5127, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x5138, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x5139, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x5178, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x5179, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x51b8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x51b9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x801f, 0xf1ff, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x8027, 0xf1ff, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x8038, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x8039, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x803a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x803b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x803c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0x8078, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x8079, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x807a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x807b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x807c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0x80b8, 0xf1ff, { 18,  18,   6,   6,   6,   6,   2}},
	{ 0x80b9, 0xf1ff, { 22,  22,   6,   6,   6,   6,   2}},
	{ 0x80ba, 0xf1ff, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0x80bb, 0xf1ff, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0x80bc, 0xf1ff, { 16,  14,   6,   6,   6,   6,   2}},
	{ 0x80f8, 0xf1ff, {148, 116,  48,  48,  48,  48,  44}},
	{ 0x80f9, 0xf1ff, {152, 120,  48,  48,  48,  48,  44}},
	{ 0x80fa, 0xf1ff, {148, 116,  49,  49,  49,  49,  44}},
	{ 0x80fb, 0xf1ff, {150, 118,  51,  51,  51,  51,  44}},
	{ 0x80fc, 0xf1ff, {144, 112,  46,  46,  46,  46,  44}},
	{ 0x810f, 0xf1ff, { 18,  18,  16,  16,  16,  16,  16}},
	{ 0x811f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x8127, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x8138, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x8139, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x814f, 0xf1ff, {255, 255,  13,  13,  13,  13,  13}},
	{ 0x8178, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x8179, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x818f, 0xf1ff, {255, 255,  13,  13,  13,  13,  13}},
	{ 0x81b8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x81b9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x81f8, 0xf1ff, {166, 130,  60,  60,  60,  60,  56}},
	{ 0x81f9, 0xf1ff, {170, 134,  60,  60,  60,  60,  56}},
	{ 0x81fa, 0xf1ff, {166, 130,  61,  61,  61,  61,  56}},
	{ 0x81fb, 0xf1ff, {168, 132,  63,  63,  63,  63,  56}},
	{ 0x81fc, 0xf1ff, {162, 126,  58,  58,  58,  58,  56}},
	{ 0x901f, 0xf1ff, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x9027, 0xf1ff, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x9038, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x9039, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x903a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x903b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x903c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0x9078, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x9079, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x907a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x907b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x907c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0x90b8, 0xf1ff, { 18,  18,   6,   6,   6,   6,   2}},
	{ 0x90b9, 0xf1ff, { 22,  22,   6,   6,   6,   6,   2}},
	{ 0x90ba, 0xf1ff, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0x90bb, 0xf1ff, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0x90bc, 0xf1ff, { 16,  14,   6,   6,   6,   6,   2}},
	{ 0x90f8, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x90f9, 0xf1ff, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0x90fa, 0xf1ff, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x90fb, 0xf1ff, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x90fc, 0xf1ff, { 12,  12,   4,   4,   4,   4,   2}},
	{ 0x910f, 0xf1ff, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0x911f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x9127, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x9138, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x9139, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x9178, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x9179, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x91b8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x91b9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x91f8, 0xf1ff, { 18,  18,   6,   6,   6,   6,   2}},
	{ 0x91f9, 0xf1ff, { 22,  22,   6,   6,   6,   6,   2}},
	{ 0x91fa, 0xf1ff, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0x91fb, 0xf1ff, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0x91fc, 0xf1ff, { 16,  14,   6,   6,   6,   6,   2}},
	{ 0xb01f, 0xf1ff, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xb027, 0xf1ff, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xb038, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0xb039, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0xb03a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xb03b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xb03c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0xb078, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0xb079, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0xb07a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xb07b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xb07c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0xb0b8, 0xf1ff, { 18,  18,   6,   6,   6,   6,   2}},
	{ 0xb0b9, 0xf1ff, { 22,  22,   6,   6,   6,   6,   2}},
	{ 0xb0ba, 0xf1ff, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0xb0bb, 0xf1ff, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xb0bc, 0xf1ff, { 14,  14,   6,   6,   6,   6,   2}},
	{ 0xb0f8, 0xf1ff, { 14,  14,   8,   8,   8,   8,   4}},
	{ 0xb0f9, 0xf1ff, { 18,  18,   8,   8,   8,   8,   4}},
	{ 0xb0fa, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xb0fb, 0xf1ff, { 16,  16,  11,  11,  11,  11,   4}},
	{ 0xb0fc, 0xf1ff, { 10,  10,   6,   6,   6,   6,   4}},
	{ 0xb10f, 0xf1ff, { 12,  12,   9,   9,   9,   9,   9}},
	{ 0xb11f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xb127, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xb138, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0xb139, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xb178, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0xb179, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xb1b8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0xb1b9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0xb1f8, 0xf1ff, { 18,  18,   8,   8,   8,   8,   4}},
	{ 0xb1f9, 0xf1ff, { 22,  22,   8,   8,   8,   8,   4}},
	{ 0xb1fa, 0xf1ff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0xb1fb, 0xf1ff, { 20,  20,  11,  11,  11,  11,   4}},
	{ 0xb1fc, 0xf1ff, { 14,  14,   8,   8,   8,   8,   4}},
	{ 0xc01f, 0xf1ff, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xc027, 0xf1ff, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xc038, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0xc039, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0xc03a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xc03b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xc03c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0xc078, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0xc079, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0xc07a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xc07b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xc07c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0xc0b8, 0xf1ff, { 18,  18,   6,   6,   6,   6,   2}},
	{ 0xc0b9, 0xf1ff, { 22,  22,   6,   6,   6,   6,   2}},
	{ 0xc0ba, 0xf1ff, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0xc0bb, 0xf1ff, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xc0bc, 0xf1ff, { 16,  14,   6,   6,   6,   6,   2}},
	{ 0xc0f8, 0xf1ff, { 62,  38,  31,  31,  31,  31,  27}},
	{ 0xc0f9, 0xf1ff, { 66,  42,  31,  31,  31,  31,  27}},
	{ 0xc0fa, 0xf1ff, { 62,  38,  32,  32,  32,  32,  27}},
	{ 0xc0fb, 0xf1ff, { 64,  40,  34,  34,  34,  34,  27}},
	{ 0xc0fc, 0xf1ff, { 58,  34,  29,  29,  29,  29,  27}},
	{ 0xc10f, 0xf1ff, { 18,  18,  16,  16,  16,  16,  16}},
	{ 0xc11f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xc127, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xc138, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0xc139, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xc178, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0xc179, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xc1b8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0xc1b9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0xc1f8, 0xf1ff, { 62,  40,  31,  31,  31,  31,  27}},
	{ 0xc1f9, 0xf1ff, { 66,  44,  31,  31,  31,  31,  27}},
	{ 0xc1fa, 0xf1ff, { 62,  40,  32,  32,  32,  32,  27}},
	{ 0xc1fb, 0xf1ff, { 64,  42,  34,  34,  34,  34,  27}},
	{ 0xc1fc, 0xf1ff, { 58,  36,  29,  29,  29,  29,  27}},
	{ 0xd01f, 0xf1ff, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0xd027, 0xf1ff, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0xd038, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0xd039, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0xd03a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xd03b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xd03c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0xd078, 0xf1ff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0xd079, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0xd07a, 0xf1ff, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0xd07b, 0xf1ff, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0xd07c, 0xf1ff, {  8,   8,   4,   4,   4,   4,   2}},
	{ 0xd0b8, 0xf1ff, { 18,  18,   6,   6,   6,   6,   2}},
	{ 0xd0b9, 0xf1ff, { 22,  22,   6,   6,   6,   6,   2}},
	{ 0xd0ba, 0xf1ff, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0xd0bb, 0xf1ff, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xd0bc, 0xf1ff, { 16,  14,   6,   6,   6,   6,   2}},
	{ 0xd0f8, 0xf1ff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0xd0f9, 0xf1ff, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0xd0fa, 0xf1ff, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0xd0fb, 0xf1ff, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0xd0fc, 0xf1ff, { 12,  12,   4,   4,   4,   4,   2}},
	{ 0xd10f, 0xf1ff, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0xd11f, 0xf1ff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0xd127, 0xf1ff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0xd138, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0xd139, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xd178, 0xf1ff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0xd179, 0xf1ff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0xd1b8, 0xf1ff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0xd1b9, 0xf1ff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0xd1f8, 0xf1ff, { 18,  18,   6,   6,   6,   6,   2}},
	{ 0xd1f9, 0xf1ff, { 22,  22,   6,   6,   6,   6,   2}},
	{ 0xd1fa, 0xf1ff, { 18,  18,   7,   7,   7,   7,   2}},
	{ 0xd1fb, 0xf1ff, { 20,  20,   9,   9,   9,   9,   2}},
	{ 0xd1fc, 0xf1ff, { 16,  14,   6,   6,   6,   6,   2}},
	{ 0x0000, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0010, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0018, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0020, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0028, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0030, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0040, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0050, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0058, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0060, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0068, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0070, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0080, 0xfff8, { 16,  14,   2,   2,   2,   2,   2}},
	{ 0x0090, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x0098, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x00a0, 0xfff8, { 30,  30,   9,   9,   9,   9,   4}},
	{ 0x00a8, 0xfff8, { 32,  32,   9,   9,   9,   9,   4}},
	{ 0x00b0, 0xfff8, { 34,  34,  11,  11,  11,  11,   4}},
	{ 0x00d0, 0xfff8, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x00e8, 0xfff8, {255, 255,  23,  23,  23,  23,  18}},
	{ 0x00f0, 0xfff8, {255, 255,  25,  25,  25,  25,  18}},
	{ 0x0200, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0210, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0218, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0220, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0228, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0230, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0240, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0250, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0258, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0260, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0268, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0270, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0280, 0xfff8, { 14,  14,   2,   2,   2,   2,   2}},
	{ 0x0290, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x0298, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x02a0, 0xfff8, { 30,  30,   9,   9,   9,   9,   4}},
	{ 0x02a8, 0xfff8, { 32,  32,   9,   9,   9,   9,   4}},
	{ 0x02b0, 0xfff8, { 34,  34,  11,  11,  11,  11,   4}},
	{ 0x02d0, 0xfff8, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x02e8, 0xfff8, {255, 255,  23,  23,  23,  23,  18}},
	{ 0x02f0, 0xfff8, {255, 255,  25,  25,  25,  25,  18}},
	{ 0x0400, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0410, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0418, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0420, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0428, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0430, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0440, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0450, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0458, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0460, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0468, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0470, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0480, 0xfff8, { 16,  14,   2,   2,   2,   2,   2}},
	{ 0x0490, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x0498, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x04a0, 0xfff8, { 30,  30,   9,   9,   9,   9,   4}},
	{ 0x04a8, 0xfff8, { 32,  32,   9,   9,   9,   9,   4}},
	{ 0x04b0, 0xfff8, { 34,  34,  11,  11,  11,  11,   4}},
	{ 0x04d0, 0xfff8, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x04e8, 0xfff8, {255, 255,  23,  23,  23,  23,  18}},
	{ 0x04f0, 0xfff8, {255, 255,  25,  25,  25,  25,  18}},
	{ 0x0600, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0610, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0618, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0620, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0628, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0630, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0640, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0650, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0658, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0660, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0668, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0670, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0680, 0xfff8, { 16,  14,   2,   2,   2,   2,   2}},
	{ 0x0690, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x0698, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x06a0, 0xfff8, { 30,  30,   9,   9,   9,   9,   4}},
	{ 0x06a8, 0xfff8, { 32,  32,   9,   9,   9,   9,   4}},
	{ 0x06b0, 0xfff8, { 34,  34,  11,  11,  11,  11,   4}},
	{ 0x06d0, 0xfff8, {255, 255,  64,  64,  64,  64,  60}},
	{ 0x06e8, 0xfff8, {255, 255,  65,  65,  65,  65,  60}},
	{ 0x06f0, 0xfff8, {255, 255,  67,  67,  67,  67,  60}},
	{ 0x0800, 0xfff8, { 10,  10,   4,   4,   4,   4,   4}},
	{ 0x0810, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x0818, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x0820, 0xfff8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x0828, 0xfff8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x0830, 0xfff8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x0840, 0xfff8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x0850, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0858, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0860, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0868, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0870, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0880, 0xfff8, { 14,  14,   4,   4,   4,   4,   4}},
	{ 0x0890, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0898, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x08a0, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x08a8, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x08b0, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x08c0, 0xfff8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x08d0, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x08d8, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x08e0, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x08e8, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x08f0, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0a00, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0a10, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0a18, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0a20, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0a28, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0a30, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0a40, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0a50, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0a58, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0a60, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0a68, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x0a70, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x0a80, 0xfff8, { 16,  14,   2,   2,   2,   2,   2}},
	{ 0x0a90, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x0a98, 0xfff8, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x0aa0, 0xfff8, { 30,  30,   9,   9,   9,   9,   4}},
	{ 0x0aa8, 0xfff8, { 32,  32,   9,   9,   9,   9,   4}},
	{ 0x0ab0, 0xfff8, { 34,  34,  11,  11,  11,  11,   4}},
	{ 0x0ad0, 0xfff8, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0ad8, 0xfff8, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0ae0, 0xfff8, {255, 255,  17,  17,  17,  17,  12}},
	{ 0x0ae8, 0xfff8, {255, 255,  17,  17,  17,  17,  12}},
	{ 0x0af0, 0xfff8, {255, 255,  19,  19,  19,  19,  12}},
	{ 0x0c00, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0c10, 0xfff8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x0c18, 0xfff8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x0c20, 0xfff8, { 14,  14,   7,   7,   7,   7,   2}},
	{ 0x0c28, 0xfff8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x0c30, 0xfff8, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x0c40, 0xfff8, {  8,   8,   2,   2,   2,   2,   2}},
	{ 0x0c50, 0xfff8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x0c58, 0xfff8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x0c60, 0xfff8, { 14,  14,   7,   7,   7,   7,   2}},
	{ 0x0c68, 0xfff8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x0c70, 0xfff8, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x0c80, 0xfff8, { 14,  12,   2,   2,   2,   2,   2}},
	{ 0x0c90, 0xfff8, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0x0c98, 0xfff8, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0x0ca0, 0xfff8, { 22,  22,   7,   7,   7,   7,   2}},
	{ 0x0ca8, 0xfff8, { 24,  24,   7,   7,   7,   7,   2}},
	{ 0x0cb0, 0xfff8, { 26,  26,   9,   9,   9,   9,   2}},
	{ 0x0cd0, 0xfff8, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0cd8, 0xfff8, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0ce0, 0xfff8, {255, 255,  17,  17,  17,  17,  12}},
	{ 0x0ce8, 0xfff8, {255, 255,  17,  17,  17,  17,  12}},
	{ 0x0cf0, 0xfff8, {255, 255,  19,  19,  19,  19,  12}},
	{ 0x0e10, 0xfff8, {255,  18,   9,   9,   9,   9,   5}},
	{ 0x0e18, 0xfff8, {255,  18,   9,   9,   9,   9,   5}},
	{ 0x0e20, 0xfff8, {255,  20,  10,  10,  10,  10,   5}},
	{ 0x0e28, 0xfff8, {255,  26,  10,  10,  10,  10,   5}},
	{ 0x0e30, 0xfff8, {255,  30,  12,  12,  12,  12,   5}},
	{ 0x0e50, 0xfff8, {255,  18,   9,   9,   9,   9,   5}},
	{ 0x0e58, 0xfff8, {255,  18,   9,   9,   9,   9,   5}},
	{ 0x0e60, 0xfff8, {255,  20,  10,  10,  10,  10,   5}},
	{ 0x0e68, 0xfff8, {255,  26,  10,  10,  10,  10,   5}},
	{ 0x0e70, 0xfff8, {255,  30,  12,  12,  12,  12,   5}},
	{ 0x0e90, 0xfff8, {255,  22,   9,   9,   9,   9,   5}},
	{ 0x0e98, 0xfff8, {255,  22,   9,   9,   9,   9,   5}},
	{ 0x0ea0, 0xfff8, {255,  28,  10,  10,  10,  10,   5}},
	{ 0x0ea8, 0xfff8, {255,  32,  10,  10,  10,  10,   5}},
	{ 0x0eb0, 0xfff8, {255,  36,  12,  12,  12,  12,   5}},
	{ 0x0ed0, 0xfff8, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0ed8, 0xfff8, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0ee0, 0xfff8, {255, 255,  17,  17,  17,  17,  12}},
	{ 0x0ee8, 0xfff8, {255, 255,  17,  17,  17,  17,  12}},
	{ 0x0ef0, 0xfff8, {255, 255,  19,  19,  19,  19,  12}},
	{ 0x11c0, 0xfff8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x11d0, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x11d8, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x11e0, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x11e8, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x11f0, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x13c0, 0xfff8, { 16,  16,   6,   6,   6,   6,   6}},
	{ 0x13d0, 0xfff8, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x13d8, 0xfff8, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x13e0, 0xfff8, { 22,  22,  11,  11,  11,  11,   6}},
	{ 0x13e8, 0xfff8, { 24,  24,  11,  11,  11,  11,   6}},
	{ 0x13f0, 0xfff8, { 26,  26,  13,  13,  13,  13,   6}},
	{ 0x1ec0, 0xfff8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x1ed0, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x1ed8, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x1ee0, 0xfff8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x1ee8, 0xfff8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x1ef0, 0xfff8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x1f00, 0xfff8, {  8,   8,   5,   5,   5,   5,   5}},
	{ 0x1f10, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x1f18, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x1f20, 0xfff8, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0x1f28, 0xfff8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0x1f30, 0xfff8, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0x21c0, 0xfff8, { 16,  16,   4,   4,   4,   4,   4}},
	{ 0x21c8, 0xfff8, { 16,  16,   4,   4,   4,   4,   4}},
	{ 0x21d0, 0xfff8, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x21d8, 0xfff8, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x21e0, 0xfff8, { 26,  26,   9,   9,   9,   9,   4}},
	{ 0x21e8, 0xfff8, { 28,  28,   9,   9,   9,   9,   4}},
	{ 0x21f0, 0xfff8, { 30,  30,  11,  11,  11,  11,   4}},
	{ 0x23c0, 0xfff8, { 20,  20,   6,   6,   6,   6,   6}},
	{ 0x23c8, 0xfff8, { 20,  20,   6,   6,   6,   6,   6}},
	{ 0x23d0, 0xfff8, { 28,  28,  10,  10,  10,  10,   6}},
	{ 0x23d8, 0xfff8, { 28,  28,  10,  10,  10,  10,   6}},
	{ 0x23e0, 0xfff8, { 30,  30,  11,  11,  11,  11,   6}},
	{ 0x23e8, 0xfff8, { 32,  32,  11,  11,  11,  11,   6}},
	{ 0x23f0, 0xfff8, { 34,  34,  13,  13,  13,  13,   6}},
	{ 0x31c0, 0xfff8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x31c8, 0xfff8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x31d0, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x31d8, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x31e0, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x31e8, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x31f0, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x33c0, 0xfff8, { 16,  16,   6,   6,   6,   6,   6}},
	{ 0x33c8, 0xfff8, { 16,  16,   6,   6,   6,   6,   6}},
	{ 0x33d0, 0xfff8, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x33d8, 0xfff8, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x33e0, 0xfff8, { 22,  22,  11,  11,  11,  11,   6}},
	{ 0x33e8, 0xfff8, { 24,  24,  11,  11,  11,  11,   6}},
	{ 0x33f0, 0xfff8, { 26,  26,  13,  13,  13,  13,   6}},
	{ 0x4000, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4010, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4018, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4020, 0xfff8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4028, 0xfff8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x4030, 0xfff8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x4040, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4050, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4058, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4060, 0xfff8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4068, 0xfff8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x4070, 0xfff8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x4080, 0xfff8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0x4090, 0xfff8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x4098, 0xfff8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x40a0, 0xfff8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x40a8, 0xfff8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x40b0, 0xfff8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x40c0, 0xfff8, {  6,   4,   8,   8,   8,   8,   8}},
	{ 0x40d0, 0xfff8, { 12,  12,  12,  12,  12,  12,   8}},
	{ 0x40d8, 0xfff8, { 12,  12,  12,  12,  12,  12,   8}},
	{ 0x40e0, 0xfff8, { 14,  14,  13,  13,  13,  13,   8}},
	{ 0x40e8, 0xfff8, { 16,  16,  13,  13,  13,  13,   8}},
	{ 0x40f0, 0xfff8, { 18,  18,  15,  15,  15,  15,   8}},
	{ 0x4200, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4210, 0xfff8, { 12,   8,   8,   8,   8,   8,   4}},
	{ 0x4218, 0xfff8, { 12,   8,   8,   8,   8,   8,   4}},
	{ 0x4220, 0xfff8, { 14,  10,   9,   9,   9,   9,   4}},
	{ 0x4228, 0xfff8, { 16,  12,   9,   9,   9,   9,   4}},
	{ 0x4230, 0xfff8, { 18,  14,  11,  11,  11,  11,   4}},
	{ 0x4240, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4250, 0xfff8, { 12,   8,   8,   8,   8,   8,   4}},
	{ 0x4258, 0xfff8, { 12,   8,   8,   8,   8,   8,   4}},
	{ 0x4260, 0xfff8, { 14,  10,   9,   9,   9,   9,   4}},
	{ 0x4268, 0xfff8, { 16,  12,   9,   9,   9,   9,   4}},
	{ 0x4270, 0xfff8, { 18,  14,  11,  11,  11,  11,   4}},
	{ 0x4280, 0xfff8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0x4290, 0xfff8, { 20,  12,   8,   8,   8,   8,   4}},
	{ 0x4298, 0xfff8, { 20,  12,   8,   8,   8,   8,   4}},
	{ 0x42a0, 0xfff8, { 22,  14,   9,   9,   9,   9,   4}},
	{ 0x42a8, 0xfff8, { 24,  16,   9,   9,   9,   9,   4}},
	{ 0x42b0, 0xfff8, { 26,  20,  11,  11,  11,  11,   4}},
	{ 0x42c0, 0xfff8, {255,   4,   4,   4,   4,   4,   4}},
	{ 0x42d0, 0xfff8, {255,  12,   8,   8,   8,   8,   4}},
	{ 0x42d8, 0xfff8, {255,  12,   8,   8,   8,   8,   4}},
	{ 0x42e0, 0xfff8, {255,  14,   9,   9,   9,   9,   4}},
	{ 0x42e8, 0xfff8, {255,  16,   9,   9,   9,   9,   4}},
	{ 0x42f0, 0xfff8, {255,  18,  11,  11,  11,  11,   4}},
	{ 0x4400, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4410, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4418, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4420, 0xfff8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4428, 0xfff8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x4430, 0xfff8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x4440, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4450, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4458, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4460, 0xfff8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4468, 0xfff8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x4470, 0xfff8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x4480, 0xfff8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0x4490, 0xfff8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x4498, 0xfff8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x44a0, 0xfff8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x44a8, 0xfff8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x44b0, 0xfff8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x44c0, 0xfff8, { 12,  12,   4,   4,   4,   4,   4}},
	{ 0x44d0, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x44d8, 0xfff8, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x44e0, 0xfff8, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x44e8, 0xfff8, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x44f0, 0xfff8, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x4600, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4610, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4618, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4620, 0xfff8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4628, 0xfff8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x4630, 0xfff8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x4640, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4650, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4658, 0xfff8, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4660, 0xfff8, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4668, 0xfff8, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x4670, 0xfff8, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x4680, 0xfff8, {  6,   6,   2,   2,   2,   2,   2}},
	{ 0x4690, 0xfff8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x4698, 0xfff8, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x46a0, 0xfff8, { 22,  22,   9,   9,   9,   9,   4}},
	{ 0x46a8, 0xfff8, { 24,  24,   9,   9,   9,   9,   4}},
	{ 0x46b0, 0xfff8, { 26,  26,  11,  11,  11,  11,   4}},
	{ 0x46c0, 0xfff8, { 12,  12,   8,   8,   8,   8,   8}},
	{ 0x46d0, 0xfff8, { 16,  16,  12,  12,  12,  12,   8}},
	{ 0x46d8, 0xfff8, { 16,  16,  12,  12,  12,  12,   8}},
	{ 0x46e0, 0xfff8, { 18,  18,  13,  13,  13,  13,   8}},
	{ 0x46e8, 0xfff8, { 20,  20,  13,  13,  13,  13,   8}},
	{ 0x46f0, 0xfff8, { 22,  22,  15,  15,  15,  15,   8}},
	{ 0x4800, 0xfff8, {  6,   6,   6,   6,   6,   6,   6}},
	{ 0x4808, 0xfff8, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x4810, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x4818, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x4820, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x4828, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x4830, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x4840, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x4848, 0xfff8, {255,  10,  10,  10,  10,  10,  10}},
	{ 0x4850, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x4868, 0xfff8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0x4870, 0xfff8, { 20,  20,  12,  12,  12,  12,   5}},
	{ 0x4880, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x4890, 0xfff8, {  8,   8,   8,   8,   8,   8,   4}},
	{ 0x48a0, 0xfff8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x48a8, 0xfff8, { 12,  12,   9,   9,   9,   9,   4}},
	{ 0x48b0, 0xfff8, { 14,  14,  11,  11,  11,  11,   4}},
	{ 0x48c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x48d0, 0xfff8, {  8,   8,   8,   8,   8,   8,   4}},
	{ 0x48e0, 0xfff8, {  8,   8,   4,   4,   4,   4,   4}},
	{ 0x48e8, 0xfff8, { 12,  12,   9,   9,   9,   9,   4}},
	{ 0x48f0, 0xfff8, { 14,  14,  11,  11,  11,  11,   4}},
	{ 0x49c0, 0xfff8, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x4a00, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4a10, 0xfff8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x4a18, 0xfff8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x4a20, 0xfff8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x4a28, 0xfff8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x4a30, 0xfff8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x4a40, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4a48, 0xfff8, {255, 255,   2,   2,   2,   2,   2}},
	{ 0x4a50, 0xfff8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x4a58, 0xfff8, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x4a60, 0xfff8, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x4a68, 0xfff8, { 12,  12,   7,   7,   7,   7,   2}},
	{ 0x4a70, 0xfff8, { 14,  14,   9,   9,   9,   9,   2}},
	{ 0x4a80, 0xfff8, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4a88, 0xfff8, {255, 255,   2,   2,   2,   2,   2}},
	{ 0x4a90, 0xfff8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x4a98, 0xfff8, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x4aa0, 0xfff8, { 14,  14,   7,   7,   7,   7,   2}},
	{ 0x4aa8, 0xfff8, { 16,  16,   7,   7,   7,   7,   2}},
	{ 0x4ab0, 0xfff8, { 18,  18,   9,   9,   9,   9,   2}},
	{ 0x4ac0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x4ad0, 0xfff8, { 18,  18,  16,  16,  16,  16,  12}},
	{ 0x4ad8, 0xfff8, { 18,  18,  16,  16,  16,  16,  12}},
	{ 0x4ae0, 0xfff8, { 20,  20,  17,  17,  17,  17,  12}},
	{ 0x4ae8, 0xfff8, { 22,  22,  17,  17,  17,  17,  12}},
	{ 0x4af0, 0xfff8, { 24,  24,  19,  19,  19,  19,  12}},
	{ 0x4c00, 0xfff8, {255, 255,  43,  43,  43,  43,  43}},
	{ 0x4c10, 0xfff8, {255, 255,  47,  47,  47,  47,  43}},
	{ 0x4c18, 0xfff8, {255, 255,  47,  47,  47,  47,  43}},
	{ 0x4c20, 0xfff8, {255, 255,  48,  48,  48,  48,  43}},
	{ 0x4c28, 0xfff8, {255, 255,  48,  48,  48,  48,  43}},
	{ 0x4c30, 0xfff8, {255, 255,  50,  50,  50,  50,  43}},
	{ 0x4c40, 0xfff8, {255, 255,  84,  84,  84,  84,  84}},
	{ 0x4c50, 0xfff8, {255, 255,  88,  88,  88,  88,  84}},
	{ 0x4c58, 0xfff8, {255, 255,  88,  88,  88,  88,  84}},
	{ 0x4c60, 0xfff8, {255, 255,  89,  89,  89,  89,  84}},
	{ 0x4c68, 0xfff8, {255, 255,  89,  89,  89,  89,  84}},
	{ 0x4c70, 0xfff8, {255, 255,  91,  91,  91,  91,  84}},
	{ 0x4c90, 0xfff8, { 12,  12,  12,  12,  12,  12,   8}},
	{ 0x4c98, 0xfff8, { 12,  12,   8,   8,   8,   8,   8}},
	{ 0x4ca8, 0xfff8, { 16,  16,  13,  13,  13,  13,   8}},
	{ 0x4cb0, 0xfff8, { 18,  18,  15,  15,  15,  15,   8}},
	{ 0x4cd0, 0xfff8, { 12,  12,  12,  12,  12,  12,   8}},
	{ 0x4cd8, 0xfff8, { 12,  12,   8,   8,   8,   8,   8}},
	{ 0x4ce8, 0xfff8, { 16,  16,  13,  13,  13,  13,   8}},
	{ 0x4cf0, 0xfff8, { 18,  18,  15,  15,  15,  15,   8}},
	{ 0x4e50, 0xfff8, { 16,  16,   5,   5,   5,   5,   5}},
	{ 0x4e58, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x4e60, 0xfff8, {  4,   6,   2,   2,   2,   2,   2}},
	{ 0x4e68, 0xfff8, {  4,   6,   2,   2,   2,   2,   2}},
	{ 0x4e90, 0xfff8, { 16,  16,   4,   4,   4,   4,   0}},
	{ 0x4ea8, 0xfff8, { 18,  18,   5,   5,   5,   5,   0}},
	{ 0x4eb0, 0xfff8, { 22,  22,   7,   7,   7,   7,   0}},
	{ 0x4ed0, 0xfff8, {  8,   8,   4,   4,   4,   4,   0}},
	{ 0x4ee8, 0xfff8, { 10,  10,   5,   5,   5,   5,   0}},
	{ 0x4ef0, 0xfff8, { 14,  14,   7,   7,   7,   7,   0}},
	{ 0x50c0, 0xfff8, {  6,   4,   4,   4,   4,   4,   4}},
	{ 0x50c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x50d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x50d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x50e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x50e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x50f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x51c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x51c8, 0xfff8, { 12,  12,   6,   4,   4,   4,   4}},
	{ 0x51d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x51d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x51e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x51e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x51f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x52c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x52c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x52d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x52d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x52e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x52e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x52f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x53c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x53c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x53d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x53d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x53e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x53e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x53f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x54c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x54c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x54d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x54d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x54e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x54e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x54f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x55c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x55c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x55d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x55d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x55e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x55e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x55f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x56c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x56c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x56d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x56d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x56e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x56e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x56f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x57c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x57c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x57d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x57d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x57e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x57e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x57f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x58c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x58c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x58d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x58d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x58e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x58e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x58f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x59c0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x59c8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x59d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x59d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x59e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x59e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x59f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x5ac0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x5ac8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x5ad0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5ad8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5ae0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5ae8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x5af0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x5bc0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x5bc8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x5bd0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5bd8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5be0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5be8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x5bf0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x5cc0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x5cc8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x5cd0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5cd8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5ce0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5ce8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x5cf0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x5dc0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x5dc8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x5dd0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5dd8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5de0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5de8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x5df0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x5ec0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x5ec8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x5ed0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5ed8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5ee0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5ee8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x5ef0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x5fc0, 0xfff8, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x5fc8, 0xfff8, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x5fd0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5fd8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5fe0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5fe8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0x5ff0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0x8f08, 0xfff8, { 18,  18,  16,  16,  16,  16,  16}},
	{ 0x8f48, 0xfff8, {255, 255,  13,  13,  13,  13,  13}},
	{ 0x8f88, 0xfff8, {255, 255,  13,  13,  13,  13,  13}},
	{ 0x9f08, 0xfff8, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0xbf08, 0xfff8, { 12,  12,   9,   9,   9,   9,   9}},
	{ 0xcf08, 0xfff8, { 18,  18,  16,  16,  16,  16,  16}},
	{ 0xdf08, 0xfff8, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0xe0d0, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe0d8, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe0e0, 0xfff8, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0xe0e8, 0xfff8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0xe0f0, 0xfff8, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0xe1d0, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0xe1d8, 0xfff8, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0xe1e0, 0xfff8, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0xe1e8, 0xfff8, { 16,  16,  11,  11,  11,  11,   6}},
	{ 0xe1f0, 0xfff8, { 18,  18,  13,  13,  13,  13,   6}},
	{ 0xe2d0, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe2d8, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe2e0, 0xfff8, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0xe2e8, 0xfff8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0xe2f0, 0xfff8, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0xe3d0, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe3d8, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe3e0, 0xfff8, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0xe3e8, 0xfff8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0xe3f0, 0xfff8, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0xe4d0, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe4d8, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe4e0, 0xfff8, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0xe4e8, 0xfff8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0xe4f0, 0xfff8, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0xe5d0, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe5d8, 0xfff8, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0xe5e0, 0xfff8, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0xe5e8, 0xfff8, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0xe5f0, 0xfff8, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0xe6d0, 0xfff8, { 12,  12,  11,  11,  11,  11,   7}},
	{ 0xe6d8, 0xfff8, { 12,  12,  11,  11,  11,  11,   7}},
	{ 0xe6e0, 0xfff8, { 14,  14,  12,  12,  12,  12,   7}},
	{ 0xe6e8, 0xfff8, { 16,  16,  12,  12,  12,  12,   7}},
	{ 0xe6f0, 0xfff8, { 18,  18,  14,  14,  14,  14,   7}},
	{ 0xe7d0, 0xfff8, { 12,  12,  11,  11,  11,  11,   7}},
	{ 0xe7d8, 0xfff8, { 12,  12,  11,  11,  11,  11,   7}},
	{ 0xe7e0, 0xfff8, { 14,  14,  12,  12,  12,  12,   7}},
	{ 0xe7e8, 0xfff8, { 16,  16,  12,  12,  12,  12,   7}},
	{ 0xe7f0, 0xfff8, { 18,  18,  14,  14,  14,  14,   7}},
	{ 0xe8c0, 0xfff8, {255, 255,   6,   6,   6,   6,   6}},
	{ 0xe8d0, 0xfff8, {255, 255,  17,  17,  17,  17,  13}},
	{ 0xe8e8, 0xfff8, {255, 255,  18,  18,  18,  18,  13}},
	{ 0xe8f0, 0xfff8, {255, 255,  20,  20,  20,  20,  13}},
	{ 0xe9c0, 0xfff8, {255, 255,   8,   8,   8,   8,   8}},
	{ 0xe9d0, 0xfff8, {255, 255,  19,  19,  19,  19,  15}},
	{ 0xe9e8, 0xfff8, {255, 255,  20,  20,  20,  20,  15}},
	{ 0xe9f0, 0xfff8, {255, 255,  22,  22,  22,  22,  15}},
	{ 0xeac0, 0xfff8, {255, 255,  12,  12,  12,  12,  12}},
	{ 0xead0, 0xfff8, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xeae8, 0xfff8, {255, 255,  25,  25,  25,  25,  20}},
	{ 0xeaf0, 0xfff8, {255, 255,  27,  27,  27,  27,  20}},
	{ 0xebc0, 0xfff8, {255, 255,   8,   8,   8,   8,   8}},
	{ 0xebd0, 0xfff8, {255, 255,  19,  19,  19,  19,  15}},
	{ 0xebe8, 0xfff8, {255, 255,  20,  20,  20,  20,  15}},
	{ 0xebf0, 0xfff8, {255, 255,  22,  22,  22,  22,  15}},
	{ 0xecc0, 0xfff8, {255, 255,  12,  12,  12,  12,  12}},
	{ 0xecd0, 0xfff8, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xece8, 0xfff8, {255, 255,  25,  25,  25,  25,  20}},
	{ 0xecf0, 0xfff8, {255, 255,  27,  27,  27,  27,  20}},
	{ 0xedc0, 0xfff8, {255, 255,  18,  18,  18,  18,  18}},
	{ 0xedd0, 0xfff8, {255, 255,  32,  32,  32,  32,  28}},
	{ 0xede8, 0xfff8, {255, 255,  33,  33,  33,  33,  28}},
	{ 0xedf0, 0xfff8, {255, 255,  35,  35,  35,  35,  28}},
	{ 0xeec0, 0xfff8, {255, 255,  12,  12,  12,  12,  12}},
	{ 0xeed0, 0xfff8, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xeee8, 0xfff8, {255, 255,  25,  25,  25,  25,  20}},
	{ 0xeef0, 0xfff8, {255, 255,  27,  27,  27,  27,  20}},
	{ 0xefc0, 0xfff8, {255, 255,  10,  10,  10,  10,  10}},
	{ 0xefd0, 0xfff8, {255, 255,  21,  21,  21,  21,  17}},
	{ 0xefe8, 0xfff8, {255, 255,  22,  22,  22,  22,  17}},
	{ 0xeff0, 0xfff8, {255, 255,  24,  24,  24,  24,  17}},
	{ 0xf278, 0xfff8, {255, 255,   4,   4, 255, 255, 255}},
	{ 0xf510, 0xfff8, {255, 255, 255, 255,   4,   4,   4}},
	{ 0xf518, 0xfff8, {255, 255, 255, 255,   4,   4,   4}},
	{ 0xf620, 0xfff8, {255, 255, 255, 255,   4,   4,   4}},
	{ 0x001f, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0027, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0038, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0039, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x003c, 0xffff, { 20,  16,  12,  12,  12,  12,  12}},
	{ 0x0078, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0079, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x007c, 0xffff, { 20,  16,  12,  12,  12,  12,  12}},
	{ 0x00b8, 0xffff, { 32,  32,   8,   8,   8,   8,   4}},
	{ 0x00b9, 0xffff, { 36,  36,   8,   8,   8,   8,   4}},
	{ 0x00f8, 0xffff, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x00f9, 0xffff, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x00fa, 0xffff, {255, 255,  23,  23,  23,  23,  23}},
	{ 0x00fb, 0xffff, {255, 255,  23,  23,  23,  23,  23}},
	{ 0x021f, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0227, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0238, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0239, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x023c, 0xffff, { 20,  16,  12,  12,  12,  12,  12}},
	{ 0x0278, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0279, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x027c, 0xffff, { 20,  16,  12,  12,  12,  12,  12}},
	{ 0x02b8, 0xffff, { 32,  32,   8,   8,   8,   8,   4}},
	{ 0x02b9, 0xffff, { 36,  36,   8,   8,   8,   8,   4}},
	{ 0x02f8, 0xffff, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x02f9, 0xffff, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x02fa, 0xffff, {255, 255,  23,  23,  23,  23,  23}},
	{ 0x02fb, 0xffff, {255, 255,  23,  23,  23,  23,  23}},
	{ 0x041f, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0427, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0438, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0439, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x0478, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0479, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x04b8, 0xffff, { 32,  32,   8,   8,   8,   8,   4}},
	{ 0x04b9, 0xffff, { 36,  36,   8,   8,   8,   8,   4}},
	{ 0x04f8, 0xffff, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x04f9, 0xffff, {255, 255,  22,  22,  22,  22,  18}},
	{ 0x04fa, 0xffff, {255, 255,  23,  23,  23,  23,  23}},
	{ 0x04fb, 0xffff, {255, 255,  23,  23,  23,  23,  23}},
	{ 0x061f, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0627, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0638, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0639, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x0678, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0679, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x06b8, 0xffff, { 32,  32,   8,   8,   8,   8,   4}},
	{ 0x06b9, 0xffff, { 36,  36,   8,   8,   8,   8,   4}},
	{ 0x06f8, 0xffff, {255, 255,  64,  64,  64,  64,  60}},
	{ 0x06f9, 0xffff, {255, 255,  64,  64,  64,  64,  60}},
	{ 0x06fa, 0xffff, {255, 255,  65,  65,  65,  65,  60}},
	{ 0x06fb, 0xffff, {255, 255,  67,  67,  67,  67,  60}},
	{ 0x081f, 0xffff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x0827, 0xffff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x0838, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0839, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x083a, 0xffff, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x083b, 0xffff, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x085f, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0867, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0878, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0879, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x089f, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x08a7, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x08b8, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x08b9, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x08df, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x08e7, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x08f8, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x08f9, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x0a1f, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x0a27, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x0a38, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0a39, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x0a3c, 0xffff, { 20,  16,  12,  12,  12,  12,  12}},
	{ 0x0a78, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x0a79, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x0a7c, 0xffff, { 20,  16,  12,  12,  12,  12,  12}},
	{ 0x0ab8, 0xffff, { 32,  32,   8,   8,   8,   8,   4}},
	{ 0x0ab9, 0xffff, { 36,  36,   8,   8,   8,   8,   4}},
	{ 0x0adf, 0xffff, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0ae7, 0xffff, {255, 255,  17,  17,  17,  17,  12}},
	{ 0x0af8, 0xffff, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0af9, 0xffff, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0c1f, 0xffff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x0c27, 0xffff, { 14,  14,   7,   7,   7,   7,   2}},
	{ 0x0c38, 0xffff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x0c39, 0xffff, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0x0c3a, 0xffff, {255, 255,   7,   7,   7,   7,   7}},
	{ 0x0c3b, 0xffff, {255, 255,   9,   9,   9,   9,   9}},
	{ 0x0c78, 0xffff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x0c79, 0xffff, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0x0c7a, 0xffff, {255, 255,   7,   7,   7,   7,   7}},
	{ 0x0c7b, 0xffff, {255, 255,   9,   9,   9,   9,   9}},
	{ 0x0cb8, 0xffff, { 24,  24,   6,   6,   6,   6,   2}},
	{ 0x0cb9, 0xffff, { 28,  28,   6,   6,   6,   6,   2}},
	{ 0x0cba, 0xffff, {255, 255,   7,   7,   7,   7,   7}},
	{ 0x0cbb, 0xffff, {255, 255,   9,   9,   9,   9,   9}},
	{ 0x0cf8, 0xffff, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0cf9, 0xffff, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0cfc, 0xffff, {255, 255,  12,  12,  12,  12,  12}},
	{ 0x0e1f, 0xffff, {255,  18,   9,   9,   9,   9,   5}},
	{ 0x0e27, 0xffff, {255,  20,  10,  10,  10,  10,   5}},
	{ 0x0e38, 0xffff, {255,  26,   9,   9,   9,   9,   5}},
	{ 0x0e39, 0xffff, {255,  30,   9,   9,   9,   9,   5}},
	{ 0x0e78, 0xffff, {255,  26,   9,   9,   9,   9,   5}},
	{ 0x0e79, 0xffff, {255,  30,   9,   9,   9,   9,   5}},
	{ 0x0eb8, 0xffff, {255,  32,   9,   9,   9,   9,   5}},
	{ 0x0eb9, 0xffff, {255,  36,   9,   9,   9,   9,   5}},
	{ 0x0ef8, 0xffff, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0ef9, 0xffff, {255, 255,  16,  16,  16,  16,  12}},
	{ 0x0efc, 0xffff, {255, 255,  12,  12,  12,  12,  12}},
	{ 0x11df, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x11e7, 0xffff, { 18,  18,   9,   9,   9,   9,   4}},
	{ 0x11f8, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x11f9, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x11fa, 0xffff, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x11fb, 0xffff, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x11fc, 0xffff, { 16,  16,   6,   6,   6,   6,   4}},
	{ 0x13df, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x13e7, 0xffff, { 22,  22,  11,  11,  11,  11,   6}},
	{ 0x13f8, 0xffff, { 24,  24,  10,  10,  10,  10,   6}},
	{ 0x13f9, 0xffff, { 28,  28,  10,  10,  10,  10,   6}},
	{ 0x13fa, 0xffff, { 24,  24,  11,  11,  11,  11,   6}},
	{ 0x13fb, 0xffff, { 26,  26,  13,  13,  13,  13,   6}},
	{ 0x13fc, 0xffff, { 20,  20,   8,   8,   8,   8,   6}},
	{ 0x1edf, 0xffff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x1ee7, 0xffff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x1ef8, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x1ef9, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x1efa, 0xffff, { 16,  16,   9,   9,   9,   9,   4}},
	{ 0x1efb, 0xffff, { 18,  18,  11,  11,  11,  11,   4}},
	{ 0x1efc, 0xffff, { 12,  12,   6,   6,   6,   6,   4}},
	{ 0x1f1f, 0xffff, { 12,  12,   9,   9,   9,   9,   5}},
	{ 0x1f27, 0xffff, { 14,  14,  10,  10,  10,  10,   5}},
	{ 0x1f38, 0xffff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x1f39, 0xffff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0x1f3a, 0xffff, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0x1f3b, 0xffff, { 18,  18,  12,  12,  12,  12,   5}},
	{ 0x1f3c, 0xffff, { 12,  12,   7,   7,   7,   7,   5}},
	{ 0x21f8, 0xffff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x21f9, 0xffff, { 32,  32,   8,   8,   8,   8,   4}},
	{ 0x21fa, 0xffff, { 28,  28,   9,   9,   9,   9,   4}},
	{ 0x21fb, 0xffff, { 30,  30,  11,  11,  11,  11,   4}},
	{ 0x21fc, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x23f8, 0xffff, { 32,  32,  10,  10,  10,  10,   6}},
	{ 0x23f9, 0xffff, { 36,  36,  10,  10,  10,  10,   6}},
	{ 0x23fa, 0xffff, { 32,  32,  11,  11,  11,  11,   6}},
	{ 0x23fb, 0xffff, { 34,  34,  13,  13,  13,  13,   6}},
	{ 0x23fc, 0xffff, { 28,  28,  10,  10,  10,  10,   6}},
	{ 0x31f8, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x31f9, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x31fa, 0xffff, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x31fb, 0xffff, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x31fc, 0xffff, { 16,  16,   6,   6,   6,   6,   4}},
	{ 0x33f8, 0xffff, { 24,  24,  10,  10,  10,  10,   6}},
	{ 0x33f9, 0xffff, { 28,  28,  10,  10,  10,  10,   6}},
	{ 0x33fa, 0xffff, { 24,  24,  11,  11,  11,  11,   6}},
	{ 0x33fb, 0xffff, { 26,  26,  13,  13,  13,  13,   6}},
	{ 0x33fc, 0xffff, { 20,  20,   8,   8,   8,   8,   6}},
	{ 0x401f, 0xffff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4027, 0xffff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4038, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x4039, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x4078, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x4079, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x40b8, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x40b9, 0xffff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x40f8, 0xffff, { 16,  16,  12,  12,  12,  12,   8}},
	{ 0x40f9, 0xffff, { 20,  20,  12,  12,  12,  12,   8}},
	{ 0x421f, 0xffff, { 12,   8,   8,   8,   8,   8,   4}},
	{ 0x4227, 0xffff, { 14,  10,   9,   9,   9,   9,   4}},
	{ 0x4238, 0xffff, { 16,  12,   8,   8,   8,   8,   4}},
	{ 0x4239, 0xffff, { 20,  14,   8,   8,   8,   8,   4}},
	{ 0x4278, 0xffff, { 16,  12,   8,   8,   8,   8,   4}},
	{ 0x4279, 0xffff, { 20,  14,   8,   8,   8,   8,   4}},
	{ 0x42b8, 0xffff, { 24,  16,   8,   8,   8,   8,   4}},
	{ 0x42b9, 0xffff, { 28,  20,   8,   8,   8,   8,   4}},
	{ 0x42f8, 0xffff, {255,  16,   8,   8,   8,   8,   4}},
	{ 0x42f9, 0xffff, {255,  20,   8,   8,   8,   8,   4}},
	{ 0x441f, 0xffff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4427, 0xffff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4438, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x4439, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x4478, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x4479, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x44b8, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x44b9, 0xffff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x44f8, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x44f9, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x44fa, 0xffff, { 20,  20,   9,   9,   9,   9,   4}},
	{ 0x44fb, 0xffff, { 22,  22,  11,  11,  11,  11,   4}},
	{ 0x44fc, 0xffff, { 16,  16,   6,   6,   6,   6,   4}},
	{ 0x461f, 0xffff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x4627, 0xffff, { 14,  14,   9,   9,   9,   9,   4}},
	{ 0x4638, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x4639, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x4678, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x4679, 0xffff, { 20,  20,   8,   8,   8,   8,   4}},
	{ 0x46b8, 0xffff, { 24,  24,   8,   8,   8,   8,   4}},
	{ 0x46b9, 0xffff, { 28,  28,   8,   8,   8,   8,   4}},
	{ 0x46f8, 0xffff, { 20,  20,  12,  12,  12,  12,   8}},
	{ 0x46f9, 0xffff, { 24,  24,  12,  12,  12,  12,   8}},
	{ 0x46fa, 0xffff, { 20,  20,  13,  13,  13,  13,   8}},
	{ 0x46fb, 0xffff, { 22,  22,  15,  15,  15,  15,   8}},
	{ 0x46fc, 0xffff, { 16,  16,  10,  10,  10,  10,   8}},
	{ 0x480f, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x481f, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x4827, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x4838, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x4839, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x4878, 0xffff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0x4879, 0xffff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0x487a, 0xffff, { 16,  16,  10,  10,  10,  10,   5}},
	{ 0x487b, 0xffff, { 20,  20,  12,  12,  12,  12,   5}},
	{ 0x48b8, 0xffff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x48b9, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x48f8, 0xffff, { 12,  12,   8,   8,   8,   8,   4}},
	{ 0x48f9, 0xffff, { 16,  16,   8,   8,   8,   8,   4}},
	{ 0x4a1f, 0xffff, {  8,   8,   6,   6,   6,   6,   2}},
	{ 0x4a27, 0xffff, { 10,  10,   7,   7,   7,   7,   2}},
	{ 0x4a38, 0xffff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x4a39, 0xffff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x4a3a, 0xffff, {255, 255,   7,   7,   7,   7,   7}},
	{ 0x4a3b, 0xffff, {255, 255,   9,   9,   9,   9,   9}},
	{ 0x4a3c, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x4a78, 0xffff, { 12,  12,   6,   6,   6,   6,   2}},
	{ 0x4a79, 0xffff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x4a7a, 0xffff, {255, 255,   7,   7,   7,   7,   7}},
	{ 0x4a7b, 0xffff, {255, 255,   9,   9,   9,   9,   9}},
	{ 0x4a7c, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x4ab8, 0xffff, { 16,  16,   6,   6,   6,   6,   2}},
	{ 0x4ab9, 0xffff, { 20,  20,   6,   6,   6,   6,   2}},
	{ 0x4aba, 0xffff, {255, 255,   7,   7,   7,   7,   7}},
	{ 0x4abb, 0xffff, {255, 255,   9,   9,   9,   9,   9}},
	{ 0x4abc, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x4adf, 0xffff, { 18,  18,  16,  16,  16,  16,  12}},
	{ 0x4ae7, 0xffff, { 20,  20,  17,  17,  17,  17,  12}},
	{ 0x4af8, 0xffff, { 22,  22,  16,  16,  16,  16,  12}},
	{ 0x4af9, 0xffff, { 26,  26,  16,  16,  16,  16,  12}},
	{ 0x4afc, 0xffff, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x4c38, 0xffff, {255, 255,  47,  47,  47,  47,  43}},
	{ 0x4c39, 0xffff, {255, 255,  47,  47,  47,  47,  43}},
	{ 0x4c3a, 0xffff, {255, 255,  48,  48,  48,  48,  43}},
	{ 0x4c3b, 0xffff, {255, 255,  50,  50,  50,  50,  43}},
	{ 0x4c3c, 0xffff, {255, 255,  47,  47,  47,  47,  43}},
	{ 0x4c78, 0xffff, {255, 255,  88,  88,  88,  88,  84}},
	{ 0x4c79, 0xffff, {255, 255,  88,  88,  88,  88,  84}},
	{ 0x4c7a, 0xffff, {255, 255,  89,  89,  89,  89,  84}},
	{ 0x4c7b, 0xffff, {255, 255,  91,  91,  91,  91,  84}},
	{ 0x4c7c, 0xffff, {255, 255,  88,  88,  88,  88,  84}},
	{ 0x4cb8, 0xffff, { 16,  16,  12,  12,  12,  12,   8}},
	{ 0x4cb9, 0xffff, { 20,  20,  12,  12,  12,  12,   8}},
	{ 0x4cba, 0xffff, { 16,  16,   9,   9,   9,   9,   9}},
	{ 0x4cbb, 0xffff, { 18,  18,  11,  11,  11,  11,  11}},
	{ 0x4cf8, 0xffff, { 16,  16,  12,  12,  12,  12,   8}},
	{ 0x4cf9, 0xffff, { 20,  20,  12,  12,  12,  12,   8}},
	{ 0x4cfa, 0xffff, { 16,  16,   9,   9,   9,   9,   9}},
	{ 0x4cfb, 0xffff, { 18,  18,  11,  11,  11,  11,  11}},
	{ 0x4e57, 0xffff, { 16,  16,   5,   5,   5,   5,   5}},
	{ 0x4e5f, 0xffff, { 12,  12,   6,   6,   6,   6,   6}},
	{ 0x4e70, 0xffff, {  0,   0,   0,   0,   0,   0,   0}},
	{ 0x4e71, 0xffff, {  4,   4,   2,   2,   2,   2,   2}},
	{ 0x4e72, 0xffff, {  4,   4,   8,   8,   8,   8,   8}},
	{ 0x4e73, 0xffff, { 20,  24,  20,  20,  20,  20,  20}},
	{ 0x4e74, 0xffff, {255,  16,  10,  10,  10,  10,  10}},
	{ 0x4e75, 0xffff, { 16,  16,  10,  10,  10,  10,  10}},
	{ 0x4e76, 0xffff, {  4,   4,   4,   4,   4,   4,   4}},
	{ 0x4e77, 0xffff, { 20,  20,  14,  14,  14,  14,  14}},
	{ 0x4e7a, 0xffff, {255,  12,   6,   6,   6,   6,   6}},
	{ 0x4e7b, 0xffff, {255,  10,  12,  12,  12,  12,  12}},
	{ 0x4eb8, 0xffff, { 18,  18,   4,   4,   4,   4,   0}},
	{ 0x4eb9, 0xffff, { 20,  20,   4,   4,   4,   4,   0}},
	{ 0x4eba, 0xffff, { 18,  18,   5,   5,   5,   5,   0}},
	{ 0x4ebb, 0xffff, { 22,  22,   7,   7,   7,   7,   0}},
	{ 0x4ef8, 0xffff, { 10,  10,   4,   4,   4,   4,   0}},
	{ 0x4ef9, 0xffff, { 12,  12,   4,   4,   4,   4,   0}},
	{ 0x4efa, 0xffff, { 10,  10,   5,   5,   5,   5,   0}},
	{ 0x4efb, 0xffff, { 14,  14,   7,   7,   7,   7,   0}},
	{ 0x50df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x50e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x50f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x50f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x50fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x50fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x50fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x51df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x51e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x51f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x51f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x51fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x51fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x51fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x52df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x52e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x52f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x52f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x52fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x52fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x52fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x53df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x53e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x53f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x53f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x53fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x53fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x53fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x54df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x54e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x54f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x54f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x54fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x54fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x54fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x55df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x55e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x55f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x55f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x55fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x55fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x55fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x56df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x56e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x56f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x56f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x56fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x56fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x56fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x57df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x57e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x57f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x57f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x57fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x57fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x57fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x58df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x58e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x58f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x58f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x58fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x58fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x58fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x59df, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x59e7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x59f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x59f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x59fa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x59fb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x59fc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x5adf, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5ae7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5af8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x5af9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x5afa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x5afb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x5afc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x5bdf, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5be7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5bf8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x5bf9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x5bfa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x5bfb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x5bfc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x5cdf, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5ce7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5cf8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x5cf9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x5cfa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x5cfb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x5cfc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x5ddf, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5de7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5df8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x5df9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x5dfa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x5dfb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x5dfc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x5edf, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5ee7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5ef8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x5ef9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x5efa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x5efb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x5efc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x5fdf, 0xffff, { 12,  12,  10,  10,  10,  10,   6}},
	{ 0x5fe7, 0xffff, { 14,  14,  11,  11,  11,  11,   6}},
	{ 0x5ff8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0x5ff9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0x5ffa, 0xffff, {255, 255,   6,   6,   6,   6,   6}},
	{ 0x5ffb, 0xffff, {255, 255,   8,   8,   8,   8,   8}},
	{ 0x5ffc, 0xffff, {255, 255,   4,   4,   4,   4,   4}},
	{ 0x6000, 0xffff, { 10,  10,  10,  10,  10,  10,  10}},
	{ 0x60ff, 0xffff, { 10,  10,  10,  10,  10,  10,  10}},
	{ 0x6100, 0xffff, { 18,  18,   7,   7,   7,   7,   7}},
	{ 0x61ff, 0xffff, { 18,  18,   7,   7,   7,   7,   7}},
	{ 0x6200, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x62ff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6300, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x63ff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6400, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x64ff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6500, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x65ff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6600, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x66ff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6700, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x67ff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6800, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x68ff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6900, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x69ff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6a00, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6aff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6b00, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6bff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6c00, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6cff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6d00, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6dff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6e00, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6eff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6f00, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x6fff, 0xffff, { 10,  10,   6,   6,   6,   6,   6}},
	{ 0x8f0f, 0xffff, { 18,  18,  16,  16,  16,  16,  16}},
	{ 0x8f4f, 0xffff, {255, 255,  13,  13,  13,  13,  13}},
	{ 0x8f8f, 0xffff, {255, 255,  13,  13,  13,  13,  13}},
	{ 0x9f0f, 0xffff, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0xbf0f, 0xffff, { 12,  12,   9,   9,   9,   9,   9}},
	{ 0xcf0f, 0xffff, { 18,  18,  16,  16,  16,  16,  16}},
	{ 0xdf0f, 0xffff, { 18,  18,  12,  12,  12,  12,  12}},
	{ 0xe0f8, 0xffff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0xe0f9, 0xffff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0xe1f8, 0xffff, { 16,  16,  10,  10,  10,  10,   6}},
	{ 0xe1f9, 0xffff, { 20,  20,  10,  10,  10,  10,   6}},
	{ 0xe2f8, 0xffff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0xe2f9, 0xffff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0xe3f8, 0xffff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0xe3f9, 0xffff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0xe4f8, 0xffff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0xe4f9, 0xffff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0xe5f8, 0xffff, { 16,  16,   9,   9,   9,   9,   5}},
	{ 0xe5f9, 0xffff, { 20,  20,   9,   9,   9,   9,   5}},
	{ 0xe6f8, 0xffff, { 16,  16,  11,  11,  11,  11,   7}},
	{ 0xe6f9, 0xffff, { 20,  20,  11,  11,  11,  11,   7}},
	{ 0xe7f8, 0xffff, { 16,  16,  11,  11,  11,  11,   7}},
	{ 0xe7f9, 0xffff, { 20,  20,  11,  11,  11,  11,   7}},
	{ 0xe8f8, 0xffff, {255, 255,  17,  17,  17,  17,  13}},
	{ 0xe8f9, 0xffff, {255, 255,  17,  17,  17,  17,  13}},
	{ 0xe8fa, 0xffff, {255, 255,  18,  18,  18,  18,  13}},
	{ 0xe8fb, 0xffff, {255, 255,  20,  20,  20,  20,  13}},
	{ 0xe9f8, 0xffff, {255, 255,  19,  19,  19,  19,  15}},
	{ 0xe9f9, 0xffff, {255, 255,  19,  19,  19,  19,  15}},
	{ 0xe9fa, 0xffff, {255, 255,  20,  20,  20,  20,  15}},
	{ 0xe9fb, 0xffff, {255, 255,  22,  22,  22,  22,  15}},
	{ 0xeaf8, 0xffff, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xeaf9, 0xffff, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xebf8, 0xffff, {255, 255,  19,  19,  19,  19,  15}},
	{ 0xebf9, 0xffff, {255, 255,  19,  19,  19,  19,  15}},
	{ 0xebfa, 0xffff, {255, 255,  20,  20,  20,  20,  15}},
	{ 0xebfb, 0xffff, {255, 255,  22,  22,  22,  22,  15}},
	{ 0xecf8, 0xffff, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xecf9, 0xffff, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xedf8, 0xffff, {255, 255,  32,  32,  32,  32,  28}},
	{ 0xedf9, 0xffff, {255, 255,  32,  32,  32,  32,  28}},
	{ 0xedfa, 0xffff, {255, 255,  33,  33,  33,  33,  28}},
	{ 0xedfb, 0xffff, {255, 255,  35,  35,  35,  35,  28}},
	{ 0xeef8, 0xffff, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xeef9, 0xffff, {255, 255,  24,  24,  24,  24,  20}},
	{ 0xeff8, 0xffff, {255, 255,  21,  21,  21,  21,  17}},
	{ 0xeff9, 0xffff, {255, 255,  21,  21,  21,  21,  17}},
	{ 0, 0, {0, 0, 0, 0, 0}}
};
