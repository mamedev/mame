#include "emu.h"
#include "m68000.h"

/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */


void m68000_base_device::m68k_op_1010()
{
	m68ki_exception_1010();
}


void m68000_base_device::m68k_op_1111()
{
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_040fpu0_32()
{
	if(m_has_fpu)
	{
		m68040_fpu_op0();
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_040fpu1_32()
{
	if(m_has_fpu)
	{
		m68040_fpu_op1();
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_abcd_8_rr()
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


void m68000_base_device::m68k_op_abcd_8_mm_ax7()
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


void m68000_base_device::m68k_op_abcd_8_mm_ay7()
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


void m68000_base_device::m68k_op_abcd_8_mm_axy7()
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


void m68000_base_device::m68k_op_abcd_8_mm()
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


void m68000_base_device::m68k_op_add_8_er_d()
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


void m68000_base_device::m68k_op_add_8_er_ai()
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


void m68000_base_device::m68k_op_add_8_er_pi()
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


void m68000_base_device::m68k_op_add_8_er_pi7()
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


void m68000_base_device::m68k_op_add_8_er_pd()
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


void m68000_base_device::m68k_op_add_8_er_pd7()
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


void m68000_base_device::m68k_op_add_8_er_di()
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


void m68000_base_device::m68k_op_add_8_er_ix()
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


void m68000_base_device::m68k_op_add_8_er_aw()
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


void m68000_base_device::m68k_op_add_8_er_al()
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


void m68000_base_device::m68k_op_add_8_er_pcdi()
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


void m68000_base_device::m68k_op_add_8_er_pcix()
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


void m68000_base_device::m68k_op_add_8_er_i()
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


void m68000_base_device::m68k_op_add_16_er_d()
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


void m68000_base_device::m68k_op_add_16_er_a()
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


void m68000_base_device::m68k_op_add_16_er_ai()
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


void m68000_base_device::m68k_op_add_16_er_pi()
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


void m68000_base_device::m68k_op_add_16_er_pd()
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


void m68000_base_device::m68k_op_add_16_er_di()
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


void m68000_base_device::m68k_op_add_16_er_ix()
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


void m68000_base_device::m68k_op_add_16_er_aw()
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


void m68000_base_device::m68k_op_add_16_er_al()
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


void m68000_base_device::m68k_op_add_16_er_pcdi()
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


void m68000_base_device::m68k_op_add_16_er_pcix()
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


void m68000_base_device::m68k_op_add_16_er_i()
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


void m68000_base_device::m68k_op_add_32_er_d()
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


void m68000_base_device::m68k_op_add_32_er_a()
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


void m68000_base_device::m68k_op_add_32_er_ai()
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


void m68000_base_device::m68k_op_add_32_er_pi()
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


void m68000_base_device::m68k_op_add_32_er_pd()
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


void m68000_base_device::m68k_op_add_32_er_di()
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


void m68000_base_device::m68k_op_add_32_er_ix()
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


void m68000_base_device::m68k_op_add_32_er_aw()
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


void m68000_base_device::m68k_op_add_32_er_al()
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


void m68000_base_device::m68k_op_add_32_er_pcdi()
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


void m68000_base_device::m68k_op_add_32_er_pcix()
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


void m68000_base_device::m68k_op_add_32_er_i()
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


void m68000_base_device::m68k_op_add_8_re_ai()
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


void m68000_base_device::m68k_op_add_8_re_pi()
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


void m68000_base_device::m68k_op_add_8_re_pi7()
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


void m68000_base_device::m68k_op_add_8_re_pd()
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


void m68000_base_device::m68k_op_add_8_re_pd7()
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


void m68000_base_device::m68k_op_add_8_re_di()
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


void m68000_base_device::m68k_op_add_8_re_ix()
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


void m68000_base_device::m68k_op_add_8_re_aw()
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


void m68000_base_device::m68k_op_add_8_re_al()
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


void m68000_base_device::m68k_op_add_16_re_ai()
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


void m68000_base_device::m68k_op_add_16_re_pi()
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


void m68000_base_device::m68k_op_add_16_re_pd()
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


void m68000_base_device::m68k_op_add_16_re_di()
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


void m68000_base_device::m68k_op_add_16_re_ix()
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


void m68000_base_device::m68k_op_add_16_re_aw()
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


void m68000_base_device::m68k_op_add_16_re_al()
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


void m68000_base_device::m68k_op_add_32_re_ai()
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


void m68000_base_device::m68k_op_add_32_re_pi()
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


void m68000_base_device::m68k_op_add_32_re_pd()
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


void m68000_base_device::m68k_op_add_32_re_di()
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


void m68000_base_device::m68k_op_add_32_re_ix()
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


void m68000_base_device::m68k_op_add_32_re_aw()
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


void m68000_base_device::m68k_op_add_32_re_al()
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


void m68000_base_device::m68k_op_adda_16_d()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(DY()));
}


void m68000_base_device::m68k_op_adda_16_a()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(AY()));
}


void m68000_base_device::m68k_op_adda_16_ai()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_pi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_pd()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_di()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_ix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_aw()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AW_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_al()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AL_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_pcdi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_pcix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_16_i()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_I_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_d()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + DY());
}


void m68000_base_device::m68k_op_adda_32_a()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + AY());
}


void m68000_base_device::m68k_op_adda_32_ai()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_AI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_pi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_pd()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PD_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_di()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_DI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_ix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_IX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_aw()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AW_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_al()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AL_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_pcdi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCDI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_pcix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCIX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_adda_32_i()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_I_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


void m68000_base_device::m68k_op_addi_8_d()
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


void m68000_base_device::m68k_op_addi_8_ai()
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


void m68000_base_device::m68k_op_addi_8_pi()
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


void m68000_base_device::m68k_op_addi_8_pi7()
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


void m68000_base_device::m68k_op_addi_8_pd()
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


void m68000_base_device::m68k_op_addi_8_pd7()
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


void m68000_base_device::m68k_op_addi_8_di()
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


void m68000_base_device::m68k_op_addi_8_ix()
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


void m68000_base_device::m68k_op_addi_8_aw()
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


void m68000_base_device::m68k_op_addi_8_al()
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


void m68000_base_device::m68k_op_addi_16_d()
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


void m68000_base_device::m68k_op_addi_16_ai()
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


void m68000_base_device::m68k_op_addi_16_pi()
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


void m68000_base_device::m68k_op_addi_16_pd()
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


void m68000_base_device::m68k_op_addi_16_di()
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


void m68000_base_device::m68k_op_addi_16_ix()
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


void m68000_base_device::m68k_op_addi_16_aw()
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


void m68000_base_device::m68k_op_addi_16_al()
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


void m68000_base_device::m68k_op_addi_32_d()
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


void m68000_base_device::m68k_op_addi_32_ai()
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


void m68000_base_device::m68k_op_addi_32_pi()
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


void m68000_base_device::m68k_op_addi_32_pd()
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


void m68000_base_device::m68k_op_addi_32_di()
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


void m68000_base_device::m68k_op_addi_32_ix()
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


void m68000_base_device::m68k_op_addi_32_aw()
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


void m68000_base_device::m68k_op_addi_32_al()
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


void m68000_base_device::m68k_op_addq_8_d()
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


void m68000_base_device::m68k_op_addq_8_ai()
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


void m68000_base_device::m68k_op_addq_8_pi()
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


void m68000_base_device::m68k_op_addq_8_pi7()
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


void m68000_base_device::m68k_op_addq_8_pd()
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


void m68000_base_device::m68k_op_addq_8_pd7()
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


void m68000_base_device::m68k_op_addq_8_di()
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


void m68000_base_device::m68k_op_addq_8_ix()
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


void m68000_base_device::m68k_op_addq_8_aw()
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


void m68000_base_device::m68k_op_addq_8_al()
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


void m68000_base_device::m68k_op_addq_16_d()
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


void m68000_base_device::m68k_op_addq_16_a()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((m_ir >> 9) - 1) & 7) + 1);
}


void m68000_base_device::m68k_op_addq_16_ai()
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


void m68000_base_device::m68k_op_addq_16_pi()
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


void m68000_base_device::m68k_op_addq_16_pd()
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


void m68000_base_device::m68k_op_addq_16_di()
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


void m68000_base_device::m68k_op_addq_16_ix()
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


void m68000_base_device::m68k_op_addq_16_aw()
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


void m68000_base_device::m68k_op_addq_16_al()
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


void m68000_base_device::m68k_op_addq_32_d()
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


void m68000_base_device::m68k_op_addq_32_a()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((m_ir >> 9) - 1) & 7) + 1);
}


void m68000_base_device::m68k_op_addq_32_ai()
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


void m68000_base_device::m68k_op_addq_32_pi()
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


void m68000_base_device::m68k_op_addq_32_pd()
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


void m68000_base_device::m68k_op_addq_32_di()
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


void m68000_base_device::m68k_op_addq_32_ix()
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


void m68000_base_device::m68k_op_addq_32_aw()
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


void m68000_base_device::m68k_op_addq_32_al()
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


void m68000_base_device::m68k_op_addx_8_rr()
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


void m68000_base_device::m68k_op_addx_16_rr()
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


void m68000_base_device::m68k_op_addx_32_rr()
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


void m68000_base_device::m68k_op_addx_8_mm_ax7()
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


void m68000_base_device::m68k_op_addx_8_mm_ay7()
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


void m68000_base_device::m68k_op_addx_8_mm_axy7()
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


void m68000_base_device::m68k_op_addx_8_mm()
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


void m68000_base_device::m68k_op_addx_16_mm()
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


void m68000_base_device::m68k_op_addx_32_mm()
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


void m68000_base_device::m68k_op_and_8_er_d()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (DY() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_ai()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_AI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_pi()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_PI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_pi7()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_A7_PI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_pd()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_PD_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_pd7()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_A7_PD_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_di()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_DI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_ix()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AY_IX_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_aw()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AW_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_al()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_AL_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_pcdi()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_PCDI_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_pcix()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_PCIX_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_er_i()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DX() &= (OPER_I_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_d()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (DY() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_ai()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_AI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_pi()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_PI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_pd()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_PD_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_di()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_DI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_ix()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AY_IX_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_aw()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AW_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_al()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_AL_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_pcdi()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_PCDI_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_pcix()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_PCIX_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_16_er_i()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DX() &= (OPER_I_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_d()
{
	m_not_z_flag = DX() &= DY();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_ai()
{
	m_not_z_flag = DX() &= OPER_AY_AI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_pi()
{
	m_not_z_flag = DX() &= OPER_AY_PI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_pd()
{
	m_not_z_flag = DX() &= OPER_AY_PD_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_di()
{
	m_not_z_flag = DX() &= OPER_AY_DI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_ix()
{
	m_not_z_flag = DX() &= OPER_AY_IX_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_aw()
{
	m_not_z_flag = DX() &= OPER_AW_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_al()
{
	m_not_z_flag = DX() &= OPER_AL_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_pcdi()
{
	m_not_z_flag = DX() &= OPER_PCDI_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_pcix()
{
	m_not_z_flag = DX() &= OPER_PCIX_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_32_er_i()
{
	m_not_z_flag = DX() &= OPER_I_32();

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_and_8_re_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_8_re_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_8_re_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_8_re_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_8_re_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_8_re_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_8_re_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_8_re_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_8_re_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = DX() & m68ki_read_8(ea);

	m_n_flag = NFLAG_8(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_16_re_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_16_re_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_16_re_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_16_re_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_16_re_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_16_re_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_16_re_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = DX() & m68ki_read_16(ea);

	m_n_flag = NFLAG_16(res);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, m_not_z_flag);
}


void m68000_base_device::m68k_op_and_32_re_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68000_base_device::m68k_op_and_32_re_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68000_base_device::m68k_op_and_32_re_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68000_base_device::m68k_op_and_32_re_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68000_base_device::m68k_op_and_32_re_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68000_base_device::m68k_op_and_32_re_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68000_base_device::m68k_op_and_32_re_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() & m68ki_read_32(ea);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68000_base_device::m68k_op_andi_8_d()
{
	m_not_z_flag = MASK_OUT_ABOVE_8(DY() &= (OPER_I_8() | 0xffffff00));

	m_n_flag = NFLAG_8(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_andi_8_ai()
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


void m68000_base_device::m68k_op_andi_8_pi()
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


void m68000_base_device::m68k_op_andi_8_pi7()
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


void m68000_base_device::m68k_op_andi_8_pd()
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


void m68000_base_device::m68k_op_andi_8_pd7()
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


void m68000_base_device::m68k_op_andi_8_di()
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


void m68000_base_device::m68k_op_andi_8_ix()
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


void m68000_base_device::m68k_op_andi_8_aw()
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


void m68000_base_device::m68k_op_andi_8_al()
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


void m68000_base_device::m68k_op_andi_16_d()
{
	m_not_z_flag = MASK_OUT_ABOVE_16(DY() &= (OPER_I_16() | 0xffff0000));

	m_n_flag = NFLAG_16(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_andi_16_ai()
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


void m68000_base_device::m68k_op_andi_16_pi()
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


void m68000_base_device::m68k_op_andi_16_pd()
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


void m68000_base_device::m68k_op_andi_16_di()
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


void m68000_base_device::m68k_op_andi_16_ix()
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


void m68000_base_device::m68k_op_andi_16_aw()
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


void m68000_base_device::m68k_op_andi_16_al()
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


void m68000_base_device::m68k_op_andi_32_d()
{
	m_not_z_flag = DY() &= (OPER_I_32());

	m_n_flag = NFLAG_32(m_not_z_flag);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_andi_32_ai()
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


void m68000_base_device::m68k_op_andi_32_pi()
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


void m68000_base_device::m68k_op_andi_32_pd()
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


void m68000_base_device::m68k_op_andi_32_di()
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


void m68000_base_device::m68k_op_andi_32_ix()
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


void m68000_base_device::m68k_op_andi_32_aw()
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


void m68000_base_device::m68k_op_andi_32_al()
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


void m68000_base_device::m68k_op_andi_8_toc()
{
	m68ki_set_ccr(m68ki_get_ccr() & OPER_I_8());
}


void m68000_base_device::m68k_op_andi_16_tos()
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


void m68000_base_device::m68k_op_asr_8_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	if(GET_MSB_8(src))
		res |= m68ki_shift_8_table[shift];

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);
}


void m68000_base_device::m68k_op_asr_16_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	if(GET_MSB_16(src))
		res |= m68ki_shift_16_table[shift];

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);
}


void m68000_base_device::m68k_op_asr_32_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	if(GET_MSB_32(src))
		res |= m68ki_shift_32_table[shift];

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_x_flag = m_c_flag = src << (9-shift);
}


void m68000_base_device::m68k_op_asr_8_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_asr_16_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_asr_32_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_asr_16_ai()
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


void m68000_base_device::m68k_op_asr_16_pi()
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


void m68000_base_device::m68k_op_asr_16_pd()
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


void m68000_base_device::m68k_op_asr_16_di()
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


void m68000_base_device::m68k_op_asr_16_ix()
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


void m68000_base_device::m68k_op_asr_16_aw()
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


void m68000_base_device::m68k_op_asr_16_al()
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


void m68000_base_device::m68k_op_asl_8_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_x_flag = m_c_flag = src << shift;
	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	src &= m68ki_shift_8_table[shift + 1];
	m_v_flag = (!(src == 0 || (src == m68ki_shift_8_table[shift + 1] && shift < 8)))<<7;
}


void m68000_base_device::m68k_op_asl_16_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (8-shift);
	src &= m68ki_shift_16_table[shift + 1];
	m_v_flag = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
}


void m68000_base_device::m68k_op_asl_32_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (24-shift);
	src &= m68ki_shift_32_table[shift + 1];
	m_v_flag = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
}


void m68000_base_device::m68k_op_asl_8_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_asl_16_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_asl_32_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_asl_16_ai()
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


void m68000_base_device::m68k_op_asl_16_pi()
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


void m68000_base_device::m68k_op_asl_16_pd()
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


void m68000_base_device::m68k_op_asl_16_di()
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


void m68000_base_device::m68k_op_asl_16_ix()
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


void m68000_base_device::m68k_op_asl_16_aw()
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


void m68000_base_device::m68k_op_asl_16_al()
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


void m68000_base_device::m68k_op_bhi_8()
{
	if(COND_HI())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bls_8()
{
	if(COND_LS())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bcc_8()
{
	if(COND_CC())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bcs_8()
{
	if(COND_CS())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bne_8()
{
	if(COND_NE())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_beq_8()
{
	if(COND_EQ())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bvc_8()
{
	if(COND_VC())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bvs_8()
{
	if(COND_VS())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bpl_8()
{
	if(COND_PL())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bmi_8()
{
	if(COND_MI())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bge_8()
{
	if(COND_GE())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_blt_8()
{
	if(COND_LT())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bgt_8()
{
	if(COND_GT())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_ble_8()
{
	if(COND_LE())
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		return;
	}
	m_remaining_cycles -= m_cyc_bcc_notake_b;
}


void m68000_base_device::m68k_op_bhi_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bls_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bcc_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bcs_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bne_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_beq_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bvc_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bvs_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bpl_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bmi_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bge_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_blt_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bgt_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_ble_16()
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
	m_remaining_cycles -= m_cyc_bcc_notake_w;
}


void m68000_base_device::m68k_op_bhi_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bls_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bcc_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bcs_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bne_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_beq_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bvc_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bvs_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bpl_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bmi_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bge_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_blt_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bgt_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_ble_32()
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
		m_remaining_cycles -= m_cyc_bcc_notake_b;
	}
}


void m68000_base_device::m68k_op_bchg_32_r_d()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst ^= mask;
}


void m68000_base_device::m68k_op_bchg_8_r_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_r_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_r_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_r_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_r_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_r_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_r_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_r_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_r_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_32_s_d()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst ^= mask;
}


void m68000_base_device::m68k_op_bchg_8_s_ai()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_s_pi()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_s_pi7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_s_pd()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_s_pd7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_s_di()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_s_ix()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_s_aw()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bchg_8_s_al()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68000_base_device::m68k_op_bclr_32_r_d()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst &= ~mask;
}


void m68000_base_device::m68k_op_bclr_8_r_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_r_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_r_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_r_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_r_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_r_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_r_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_r_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_r_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_32_s_d()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst &= ~mask;
}


void m68000_base_device::m68k_op_bclr_8_s_ai()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_s_pi()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_s_pi7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_s_pd()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_s_pd7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_s_di()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_s_ix()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_s_aw()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bclr_8_s_al()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68000_base_device::m68k_op_bfchg_32_d()
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


void m68000_base_device::m68k_op_bfchg_32_ai()
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


void m68000_base_device::m68k_op_bfchg_32_di()
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


void m68000_base_device::m68k_op_bfchg_32_ix()
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


void m68000_base_device::m68k_op_bfchg_32_aw()
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


void m68000_base_device::m68k_op_bfchg_32_al()
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


void m68000_base_device::m68k_op_bfclr_32_d()
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


void m68000_base_device::m68k_op_bfclr_32_ai()
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


void m68000_base_device::m68k_op_bfclr_32_di()
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


void m68000_base_device::m68k_op_bfclr_32_ix()
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


void m68000_base_device::m68k_op_bfclr_32_aw()
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


void m68000_base_device::m68k_op_bfclr_32_al()
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


void m68000_base_device::m68k_op_bfexts_32_d()
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


void m68000_base_device::m68k_op_bfexts_32_ai()
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


void m68000_base_device::m68k_op_bfexts_32_di()
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


void m68000_base_device::m68k_op_bfexts_32_ix()
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


void m68000_base_device::m68k_op_bfexts_32_aw()
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


void m68000_base_device::m68k_op_bfexts_32_al()
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


void m68000_base_device::m68k_op_bfexts_32_pcdi()
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


void m68000_base_device::m68k_op_bfexts_32_pcix()
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


void m68000_base_device::m68k_op_bfextu_32_d()
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


void m68000_base_device::m68k_op_bfextu_32_ai()
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


void m68000_base_device::m68k_op_bfextu_32_di()
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


void m68000_base_device::m68k_op_bfextu_32_ix()
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


void m68000_base_device::m68k_op_bfextu_32_aw()
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


void m68000_base_device::m68k_op_bfextu_32_al()
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


void m68000_base_device::m68k_op_bfextu_32_pcdi()
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


void m68000_base_device::m68k_op_bfextu_32_pcix()
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


void m68000_base_device::m68k_op_bfffo_32_d()
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


void m68000_base_device::m68k_op_bfffo_32_ai()
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


void m68000_base_device::m68k_op_bfffo_32_di()
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


void m68000_base_device::m68k_op_bfffo_32_ix()
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


void m68000_base_device::m68k_op_bfffo_32_aw()
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


void m68000_base_device::m68k_op_bfffo_32_al()
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


void m68000_base_device::m68k_op_bfffo_32_pcdi()
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


void m68000_base_device::m68k_op_bfffo_32_pcix()
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


void m68000_base_device::m68k_op_bfins_32_d()
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


void m68000_base_device::m68k_op_bfins_32_ai()
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


void m68000_base_device::m68k_op_bfins_32_di()
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


void m68000_base_device::m68k_op_bfins_32_ix()
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


void m68000_base_device::m68k_op_bfins_32_aw()
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


void m68000_base_device::m68k_op_bfins_32_al()
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


void m68000_base_device::m68k_op_bfset_32_d()
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


void m68000_base_device::m68k_op_bfset_32_ai()
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


void m68000_base_device::m68k_op_bfset_32_di()
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


void m68000_base_device::m68k_op_bfset_32_ix()
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


void m68000_base_device::m68k_op_bfset_32_aw()
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


void m68000_base_device::m68k_op_bfset_32_al()
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


void m68000_base_device::m68k_op_bftst_32_d()
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


void m68000_base_device::m68k_op_bftst_32_ai()
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


void m68000_base_device::m68k_op_bftst_32_di()
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


void m68000_base_device::m68k_op_bftst_32_ix()
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


void m68000_base_device::m68k_op_bftst_32_aw()
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


void m68000_base_device::m68k_op_bftst_32_al()
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


void m68000_base_device::m68k_op_bftst_32_pcdi()
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


void m68000_base_device::m68k_op_bftst_32_pcix()
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


void m68000_base_device::m68k_op_bkpt()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		if (!m_bkpt_ack_callback.isnull())
			(m_bkpt_ack_callback)(*m_program, 0, CPU_TYPE_IS_EC020_PLUS() ? m_ir & 7 : 0, 0xffffffff);
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_bra_8()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_bra_16()
{
	uint32_t offset = OPER_I_16();
	m_pc -= 2;
	m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
	m68ki_branch_16(offset);
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_bra_32()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		uint32_t offset = OPER_I_32();
		m_pc -= 4;
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m68ki_branch_32(offset);
		if(m_pc == m_ppc && m_remaining_cycles > 0)
			m_remaining_cycles = 0;
		return;
	}
	else
	{
		m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
		if(m_pc == m_ppc && m_remaining_cycles > 0)
			m_remaining_cycles = 0;
	}
}


void m68000_base_device::m68k_op_bset_32_r_d()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (DX() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst |= mask;
}


void m68000_base_device::m68k_op_bset_8_r_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_r_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_r_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_r_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_r_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_r_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_r_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_r_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_r_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);
	uint32_t mask = 1 << (DX() & 7);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_32_s_d()
{
	uint32_t* r_dst = &DY();
	uint32_t mask = 1 << (OPER_I_8() & 0x1f);

	m_not_z_flag = *r_dst & mask;
	*r_dst |= mask;
}


void m68000_base_device::m68k_op_bset_8_s_ai()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_AI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_s_pi()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_s_pi7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_s_pd()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_s_pd7()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_A7_PD_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_s_di()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_DI_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_s_ix()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AY_IX_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_s_aw()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AW_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bset_8_s_al()
{
	uint32_t mask = 1 << (OPER_I_8() & 7);
	uint32_t ea = EA_AL_8();
	uint32_t src = m68ki_read_8(ea);

	m_not_z_flag = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68000_base_device::m68k_op_bsr_8()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_branch_8(MASK_OUT_ABOVE_8(m_ir));
}


void m68000_base_device::m68k_op_bsr_16()
{
	uint32_t offset = OPER_I_16();
	m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m_pc -= 2;
	m68ki_branch_16(offset);
}


void m68000_base_device::m68k_op_bsr_32()
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


void m68000_base_device::m68k_op_btst_32_r_d()
{
	m_not_z_flag = DY() & (1 << (DX() & 0x1f));
}


void m68000_base_device::m68k_op_btst_8_r_ai()
{
	m_not_z_flag = OPER_AY_AI_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_pi()
{
	m_not_z_flag = OPER_AY_PI_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_pi7()
{
	m_not_z_flag = OPER_A7_PI_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_pd()
{
	m_not_z_flag = OPER_AY_PD_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_pd7()
{
	m_not_z_flag = OPER_A7_PD_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_di()
{
	m_not_z_flag = OPER_AY_DI_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_ix()
{
	m_not_z_flag = OPER_AY_IX_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_aw()
{
	m_not_z_flag = OPER_AW_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_al()
{
	m_not_z_flag = OPER_AL_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_pcdi()
{
	m_not_z_flag = OPER_PCDI_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_pcix()
{
	m_not_z_flag = OPER_PCIX_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_8_r_i()
{
	m_not_z_flag = OPER_I_8() & (1 << (DX() & 7));
}


void m68000_base_device::m68k_op_btst_32_s_d()
{
	m_not_z_flag = DY() & (1 << (OPER_I_8() & 0x1f));
}


void m68000_base_device::m68k_op_btst_8_s_ai()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_AI_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_pi()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_PI_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_pi7()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_A7_PI_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_pd()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_PD_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_pd7()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_A7_PD_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_di()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_DI_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_ix()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AY_IX_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_aw()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AW_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_al()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_AL_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_pcdi()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_PCDI_8() & (1 << bit);
}


void m68000_base_device::m68k_op_btst_8_s_pcix()
{
	uint32_t bit = OPER_I_8() & 7;

	m_not_z_flag = OPER_PCIX_8() & (1 << bit);
}


void m68000_base_device::m68k_op_callm_32_ai()
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


void m68000_base_device::m68k_op_callm_32_di()
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


void m68000_base_device::m68k_op_callm_32_ix()
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


void m68000_base_device::m68k_op_callm_32_aw()
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


void m68000_base_device::m68k_op_callm_32_al()
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


void m68000_base_device::m68k_op_callm_32_pcdi()
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


void m68000_base_device::m68k_op_callm_32_pcix()
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


void m68000_base_device::m68k_op_cas_8_ai()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_8_pi()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_8_pi7()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_8_pd()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_8_pd7()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_8_di()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_8_ix()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_8_aw()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_8_al()
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
			m_remaining_cycles -= 3;
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_16_ai()
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
			m_remaining_cycles -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_16_pi()
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
			m_remaining_cycles -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_16_pd()
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
			m_remaining_cycles -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_16_di()
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
			m_remaining_cycles -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_16_ix()
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
			m_remaining_cycles -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_16_aw()
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
			m_remaining_cycles -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_16_al()
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
			m_remaining_cycles -= 3;
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D()[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_32_ai()
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
			m_remaining_cycles -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_32_pi()
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
			m_remaining_cycles -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_32_pd()
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
			m_remaining_cycles -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_32_di()
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
			m_remaining_cycles -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_32_ix()
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
			m_remaining_cycles -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_32_aw()
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
			m_remaining_cycles -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas_32_al()
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
			m_remaining_cycles -= 3;
			m68ki_write_32(ea, REG_D()[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_cas2_16()
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
				m_remaining_cycles -= 3;
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


void m68000_base_device::m68k_op_cas2_32()
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
				m_remaining_cycles -= 3;
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


void m68000_base_device::m68k_op_chk_16_d()
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


void m68000_base_device::m68k_op_chk_16_ai()
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


void m68000_base_device::m68k_op_chk_16_pi()
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


void m68000_base_device::m68k_op_chk_16_pd()
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


void m68000_base_device::m68k_op_chk_16_di()
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


void m68000_base_device::m68k_op_chk_16_ix()
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


void m68000_base_device::m68k_op_chk_16_aw()
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


void m68000_base_device::m68k_op_chk_16_al()
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


void m68000_base_device::m68k_op_chk_16_pcdi()
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


void m68000_base_device::m68k_op_chk_16_pcix()
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


void m68000_base_device::m68k_op_chk_16_i()
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


void m68000_base_device::m68k_op_chk_32_d()
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


void m68000_base_device::m68k_op_chk_32_ai()
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


void m68000_base_device::m68k_op_chk_32_pi()
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


void m68000_base_device::m68k_op_chk_32_pd()
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


void m68000_base_device::m68k_op_chk_32_di()
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


void m68000_base_device::m68k_op_chk_32_ix()
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


void m68000_base_device::m68k_op_chk_32_aw()
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


void m68000_base_device::m68k_op_chk_32_al()
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


void m68000_base_device::m68k_op_chk_32_pcdi()
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


void m68000_base_device::m68k_op_chk_32_pcix()
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


void m68000_base_device::m68k_op_chk_32_i()
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


void m68000_base_device::m68k_op_chk2cmp2_8_pcdi()
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


void m68000_base_device::m68k_op_chk2cmp2_8_pcix()
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


void m68000_base_device::m68k_op_chk2cmp2_8_ai()
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


void m68000_base_device::m68k_op_chk2cmp2_8_di()
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


void m68000_base_device::m68k_op_chk2cmp2_8_ix()
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


void m68000_base_device::m68k_op_chk2cmp2_8_aw()
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


void m68000_base_device::m68k_op_chk2cmp2_8_al()
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


void m68000_base_device::m68k_op_chk2cmp2_16_pcdi()
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


void m68000_base_device::m68k_op_chk2cmp2_16_pcix()
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


void m68000_base_device::m68k_op_chk2cmp2_16_ai()
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


void m68000_base_device::m68k_op_chk2cmp2_16_di()
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


void m68000_base_device::m68k_op_chk2cmp2_16_ix()
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


void m68000_base_device::m68k_op_chk2cmp2_16_aw()
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


void m68000_base_device::m68k_op_chk2cmp2_16_al()
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


void m68000_base_device::m68k_op_chk2cmp2_32_pcdi()
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


void m68000_base_device::m68k_op_chk2cmp2_32_pcix()
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


void m68000_base_device::m68k_op_chk2cmp2_32_ai()
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


void m68000_base_device::m68k_op_chk2cmp2_32_di()
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


void m68000_base_device::m68k_op_chk2cmp2_32_ix()
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


void m68000_base_device::m68k_op_chk2cmp2_32_aw()
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


void m68000_base_device::m68k_op_chk2cmp2_32_al()
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


void m68000_base_device::m68k_op_clr_8_d()
{
	DY() &= 0xffffff00;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;
}


void m68000_base_device::m68k_op_clr_8_ai()
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


void m68000_base_device::m68k_op_clr_8_pi()
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


void m68000_base_device::m68k_op_clr_8_pi7()
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


void m68000_base_device::m68k_op_clr_8_pd()
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


void m68000_base_device::m68k_op_clr_8_pd7()
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


void m68000_base_device::m68k_op_clr_8_di()
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


void m68000_base_device::m68k_op_clr_8_ix()
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


void m68000_base_device::m68k_op_clr_8_aw()
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


void m68000_base_device::m68k_op_clr_8_al()
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


void m68000_base_device::m68k_op_clr_16_d()
{
	DY() &= 0xffff0000;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;
}


void m68000_base_device::m68k_op_clr_16_ai()
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


void m68000_base_device::m68k_op_clr_16_pi()
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


void m68000_base_device::m68k_op_clr_16_pd()
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


void m68000_base_device::m68k_op_clr_16_di()
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


void m68000_base_device::m68k_op_clr_16_ix()
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


void m68000_base_device::m68k_op_clr_16_aw()
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


void m68000_base_device::m68k_op_clr_16_al()
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


void m68000_base_device::m68k_op_clr_32_d()
{
	DY() = 0;

	m_n_flag = NFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	m_not_z_flag = ZFLAG_SET;
}


void m68000_base_device::m68k_op_clr_32_ai()
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


void m68000_base_device::m68k_op_clr_32_pi()
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


void m68000_base_device::m68k_op_clr_32_pd()
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


void m68000_base_device::m68k_op_clr_32_di()
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


void m68000_base_device::m68k_op_clr_32_ix()
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


void m68000_base_device::m68k_op_clr_32_aw()
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


void m68000_base_device::m68k_op_clr_32_al()
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


void m68000_base_device::m68k_op_cmp_8_d()
{
	uint32_t src = MASK_OUT_ABOVE_8(DY());
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_ai()
{
	uint32_t src = OPER_AY_AI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_pi()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_pi7()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_pd()
{
	uint32_t src = OPER_AY_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_pd7()
{
	uint32_t src = OPER_A7_PD_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_di()
{
	uint32_t src = OPER_AY_DI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_ix()
{
	uint32_t src = OPER_AY_IX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_aw()
{
	uint32_t src = OPER_AW_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_al()
{
	uint32_t src = OPER_AL_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_pcdi()
{
	uint32_t src = OPER_PCDI_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_pcix()
{
	uint32_t src = OPER_PCIX_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_8_i()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmp_16_d()
{
	uint32_t src = MASK_OUT_ABOVE_16(DY());
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_a()
{
	uint32_t src = MASK_OUT_ABOVE_16(AY());
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_ai()
{
	uint32_t src = OPER_AY_AI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_pi()
{
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_pd()
{
	uint32_t src = OPER_AY_PD_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_di()
{
	uint32_t src = OPER_AY_DI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_ix()
{
	uint32_t src = OPER_AY_IX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_aw()
{
	uint32_t src = OPER_AW_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_al()
{
	uint32_t src = OPER_AL_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_pcdi()
{
	uint32_t src = OPER_PCDI_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_pcix()
{
	uint32_t src = OPER_PCIX_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_16_i()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DX());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmp_32_d()
{
	uint32_t src = DY();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_a()
{
	uint32_t src = AY();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_ai()
{
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_pi()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_pd()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_di()
{
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_ix()
{
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_aw()
{
	uint32_t src = OPER_AW_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_al()
{
	uint32_t src = OPER_AL_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_pcdi()
{
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_pcix()
{
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmp_32_i()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = DX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_d()
{
	uint32_t src = MAKE_INT_16(DY());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_a()
{
	uint32_t src = MAKE_INT_16(AY());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_ai()
{
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_pi()
{
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_pd()
{
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_di()
{
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_ix()
{
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_aw()
{
	uint32_t src = MAKE_INT_16(OPER_AW_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_al()
{
	uint32_t src = MAKE_INT_16(OPER_AL_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_pcdi()
{
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_pcix()
{
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_16_i()
{
	uint32_t src = MAKE_INT_16(OPER_I_16());
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_d()
{
	uint32_t src = DY();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_a()
{
	uint32_t src = AY();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_ai()
{
	uint32_t src = OPER_AY_AI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_pi()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_pd()
{
	uint32_t src = OPER_AY_PD_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_di()
{
	uint32_t src = OPER_AY_DI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_ix()
{
	uint32_t src = OPER_AY_IX_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_aw()
{
	uint32_t src = OPER_AW_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_al()
{
	uint32_t src = OPER_AL_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_pcdi()
{
	uint32_t src = OPER_PCDI_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_pcix()
{
	uint32_t src = OPER_PCIX_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpa_32_i()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = AX();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpi_8_d()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = MASK_OUT_ABOVE_8(DY());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_ai()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_AI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_pi()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_pi7()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_pd()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_PD_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_pd7()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_A7_PD_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_di()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_DI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_ix()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AY_IX_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_aw()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AW_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_al()
{
	uint32_t src = OPER_I_8();
	uint32_t dst = OPER_AL_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpi_8_pcdi()
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


void m68000_base_device::m68k_op_cmpi_8_pcix()
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


void m68000_base_device::m68k_op_cmpi_16_d()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = MASK_OUT_ABOVE_16(DY());
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpi_16_ai()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_AI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpi_16_pi()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_PI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpi_16_pd()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_PD_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpi_16_di()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_DI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpi_16_ix()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AY_IX_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpi_16_aw()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AW_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpi_16_al()
{
	uint32_t src = OPER_I_16();
	uint32_t dst = OPER_AL_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpi_16_pcdi()
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


void m68000_base_device::m68k_op_cmpi_16_pcix()
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


void m68000_base_device::m68k_op_cmpi_32_d()
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


void m68000_base_device::m68k_op_cmpi_32_ai()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_AI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpi_32_pi()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_PI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpi_32_pd()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_PD_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpi_32_di()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_DI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpi_32_ix()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AY_IX_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpi_32_aw()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AW_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpi_32_al()
{
	uint32_t src = OPER_I_32();
	uint32_t dst = OPER_AL_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cmpi_32_pcdi()
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


void m68000_base_device::m68k_op_cmpi_32_pcix()
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


void m68000_base_device::m68k_op_cmpm_8_ax7()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpm_8_ay7()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = OPER_AX_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpm_8_axy7()
{
	uint32_t src = OPER_A7_PI_8();
	uint32_t dst = OPER_A7_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpm_8()
{
	uint32_t src = OPER_AY_PI_8();
	uint32_t dst = OPER_AX_PI_8();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = MASK_OUT_ABOVE_8(res);
	m_v_flag = VFLAG_SUB_8(src, dst, res);
	m_c_flag = CFLAG_8(res);
}


void m68000_base_device::m68k_op_cmpm_16()
{
	uint32_t src = OPER_AY_PI_16();
	uint32_t dst = OPER_AX_PI_16();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = MASK_OUT_ABOVE_16(res);
	m_v_flag = VFLAG_SUB_16(src, dst, res);
	m_c_flag = CFLAG_16(res);
}


void m68000_base_device::m68k_op_cmpm_32()
{
	uint32_t src = OPER_AY_PI_32();
	uint32_t dst = OPER_AX_PI_32();
	uint32_t res = dst - src;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = MASK_OUT_ABOVE_32(res);
	m_v_flag = VFLAG_SUB_32(src, dst, res);
	m_c_flag = CFLAG_SUB_32(src, dst, res);
}


void m68000_base_device::m68k_op_cpbcc_32()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpbcc)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_cpdbcc_32()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpdbcc)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_cpgen_32()
{
	if(CPU_TYPE_IS_EC020_PLUS() && (m_has_fpu || m_has_pmmu))
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpgen)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_cpscc_32()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpscc)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_cptrapcc_32()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cptrapcc)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_ftrapcc_32()
{
	if(m_has_fpu)
	{
		m68881_ftrap();
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_dbt_16()
{
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbf_16()
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
		m_remaining_cycles -= m_cyc_dbcc_f_noexp;
		return;
	}
	m_pc += 2;
	m_remaining_cycles -= m_cyc_dbcc_f_exp;
}


void m68000_base_device::m68k_op_dbhi_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbls_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbcc_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbcs_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbne_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbeq_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbvc_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbvs_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbpl_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbmi_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbge_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dblt_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dbgt_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_dble_16()
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
			m_remaining_cycles -= m_cyc_dbcc_f_noexp;
			return;
		}
		m_pc += 2;
		m_remaining_cycles -= m_cyc_dbcc_f_exp;
		return;
	}
	m_pc += 2;
}


void m68000_base_device::m68k_op_divs_16_d()
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


void m68000_base_device::m68k_op_divs_16_ai()
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


void m68000_base_device::m68k_op_divs_16_pi()
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


void m68000_base_device::m68k_op_divs_16_pd()
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


void m68000_base_device::m68k_op_divs_16_di()
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


void m68000_base_device::m68k_op_divs_16_ix()
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


void m68000_base_device::m68k_op_divs_16_aw()
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


void m68000_base_device::m68k_op_divs_16_al()
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


void m68000_base_device::m68k_op_divs_16_pcdi()
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


void m68000_base_device::m68k_op_divs_16_pcix()
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


void m68000_base_device::m68k_op_divs_16_i()
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


void m68000_base_device::m68k_op_divu_16_d()
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


void m68000_base_device::m68k_op_divu_16_ai()
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


void m68000_base_device::m68k_op_divu_16_pi()
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


void m68000_base_device::m68k_op_divu_16_pd()
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


void m68000_base_device::m68k_op_divu_16_di()
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


void m68000_base_device::m68k_op_divu_16_ix()
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


void m68000_base_device::m68k_op_divu_16_aw()
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


void m68000_base_device::m68k_op_divu_16_al()
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


void m68000_base_device::m68k_op_divu_16_pcdi()
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


void m68000_base_device::m68k_op_divu_16_pcix()
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


void m68000_base_device::m68k_op_divu_16_i()
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


void m68000_base_device::m68k_op_divl_32_d()
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


void m68000_base_device::m68k_op_divl_32_ai()
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


void m68000_base_device::m68k_op_divl_32_pi()
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


void m68000_base_device::m68k_op_divl_32_pd()
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


void m68000_base_device::m68k_op_divl_32_di()
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


void m68000_base_device::m68k_op_divl_32_ix()
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


void m68000_base_device::m68k_op_divl_32_aw()
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


void m68000_base_device::m68k_op_divl_32_al()
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


void m68000_base_device::m68k_op_divl_32_pcdi()
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


void m68000_base_device::m68k_op_divl_32_pcix()
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


void m68000_base_device::m68k_op_divl_32_i()
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


void m68000_base_device::m68k_op_eor_8_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY() ^= MASK_OUT_ABOVE_8(DX()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_8_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() ^ m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_16_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() ^= MASK_OUT_ABOVE_16(DX()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_16_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_16_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_16_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_16_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_16_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_16_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_16_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() ^ m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_32_d()
{
	uint32_t res = DY() ^= DX();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_32_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_32_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_32_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_32_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_32_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_32_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eor_32_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() ^ m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eori_8_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY() ^= OPER_I_8());

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eori_8_ai()
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


void m68000_base_device::m68k_op_eori_8_pi()
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


void m68000_base_device::m68k_op_eori_8_pi7()
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


void m68000_base_device::m68k_op_eori_8_pd()
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


void m68000_base_device::m68k_op_eori_8_pd7()
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


void m68000_base_device::m68k_op_eori_8_di()
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


void m68000_base_device::m68k_op_eori_8_ix()
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


void m68000_base_device::m68k_op_eori_8_aw()
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


void m68000_base_device::m68k_op_eori_8_al()
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


void m68000_base_device::m68k_op_eori_16_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() ^= OPER_I_16());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eori_16_ai()
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


void m68000_base_device::m68k_op_eori_16_pi()
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


void m68000_base_device::m68k_op_eori_16_pd()
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


void m68000_base_device::m68k_op_eori_16_di()
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


void m68000_base_device::m68k_op_eori_16_ix()
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


void m68000_base_device::m68k_op_eori_16_aw()
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


void m68000_base_device::m68k_op_eori_16_al()
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


void m68000_base_device::m68k_op_eori_32_d()
{
	uint32_t res = DY() ^= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_eori_32_ai()
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


void m68000_base_device::m68k_op_eori_32_pi()
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


void m68000_base_device::m68k_op_eori_32_pd()
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


void m68000_base_device::m68k_op_eori_32_di()
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


void m68000_base_device::m68k_op_eori_32_ix()
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


void m68000_base_device::m68k_op_eori_32_aw()
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


void m68000_base_device::m68k_op_eori_32_al()
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


void m68000_base_device::m68k_op_eori_8_toc()
{
	m68ki_set_ccr(m68ki_get_ccr() ^ OPER_I_8());
}


void m68000_base_device::m68k_op_eori_16_tos()
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


void m68000_base_device::m68k_op_exg_32_dd()
{
	uint32_t* reg_a = &DX();
	uint32_t* reg_b = &DY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


void m68000_base_device::m68k_op_exg_32_aa()
{
	uint32_t* reg_a = &AX();
	uint32_t* reg_b = &AY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


void m68000_base_device::m68k_op_exg_32_da()
{
	uint32_t* reg_a = &DX();
	uint32_t* reg_b = &AY();
	uint32_t tmp = *reg_a;
	*reg_a = *reg_b;
	*reg_b = tmp;
}


void m68000_base_device::m68k_op_ext_16()
{
	uint32_t* r_dst = &DY();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | MASK_OUT_ABOVE_8(*r_dst) | (GET_MSB_8(*r_dst) ? 0xff00 : 0);

	m_n_flag = NFLAG_16(*r_dst);
	m_not_z_flag = MASK_OUT_ABOVE_16(*r_dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_ext_32()
{
	uint32_t* r_dst = &DY();

	*r_dst = MASK_OUT_ABOVE_16(*r_dst) | (GET_MSB_16(*r_dst) ? 0xffff0000 : 0);

	m_n_flag = NFLAG_32(*r_dst);
	m_not_z_flag = *r_dst;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_extb_32()
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


void m68000_base_device::m68k_op_illegal()
{
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_jmp_32_ai()
{
	m68ki_jump(EA_AY_AI_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_jmp_32_di()
{
	m68ki_jump(EA_AY_DI_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_jmp_32_ix()
{
	m68ki_jump(EA_AY_IX_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_jmp_32_aw()
{
	m68ki_jump(EA_AW_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_jmp_32_al()
{
	m68ki_jump(EA_AL_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_jmp_32_pcdi()
{
	m68ki_jump(EA_PCDI_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_jmp_32_pcix()
{
	m68ki_jump(EA_PCIX_32());
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	if(m_pc == m_ppc && m_remaining_cycles > 0)
		m_remaining_cycles = 0;
}


void m68000_base_device::m68k_op_jsr_32_ai()
{
	uint32_t ea = EA_AY_AI_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);
}


void m68000_base_device::m68k_op_jsr_32_di()
{
	uint32_t ea = EA_AY_DI_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);
}


void m68000_base_device::m68k_op_jsr_32_ix()
{
	uint32_t ea = EA_AY_IX_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);
}


void m68000_base_device::m68k_op_jsr_32_aw()
{
	uint32_t ea = EA_AW_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);
}


void m68000_base_device::m68k_op_jsr_32_al()
{
	uint32_t ea = EA_AL_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);
}


void m68000_base_device::m68k_op_jsr_32_pcdi()
{
	uint32_t ea = EA_PCDI_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);
}


void m68000_base_device::m68k_op_jsr_32_pcix()
{
	uint32_t ea = EA_PCIX_32();
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(m_pc);
	m68ki_jump(ea);
}


void m68000_base_device::m68k_op_lea_32_ai()
{
	AX() = EA_AY_AI_32();
}


void m68000_base_device::m68k_op_lea_32_di()
{
	AX() = EA_AY_DI_32();
}


void m68000_base_device::m68k_op_lea_32_ix()
{
	AX() = EA_AY_IX_32();
}


void m68000_base_device::m68k_op_lea_32_aw()
{
	AX() = EA_AW_32();
}


void m68000_base_device::m68k_op_lea_32_al()
{
	AX() = EA_AL_32();
}


void m68000_base_device::m68k_op_lea_32_pcdi()
{
	AX() = EA_PCDI_32();
}


void m68000_base_device::m68k_op_lea_32_pcix()
{
	AX() = EA_PCIX_32();
}


void m68000_base_device::m68k_op_link_16_a7()
{
	REG_A()[7] -= 4;
	m68ki_write_32(REG_A()[7], REG_A()[7]);
	REG_A()[7] = MASK_OUT_ABOVE_32(REG_A()[7] + MAKE_INT_16(OPER_I_16()));
}


void m68000_base_device::m68k_op_link_16()
{
	uint32_t* r_dst = &AY();

	m68ki_push_32(*r_dst);
	*r_dst = REG_A()[7];
	REG_A()[7] = MASK_OUT_ABOVE_32(REG_A()[7] + MAKE_INT_16(OPER_I_16()));
}


void m68000_base_device::m68k_op_link_32_a7()
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


void m68000_base_device::m68k_op_link_32()
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


void m68000_base_device::m68k_op_lsr_8_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_lsr_16_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_lsr_32_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_CLEAR;
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_lsr_8_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_lsr_16_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_lsr_32_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = src >> shift;

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_lsr_16_ai()
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


void m68000_base_device::m68k_op_lsr_16_pi()
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


void m68000_base_device::m68k_op_lsr_16_pd()
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


void m68000_base_device::m68k_op_lsr_16_di()
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


void m68000_base_device::m68k_op_lsr_16_ix()
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


void m68000_base_device::m68k_op_lsr_16_aw()
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


void m68000_base_device::m68k_op_lsr_16_al()
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


void m68000_base_device::m68k_op_lsl_8_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src << shift;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_lsl_16_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (8-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_lsl_32_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_x_flag = m_c_flag = src >> (24-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_lsl_8_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_lsl_16_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_lsl_32_r()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = DX() & 0x3f;
	uint32_t src = *r_dst;
	uint32_t res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		m_remaining_cycles -= shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_lsl_16_ai()
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


void m68000_base_device::m68k_op_lsl_16_pi()
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


void m68000_base_device::m68k_op_lsl_16_pd()
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


void m68000_base_device::m68k_op_lsl_16_di()
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


void m68000_base_device::m68k_op_lsl_16_ix()
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


void m68000_base_device::m68k_op_lsl_16_aw()
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


void m68000_base_device::m68k_op_lsl_16_al()
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


void m68000_base_device::m68k_op_move_8_d_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_d_i()
{
	uint32_t res = OPER_I_8();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ai_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_AI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi7_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_A7_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pi_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_PI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd7_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_A7_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_pd_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_PD_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_di_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_DI_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_ix_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AX_IX_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_aw_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AW_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_ai()
{
	uint32_t res = OPER_AY_AI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_pi()
{
	uint32_t res = OPER_AY_PI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_pi7()
{
	uint32_t res = OPER_A7_PI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_pd()
{
	uint32_t res = OPER_AY_PD_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_pd7()
{
	uint32_t res = OPER_A7_PD_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_di()
{
	uint32_t res = OPER_AY_DI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_ix()
{
	uint32_t res = OPER_AY_IX_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_aw()
{
	uint32_t res = OPER_AW_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_al()
{
	uint32_t res = OPER_AL_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_pcdi()
{
	uint32_t res = OPER_PCDI_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_pcix()
{
	uint32_t res = OPER_PCIX_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_8_al_i()
{
	uint32_t res = OPER_I_8();
	uint32_t ea = EA_AL_8();

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_a()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_d_i()
{
	uint32_t res = OPER_I_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_a()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ai_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_AI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_a()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pi_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_PI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_a()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_pd_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_PD_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_a()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_di_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_DI_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_a()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_ix_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AX_IX_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_a()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_aw_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AW_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_a()
{
	uint32_t res = MASK_OUT_ABOVE_16(AY());
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_ai()
{
	uint32_t res = OPER_AY_AI_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_pi()
{
	uint32_t res = OPER_AY_PI_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_pd()
{
	uint32_t res = OPER_AY_PD_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_di()
{
	uint32_t res = OPER_AY_DI_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_ix()
{
	uint32_t res = OPER_AY_IX_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_aw()
{
	uint32_t res = OPER_AW_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_al()
{
	uint32_t res = OPER_AL_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_pcdi()
{
	uint32_t res = OPER_PCDI_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_pcix()
{
	uint32_t res = OPER_PCIX_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_16_al_i()
{
	uint32_t res = OPER_I_16();
	uint32_t ea = EA_AL_16();

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_d()
{
	uint32_t res = DY();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_a()
{
	uint32_t res = AY();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_d_i()
{
	uint32_t res = OPER_I_32();
	uint32_t* r_dst = &DX();

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_d()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_a()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ai_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_AI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_d()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_a()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pi_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_PI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_pd_d()
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


void m68000_base_device::m68k_op_move_32_pd_a()
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


void m68000_base_device::m68k_op_move_32_pd_ai()
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


void m68000_base_device::m68k_op_move_32_pd_pi()
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


void m68000_base_device::m68k_op_move_32_pd_pd()
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


void m68000_base_device::m68k_op_move_32_pd_di()
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


void m68000_base_device::m68k_op_move_32_pd_ix()
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


void m68000_base_device::m68k_op_move_32_pd_aw()
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


void m68000_base_device::m68k_op_move_32_pd_al()
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


void m68000_base_device::m68k_op_move_32_pd_pcdi()
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


void m68000_base_device::m68k_op_move_32_pd_pcix()
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


void m68000_base_device::m68k_op_move_32_pd_i()
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


void m68000_base_device::m68k_op_move_32_di_d()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_a()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_di_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_DI_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_d()
{
	uint32_t res = DY();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_a()
{
	uint32_t res = AY();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_ix_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AX_IX_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_d()
{
	uint32_t res = DY();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_a()
{
	uint32_t res = AY();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_aw_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AW_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_d()
{
	uint32_t res = DY();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_a()
{
	uint32_t res = AY();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_ai()
{
	uint32_t res = OPER_AY_AI_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_pi()
{
	uint32_t res = OPER_AY_PI_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_pd()
{
	uint32_t res = OPER_AY_PD_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_di()
{
	uint32_t res = OPER_AY_DI_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_ix()
{
	uint32_t res = OPER_AY_IX_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_aw()
{
	uint32_t res = OPER_AW_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_al()
{
	uint32_t res = OPER_AL_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_pcdi()
{
	uint32_t res = OPER_PCDI_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_pcix()
{
	uint32_t res = OPER_PCIX_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move_32_al_i()
{
	uint32_t res = OPER_I_32();
	uint32_t ea = EA_AL_32();

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_movea_16_d()
{
	AX() = MAKE_INT_16(DY());
}


void m68000_base_device::m68k_op_movea_16_a()
{
	AX() = MAKE_INT_16(AY());
}


void m68000_base_device::m68k_op_movea_16_ai()
{
	AX() = MAKE_INT_16(OPER_AY_AI_16());
}


void m68000_base_device::m68k_op_movea_16_pi()
{
	AX() = MAKE_INT_16(OPER_AY_PI_16());
}


void m68000_base_device::m68k_op_movea_16_pd()
{
	AX() = MAKE_INT_16(OPER_AY_PD_16());
}


void m68000_base_device::m68k_op_movea_16_di()
{
	AX() = MAKE_INT_16(OPER_AY_DI_16());
}


void m68000_base_device::m68k_op_movea_16_ix()
{
	AX() = MAKE_INT_16(OPER_AY_IX_16());
}


void m68000_base_device::m68k_op_movea_16_aw()
{
	AX() = MAKE_INT_16(OPER_AW_16());
}


void m68000_base_device::m68k_op_movea_16_al()
{
	AX() = MAKE_INT_16(OPER_AL_16());
}


void m68000_base_device::m68k_op_movea_16_pcdi()
{
	AX() = MAKE_INT_16(OPER_PCDI_16());
}


void m68000_base_device::m68k_op_movea_16_pcix()
{
	AX() = MAKE_INT_16(OPER_PCIX_16());
}


void m68000_base_device::m68k_op_movea_16_i()
{
	AX() = MAKE_INT_16(OPER_I_16());
}


void m68000_base_device::m68k_op_movea_32_d()
{
	AX() = DY();
}


void m68000_base_device::m68k_op_movea_32_a()
{
	AX() = AY();
}


void m68000_base_device::m68k_op_movea_32_ai()
{
	AX() = OPER_AY_AI_32();
}


void m68000_base_device::m68k_op_movea_32_pi()
{
	AX() = OPER_AY_PI_32();
}


void m68000_base_device::m68k_op_movea_32_pd()
{
	AX() = OPER_AY_PD_32();
}


void m68000_base_device::m68k_op_movea_32_di()
{
	AX() = OPER_AY_DI_32();
}


void m68000_base_device::m68k_op_movea_32_ix()
{
	AX() = OPER_AY_IX_32();
}


void m68000_base_device::m68k_op_movea_32_aw()
{
	AX() = OPER_AW_32();
}


void m68000_base_device::m68k_op_movea_32_al()
{
	AX() = OPER_AL_32();
}


void m68000_base_device::m68k_op_movea_32_pcdi()
{
	AX() = OPER_PCDI_32();
}


void m68000_base_device::m68k_op_movea_32_pcix()
{
	AX() = OPER_PCIX_32();
}


void m68000_base_device::m68k_op_movea_32_i()
{
	AX() = OPER_I_32();
}


void m68000_base_device::m68k_op_move_16_frc_d()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		DY() = MASK_OUT_BELOW_16(DY()) | m68ki_get_ccr();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_move_16_frc_ai()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_AI_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_move_16_frc_pi()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_PI_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_move_16_frc_pd()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_PD_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_move_16_frc_di()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_DI_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_move_16_frc_ix()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AY_IX_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_move_16_frc_aw()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AW_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_move_16_frc_al()
{
	if(CPU_TYPE_IS_010_PLUS())
	{
		m68ki_write_16(EA_AL_16(), m68ki_get_ccr());
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_move_16_toc_d()
{
	m68ki_set_ccr(DY());
}


void m68000_base_device::m68k_op_move_16_toc_ai()
{
	m68ki_set_ccr(OPER_AY_AI_16());
}


void m68000_base_device::m68k_op_move_16_toc_pi()
{
	m68ki_set_ccr(OPER_AY_PI_16());
}


void m68000_base_device::m68k_op_move_16_toc_pd()
{
	m68ki_set_ccr(OPER_AY_PD_16());
}


void m68000_base_device::m68k_op_move_16_toc_di()
{
	m68ki_set_ccr(OPER_AY_DI_16());
}


void m68000_base_device::m68k_op_move_16_toc_ix()
{
	m68ki_set_ccr(OPER_AY_IX_16());
}


void m68000_base_device::m68k_op_move_16_toc_aw()
{
	m68ki_set_ccr(OPER_AW_16());
}


void m68000_base_device::m68k_op_move_16_toc_al()
{
	m68ki_set_ccr(OPER_AL_16());
}


void m68000_base_device::m68k_op_move_16_toc_pcdi()
{
	m68ki_set_ccr(OPER_PCDI_16());
}


void m68000_base_device::m68k_op_move_16_toc_pcix()
{
	m68ki_set_ccr(OPER_PCIX_16());
}


void m68000_base_device::m68k_op_move_16_toc_i()
{
	m68ki_set_ccr(OPER_I_16());
}


void m68000_base_device::m68k_op_move_16_frs_d()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		DY() = MASK_OUT_BELOW_16(DY()) | m68ki_get_sr();
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_frs_ai()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_AI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_frs_pi()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_PI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_frs_pd()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_PD_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_frs_di()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_DI_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_frs_ix()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AY_IX_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_frs_aw()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AW_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_frs_al()
{
	if(CPU_TYPE_IS_000() || m_s_flag) /* NS990408 */
	{
		uint32_t ea = EA_AL_16();
		m68ki_write_16(ea, m68ki_get_sr());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_tos_d()
{
	if(m_s_flag)
	{
		m68ki_set_sr(DY());
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_16_tos_ai()
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


void m68000_base_device::m68k_op_move_16_tos_pi()
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


void m68000_base_device::m68k_op_move_16_tos_pd()
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


void m68000_base_device::m68k_op_move_16_tos_di()
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


void m68000_base_device::m68k_op_move_16_tos_ix()
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


void m68000_base_device::m68k_op_move_16_tos_aw()
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


void m68000_base_device::m68k_op_move_16_tos_al()
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


void m68000_base_device::m68k_op_move_16_tos_pcdi()
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


void m68000_base_device::m68k_op_move_16_tos_pcix()
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


void m68000_base_device::m68k_op_move_16_tos_i()
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


void m68000_base_device::m68k_op_move_32_fru()
{
	if(m_s_flag)
	{
		AY() = REG_USP();
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_move_32_tou()
{
	if(m_s_flag)
	{
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		REG_USP() = AY();
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_movec_32_cr()
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


void m68000_base_device::m68k_op_movec_32_rc()
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

//                  logerror("movec to cacr=%04x\n", m_cacr);
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


void m68000_base_device::m68k_op_movem_16_re_pd()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_re_ai()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_re_di()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_re_ix()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_re_aw()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_re_al()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_32_re_pd()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_re_ai()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_re_di()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_re_ix()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_re_aw()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_re_al()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_16_er_pi()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_er_pcdi()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_er_pcix()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_er_ai()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_er_di()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_er_ix()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_er_aw()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_16_er_al()
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

	m_remaining_cycles -= count<<m_cyc_movem_w;
}


void m68000_base_device::m68k_op_movem_32_er_pi()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_er_pcdi()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_er_pcix()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_er_ai()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_er_di()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_er_ix()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_er_aw()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movem_32_er_al()
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

	m_remaining_cycles -= count<<m_cyc_movem_l;
}


void m68000_base_device::m68k_op_movep_16_re()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t src = DX();

	m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 8));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));
}


void m68000_base_device::m68k_op_movep_32_re()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t src = DX();

	m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 24));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 16));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 8));
	m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));
}


void m68000_base_device::m68k_op_movep_16_er()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t* r_dst = &DX();

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | ((m68ki_read_8(ea) << 8) + m68ki_read_8(ea + 2));
}


void m68000_base_device::m68k_op_movep_32_er()
{
	uint32_t ea = EA_AY_DI_32();

	DX() = (m68ki_read_8(ea) << 24) + (m68ki_read_8(ea + 2) << 16)
		+ (m68ki_read_8(ea + 4) << 8) + m68ki_read_8(ea + 6);
}


void m68000_base_device::m68k_op_moves_8_ai()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_8_pi()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_8_pi7()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_8_pd()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_8_pd7()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_8_di()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_8_ix()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_8_aw()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_8_al()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_8(REG_D()[(word2 >> 12) & 7]) | m68ki_read_8_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_16_ai()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_16_pi()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_16_pd()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_16_di()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_16_ix()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_16_aw()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_16_al()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to data register */
			REG_D()[(word2 >> 12) & 7] = MASK_OUT_BELOW_16(REG_D()[(word2 >> 12) & 7]) | m68ki_read_16_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_32_ai()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_32_pi()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_32_pd()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_32_di()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_32_ix()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_32_aw()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moves_32_al()
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
					m_remaining_cycles -= 2;
				return;
			}
			/* Memory to register */
			REG_DA()[(word2 >> 12) & 15] = m68ki_read_32_fc(ea, m_sfc);
			if(CPU_TYPE_IS_020_VARIANT())
				m_remaining_cycles -= 2;
			return;
		}
		m68ki_exception_privilege_violation();
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_moveq_32()
{
	uint32_t res = DX() = MAKE_INT_8(MASK_OUT_ABOVE_8(m_ir));

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_move16_32()
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


void m68000_base_device::m68k_op_muls_16_d()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(DY()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_AI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_PI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_PD_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_di()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_DI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AY_IX_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AW_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_al()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_AL_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_PCDI_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_PCIX_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_muls_16_i()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_32(MAKE_INT_16(OPER_I_16()) * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_d()
{
	uint32_t* r_dst = &DX();
	uint32_t res = MASK_OUT_ABOVE_16(DY()) * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_ai()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_AI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_pi()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_PI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_pd()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_PD_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_di()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_DI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_ix()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AY_IX_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_aw()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AW_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_al()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_AL_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_pcdi()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_PCDI_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_pcix()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_PCIX_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mulu_16_i()
{
	uint32_t* r_dst = &DX();
	uint32_t res = OPER_I_16() * MASK_OUT_ABOVE_16(*r_dst);

	*r_dst = res;

	m_not_z_flag = res;
	m_n_flag = NFLAG_32(res);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_mull_32_d()
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


void m68000_base_device::m68k_op_mull_32_ai()
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


void m68000_base_device::m68k_op_mull_32_pi()
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


void m68000_base_device::m68k_op_mull_32_pd()
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


void m68000_base_device::m68k_op_mull_32_di()
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


void m68000_base_device::m68k_op_mull_32_ix()
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


void m68000_base_device::m68k_op_mull_32_aw()
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


void m68000_base_device::m68k_op_mull_32_al()
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


void m68000_base_device::m68k_op_mull_32_pcdi()
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


void m68000_base_device::m68k_op_mull_32_pcix()
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


void m68000_base_device::m68k_op_mull_32_i()
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


void m68000_base_device::m68k_op_nbcd_8_d()
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


void m68000_base_device::m68k_op_nbcd_8_ai()
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


void m68000_base_device::m68k_op_nbcd_8_pi()
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


void m68000_base_device::m68k_op_nbcd_8_pi7()
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


void m68000_base_device::m68k_op_nbcd_8_pd()
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


void m68000_base_device::m68k_op_nbcd_8_pd7()
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


void m68000_base_device::m68k_op_nbcd_8_di()
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


void m68000_base_device::m68k_op_nbcd_8_ix()
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


void m68000_base_device::m68k_op_nbcd_8_aw()
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


void m68000_base_device::m68k_op_nbcd_8_al()
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


void m68000_base_device::m68k_op_neg_8_d()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_8(*r_dst);

	m_n_flag = NFLAG_8(res);
	m_c_flag = m_x_flag = CFLAG_8(res);
	m_v_flag = *r_dst & res;
	m_not_z_flag = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | m_not_z_flag;
}


void m68000_base_device::m68k_op_neg_8_ai()
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


void m68000_base_device::m68k_op_neg_8_pi()
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


void m68000_base_device::m68k_op_neg_8_pi7()
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


void m68000_base_device::m68k_op_neg_8_pd()
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


void m68000_base_device::m68k_op_neg_8_pd7()
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


void m68000_base_device::m68k_op_neg_8_di()
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


void m68000_base_device::m68k_op_neg_8_ix()
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


void m68000_base_device::m68k_op_neg_8_aw()
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


void m68000_base_device::m68k_op_neg_8_al()
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


void m68000_base_device::m68k_op_neg_16_d()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - MASK_OUT_ABOVE_16(*r_dst);

	m_n_flag = NFLAG_16(res);
	m_c_flag = m_x_flag = CFLAG_16(res);
	m_v_flag = (*r_dst & res)>>8;
	m_not_z_flag = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | m_not_z_flag;
}


void m68000_base_device::m68k_op_neg_16_ai()
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


void m68000_base_device::m68k_op_neg_16_pi()
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


void m68000_base_device::m68k_op_neg_16_pd()
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


void m68000_base_device::m68k_op_neg_16_di()
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


void m68000_base_device::m68k_op_neg_16_ix()
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


void m68000_base_device::m68k_op_neg_16_aw()
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


void m68000_base_device::m68k_op_neg_16_al()
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


void m68000_base_device::m68k_op_neg_32_d()
{
	uint32_t* r_dst = &DY();
	uint32_t res = 0 - *r_dst;

	m_n_flag = NFLAG_32(res);
	m_c_flag = m_x_flag = CFLAG_SUB_32(*r_dst, 0, res);
	m_v_flag = (*r_dst & res)>>24;
	m_not_z_flag = MASK_OUT_ABOVE_32(res);

	*r_dst = m_not_z_flag;
}


void m68000_base_device::m68k_op_neg_32_ai()
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


void m68000_base_device::m68k_op_neg_32_pi()
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


void m68000_base_device::m68k_op_neg_32_pd()
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


void m68000_base_device::m68k_op_neg_32_di()
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


void m68000_base_device::m68k_op_neg_32_ix()
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


void m68000_base_device::m68k_op_neg_32_aw()
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


void m68000_base_device::m68k_op_neg_32_al()
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


void m68000_base_device::m68k_op_negx_8_d()
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


void m68000_base_device::m68k_op_negx_8_ai()
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


void m68000_base_device::m68k_op_negx_8_pi()
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


void m68000_base_device::m68k_op_negx_8_pi7()
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


void m68000_base_device::m68k_op_negx_8_pd()
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


void m68000_base_device::m68k_op_negx_8_pd7()
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


void m68000_base_device::m68k_op_negx_8_di()
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


void m68000_base_device::m68k_op_negx_8_ix()
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


void m68000_base_device::m68k_op_negx_8_aw()
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


void m68000_base_device::m68k_op_negx_8_al()
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


void m68000_base_device::m68k_op_negx_16_d()
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


void m68000_base_device::m68k_op_negx_16_ai()
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


void m68000_base_device::m68k_op_negx_16_pi()
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


void m68000_base_device::m68k_op_negx_16_pd()
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


void m68000_base_device::m68k_op_negx_16_di()
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


void m68000_base_device::m68k_op_negx_16_ix()
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


void m68000_base_device::m68k_op_negx_16_aw()
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


void m68000_base_device::m68k_op_negx_16_al()
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


void m68000_base_device::m68k_op_negx_32_d()
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


void m68000_base_device::m68k_op_negx_32_ai()
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


void m68000_base_device::m68k_op_negx_32_pi()
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


void m68000_base_device::m68k_op_negx_32_pd()
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


void m68000_base_device::m68k_op_negx_32_di()
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


void m68000_base_device::m68k_op_negx_32_ix()
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


void m68000_base_device::m68k_op_negx_32_aw()
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


void m68000_base_device::m68k_op_negx_32_al()
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


void m68000_base_device::m68k_op_nop()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
}


void m68000_base_device::m68k_op_not_8_d()
{
	uint32_t* r_dst = &DY();
	uint32_t res = MASK_OUT_ABOVE_8(~*r_dst);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_8_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_16_d()
{
	uint32_t* r_dst = &DY();
	uint32_t res = MASK_OUT_ABOVE_16(~*r_dst);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_16_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_16_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_16_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_16_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_16_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_16_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_16_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_32_d()
{
	uint32_t* r_dst = &DY();
	uint32_t res = *r_dst = MASK_OUT_ABOVE_32(~*r_dst);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_32_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_32_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_32_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_32_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_32_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_32_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_not_32_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_d()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= MASK_OUT_ABOVE_8(DY())));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_ai()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_AI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_pi()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_PI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_pi7()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_A7_PI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_pd()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_PD_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_pd7()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_A7_PD_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_di()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_DI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_ix()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AY_IX_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_aw()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AW_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_al()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_AL_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_pcdi()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_PCDI_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_pcix()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_PCIX_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_er_i()
{
	uint32_t res = MASK_OUT_ABOVE_8((DX() |= OPER_I_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_d()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= MASK_OUT_ABOVE_16(DY())));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_ai()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_AI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_pi()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_PI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_pd()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_PD_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_di()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_DI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_ix()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AY_IX_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_aw()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AW_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_al()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_AL_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_pcdi()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_PCDI_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_pcix()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_PCIX_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_er_i()
{
	uint32_t res = MASK_OUT_ABOVE_16((DX() |= OPER_I_16()));

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_d()
{
	uint32_t res = DX() |= DY();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_ai()
{
	uint32_t res = DX() |= OPER_AY_AI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_pi()
{
	uint32_t res = DX() |= OPER_AY_PI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_pd()
{
	uint32_t res = DX() |= OPER_AY_PD_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_di()
{
	uint32_t res = DX() |= OPER_AY_DI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_ix()
{
	uint32_t res = DX() |= OPER_AY_IX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_aw()
{
	uint32_t res = DX() |= OPER_AW_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_al()
{
	uint32_t res = DX() |= OPER_AL_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_pcdi()
{
	uint32_t res = DX() |= OPER_PCDI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_pcix()
{
	uint32_t res = DX() |= OPER_PCIX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_er_i()
{
	uint32_t res = DX() |= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_ai()
{
	uint32_t ea = EA_AY_AI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_pi()
{
	uint32_t ea = EA_AY_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_pi7()
{
	uint32_t ea = EA_A7_PI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_pd()
{
	uint32_t ea = EA_AY_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_pd7()
{
	uint32_t ea = EA_A7_PD_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_di()
{
	uint32_t ea = EA_AY_DI_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_ix()
{
	uint32_t ea = EA_AY_IX_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_aw()
{
	uint32_t ea = EA_AW_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_8_re_al()
{
	uint32_t ea = EA_AL_8();
	uint32_t res = MASK_OUT_ABOVE_8(DX() | m68ki_read_8(ea));

	m68ki_write_8(ea, res);

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_re_ai()
{
	uint32_t ea = EA_AY_AI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_re_pi()
{
	uint32_t ea = EA_AY_PI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_re_pd()
{
	uint32_t ea = EA_AY_PD_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_re_di()
{
	uint32_t ea = EA_AY_DI_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_re_ix()
{
	uint32_t ea = EA_AY_IX_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_re_aw()
{
	uint32_t ea = EA_AW_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_16_re_al()
{
	uint32_t ea = EA_AL_16();
	uint32_t res = MASK_OUT_ABOVE_16(DX() | m68ki_read_16(ea));

	m68ki_write_16(ea, res);

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_re_ai()
{
	uint32_t ea = EA_AY_AI_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_re_pi()
{
	uint32_t ea = EA_AY_PI_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_re_pd()
{
	uint32_t ea = EA_AY_PD_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_re_di()
{
	uint32_t ea = EA_AY_DI_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_re_ix()
{
	uint32_t ea = EA_AY_IX_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_re_aw()
{
	uint32_t ea = EA_AW_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_or_32_re_al()
{
	uint32_t ea = EA_AL_32();
	uint32_t res = DX() | m68ki_read_32(ea);

	m68ki_write_32(ea, res);

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_ori_8_d()
{
	uint32_t res = MASK_OUT_ABOVE_8((DY() |= OPER_I_8()));

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_ori_8_ai()
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


void m68000_base_device::m68k_op_ori_8_pi()
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


void m68000_base_device::m68k_op_ori_8_pi7()
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


void m68000_base_device::m68k_op_ori_8_pd()
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


void m68000_base_device::m68k_op_ori_8_pd7()
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


void m68000_base_device::m68k_op_ori_8_di()
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


void m68000_base_device::m68k_op_ori_8_ix()
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


void m68000_base_device::m68k_op_ori_8_aw()
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


void m68000_base_device::m68k_op_ori_8_al()
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


void m68000_base_device::m68k_op_ori_16_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY() |= OPER_I_16());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_ori_16_ai()
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


void m68000_base_device::m68k_op_ori_16_pi()
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


void m68000_base_device::m68k_op_ori_16_pd()
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


void m68000_base_device::m68k_op_ori_16_di()
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


void m68000_base_device::m68k_op_ori_16_ix()
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


void m68000_base_device::m68k_op_ori_16_aw()
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


void m68000_base_device::m68k_op_ori_16_al()
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


void m68000_base_device::m68k_op_ori_32_d()
{
	uint32_t res = DY() |= OPER_I_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_ori_32_ai()
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


void m68000_base_device::m68k_op_ori_32_pi()
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


void m68000_base_device::m68k_op_ori_32_pd()
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


void m68000_base_device::m68k_op_ori_32_di()
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


void m68000_base_device::m68k_op_ori_32_ix()
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


void m68000_base_device::m68k_op_ori_32_aw()
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


void m68000_base_device::m68k_op_ori_32_al()
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


void m68000_base_device::m68k_op_ori_8_toc()
{
	m68ki_set_ccr(m68ki_get_ccr() | OPER_I_8());
}


void m68000_base_device::m68k_op_ori_16_tos()
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


void m68000_base_device::m68k_op_pack_16_rr()
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


void m68000_base_device::m68k_op_pack_16_mm_ax7()
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


void m68000_base_device::m68k_op_pack_16_mm_ay7()
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


void m68000_base_device::m68k_op_pack_16_mm_axy7()
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


void m68000_base_device::m68k_op_pack_16_mm()
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


void m68000_base_device::m68k_op_pea_32_ai()
{
	uint32_t ea = EA_AY_AI_32();

	m68ki_push_32(ea);
}


void m68000_base_device::m68k_op_pea_32_di()
{
	uint32_t ea = EA_AY_DI_32();

	m68ki_push_32(ea);
}


void m68000_base_device::m68k_op_pea_32_ix()
{
	uint32_t ea = EA_AY_IX_32();

	m68ki_push_32(ea);
}


void m68000_base_device::m68k_op_pea_32_aw()
{
	uint32_t ea = EA_AW_32();

	m68ki_push_32(ea);
}


void m68000_base_device::m68k_op_pea_32_al()
{
	uint32_t ea = EA_AL_32();

	m68ki_push_32(ea);
}


void m68000_base_device::m68k_op_pea_32_pcdi()
{
	uint32_t ea = EA_PCDI_32();

	m68ki_push_32(ea);
}


void m68000_base_device::m68k_op_pea_32_pcix()
{
	uint32_t ea = EA_PCIX_32();

	m68ki_push_32(ea);
}


void m68000_base_device::m68k_op_pflusha_32()
{
	if ((CPU_TYPE_IS_EC020_PLUS()) && (m_has_pmmu))
	{
		logerror("68040: unhandled PFLUSHA (ir=%04x)\n", m_ir);
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_pflushan_32()
{
	if ((CPU_TYPE_IS_EC020_PLUS()) && (m_has_pmmu))
	{
		logerror("68040: unhandled PFLUSHAN (ir=%04x)\n", m_ir);
		return;
	}
	m68ki_exception_1111();
}


void m68000_base_device::m68k_op_pmmu_32()
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


void m68000_base_device::m68k_op_ptest_32()
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


void m68000_base_device::m68k_op_reset()
{
	if(m_s_flag)
	{
		if (!m_reset_instr_callback.isnull())
			(m_reset_instr_callback)(1);
		m68k_reset_peripherals();
		m_remaining_cycles -= m_cyc_reset;
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_ror_8_s()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_8(src, shift);

	if(orig_shift != 0)
		m_remaining_cycles -= orig_shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = src << (9-orig_shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_ror_16_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_16(src, shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_ror_32_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src = *r_dst;
	uint32_t res = ROR_32(src, shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = src << (9-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_ror_8_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_8(src, shift);

	if(orig_shift != 0)
	{
		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_ror_16_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 15;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_16(src, shift);

	if(orig_shift != 0)
	{
		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_ror_32_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 31;
	uint64_t src = *r_dst;
	uint32_t res = ROR_32(src, shift);

	if(orig_shift != 0)
	{
		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_ror_16_ai()
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


void m68000_base_device::m68k_op_ror_16_pi()
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


void m68000_base_device::m68k_op_ror_16_pd()
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


void m68000_base_device::m68k_op_ror_16_di()
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


void m68000_base_device::m68k_op_ror_16_ix()
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


void m68000_base_device::m68k_op_ror_16_aw()
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


void m68000_base_device::m68k_op_ror_16_al()
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


void m68000_base_device::m68k_op_rol_8_s()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_8(src, shift);

	if(orig_shift != 0)
		m_remaining_cycles -= orig_shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_c_flag = src << orig_shift;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_rol_16_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROL_16(src, shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_c_flag = src >> (8-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_rol_32_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src = *r_dst;
	uint32_t res = ROL_32(src, shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_c_flag = src >> (24-shift);
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_rol_8_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 7;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_8(src, shift);

	if(orig_shift != 0)
	{
		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_rol_16_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 15;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = MASK_OUT_ABOVE_16(ROL_16(src, shift));

	if(orig_shift != 0)
	{
		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_rol_32_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;
	uint32_t shift = orig_shift & 31;
	uint64_t src = *r_dst;
	uint32_t res = ROL_32(src, shift);

	if(orig_shift != 0)
	{
		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_rol_16_ai()
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


void m68000_base_device::m68k_op_rol_16_pi()
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


void m68000_base_device::m68k_op_rol_16_pd()
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


void m68000_base_device::m68k_op_rol_16_di()
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


void m68000_base_device::m68k_op_rol_16_ix()
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


void m68000_base_device::m68k_op_rol_16_aw()
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


void m68000_base_device::m68k_op_rol_16_al()
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


void m68000_base_device::m68k_op_roxr_8_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROR_9(src | (XFLAG_1() << 8), shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	m_c_flag = m_x_flag = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_roxr_16_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROR_17(src | (XFLAG_1() << 16), shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_roxr_32_s()
{
	uint32_t*  r_dst = &DY();
	uint32_t   shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src   = *r_dst;
	uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	res = ROR_33_64(res, shift);

	m_c_flag = m_x_flag = res >> 24;
	res = MASK_OUT_ABOVE_32(res);

	*r_dst =  res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_roxr_8_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 9;
		uint32_t src   = MASK_OUT_ABOVE_8(*r_dst);
		uint32_t res   = ROR_9(src | (XFLAG_1() << 8), shift);

		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_roxr_16_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 17;
		uint32_t src   = MASK_OUT_ABOVE_16(*r_dst);
		uint32_t res   = ROR_17(src | (XFLAG_1() << 16), shift);

		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_roxr_32_r()
{
	uint32_t*  r_dst = &DY();
	uint32_t   orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t   shift = orig_shift % 33;
		uint64_t src   = *r_dst;
		uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

		res = ROR_33_64(res, shift);

		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_roxr_16_ai()
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


void m68000_base_device::m68k_op_roxr_16_pi()
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


void m68000_base_device::m68k_op_roxr_16_pd()
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


void m68000_base_device::m68k_op_roxr_16_di()
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


void m68000_base_device::m68k_op_roxr_16_ix()
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


void m68000_base_device::m68k_op_roxr_16_aw()
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


void m68000_base_device::m68k_op_roxr_16_al()
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


void m68000_base_device::m68k_op_roxl_8_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_8(*r_dst);
	uint32_t res = ROL_9(src | (XFLAG_1() << 8), shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	m_c_flag = m_x_flag = res;
	res = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_roxl_16_s()
{
	uint32_t* r_dst = &DY();
	uint32_t shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint32_t src = MASK_OUT_ABOVE_16(*r_dst);
	uint32_t res = ROL_17(src | (XFLAG_1() << 16), shift);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	m_c_flag = m_x_flag = res >> 8;
	res = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_roxl_32_s()
{
	uint32_t*  r_dst = &DY();
	uint32_t   shift = (((m_ir >> 9) - 1) & 7) + 1;
	uint64_t src   = *r_dst;
	uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

	if(shift != 0)
		m_remaining_cycles -= shift<<m_cyc_shift;

	res = ROL_33_64(res, shift);

	m_c_flag = m_x_flag = res >> 24;
	res = MASK_OUT_ABOVE_32(res);

	*r_dst = res;

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_roxl_8_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;


	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 9;
		uint32_t src   = MASK_OUT_ABOVE_8(*r_dst);
		uint32_t res   = ROL_9(src | (XFLAG_1() << 8), shift);

		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_roxl_16_r()
{
	uint32_t* r_dst = &DY();
	uint32_t orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t shift = orig_shift % 17;
		uint32_t src   = MASK_OUT_ABOVE_16(*r_dst);
		uint32_t res   = ROL_17(src | (XFLAG_1() << 16), shift);

		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_roxl_32_r()
{
	uint32_t*  r_dst = &DY();
	uint32_t   orig_shift = DX() & 0x3f;

	if(orig_shift != 0)
	{
		uint32_t   shift = orig_shift % 33;
		uint64_t src   = *r_dst;
		uint64_t res   = src | (((uint64_t)XFLAG_1()) << 32);

		res = ROL_33_64(res, shift);

		m_remaining_cycles -= orig_shift<<m_cyc_shift;

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


void m68000_base_device::m68k_op_roxl_16_ai()
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


void m68000_base_device::m68k_op_roxl_16_pi()
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


void m68000_base_device::m68k_op_roxl_16_pd()
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


void m68000_base_device::m68k_op_roxl_16_di()
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


void m68000_base_device::m68k_op_roxl_16_ix()
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


void m68000_base_device::m68k_op_roxl_16_aw()
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


void m68000_base_device::m68k_op_roxl_16_al()
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


void m68000_base_device::m68k_op_rtd_32()
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


void m68000_base_device::m68k_op_rte_32()
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


void m68000_base_device::m68k_op_rtm_32()
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


void m68000_base_device::m68k_op_rtr_32()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_set_ccr(m68ki_pull_16());
	m68ki_jump(m68ki_pull_32());
}


void m68000_base_device::m68k_op_rts_32()
{
	m68ki_trace_t0();                  /* auto-disable (see m68kcpu.h) */
	m68ki_jump(m68ki_pull_32());
}


void m68000_base_device::m68k_op_sbcd_8_rr()
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


void m68000_base_device::m68k_op_sbcd_8_mm_ax7()
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


void m68000_base_device::m68k_op_sbcd_8_mm_ay7()
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


void m68000_base_device::m68k_op_sbcd_8_mm_axy7()
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


void m68000_base_device::m68k_op_sbcd_8_mm()
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


void m68000_base_device::m68k_op_st_8_d()
{
	DY() |= 0xff;
}


void m68000_base_device::m68k_op_st_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), 0xff);
}


void m68000_base_device::m68k_op_st_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), 0xff);
}


void m68000_base_device::m68k_op_st_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), 0xff);
}


void m68000_base_device::m68k_op_st_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), 0xff);
}


void m68000_base_device::m68k_op_st_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), 0xff);
}


void m68000_base_device::m68k_op_st_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), 0xff);
}


void m68000_base_device::m68k_op_st_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), 0xff);
}


void m68000_base_device::m68k_op_st_8_aw()
{
	m68ki_write_8(EA_AW_8(), 0xff);
}


void m68000_base_device::m68k_op_st_8_al()
{
	m68ki_write_8(EA_AL_8(), 0xff);
}


void m68000_base_device::m68k_op_sf_8_d()
{
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_sf_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), 0);
}


void m68000_base_device::m68k_op_sf_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), 0);
}


void m68000_base_device::m68k_op_sf_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), 0);
}


void m68000_base_device::m68k_op_sf_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), 0);
}


void m68000_base_device::m68k_op_sf_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), 0);
}


void m68000_base_device::m68k_op_sf_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), 0);
}


void m68000_base_device::m68k_op_sf_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), 0);
}


void m68000_base_device::m68k_op_sf_8_aw()
{
	m68ki_write_8(EA_AW_8(), 0);
}


void m68000_base_device::m68k_op_sf_8_al()
{
	m68ki_write_8(EA_AL_8(), 0);
}


void m68000_base_device::m68k_op_shi_8_d()
{
	if(COND_HI())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_sls_8_d()
{
	if(COND_LS())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_scc_8_d()
{
	if(COND_CC())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_scs_8_d()
{
	if(COND_CS())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_sne_8_d()
{
	if(COND_NE())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_seq_8_d()
{
	if(COND_EQ())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_svc_8_d()
{
	if(COND_VC())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_svs_8_d()
{
	if(COND_VS())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_spl_8_d()
{
	if(COND_PL())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_smi_8_d()
{
	if(COND_MI())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_sge_8_d()
{
	if(COND_GE())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_slt_8_d()
{
	if(COND_LT())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_sgt_8_d()
{
	if(COND_GT())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_sle_8_d()
{
	if(COND_LE())
	{
		DY() |= 0xff;
		m_remaining_cycles -= m_cyc_scc_r_true;
		return;
	}
	DY() &= 0xffffff00;
}


void m68000_base_device::m68k_op_shi_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_shi_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_shi_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_shi_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_shi_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_shi_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_shi_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_shi_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_shi_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_HI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sls_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_LS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scc_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_CC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_scs_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_CS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sne_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_NE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_seq_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_EQ() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svc_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_VC() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_svs_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_VS() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_spl_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_PL() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_smi_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_MI() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sge_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_GE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_slt_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_LT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sgt_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_GT() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_ai()
{
	m68ki_write_8(EA_AY_AI_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_pi()
{
	m68ki_write_8(EA_AY_PI_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_pi7()
{
	m68ki_write_8(EA_A7_PI_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_pd()
{
	m68ki_write_8(EA_AY_PD_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_pd7()
{
	m68ki_write_8(EA_A7_PD_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_di()
{
	m68ki_write_8(EA_AY_DI_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_ix()
{
	m68ki_write_8(EA_AY_IX_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_aw()
{
	m68ki_write_8(EA_AW_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_sle_8_al()
{
	m68ki_write_8(EA_AL_8(), COND_LE() ? 0xff : 0);
}


void m68000_base_device::m68k_op_stop()
{
	if(m_s_flag)
	{
		uint32_t new_sr = OPER_I_16();
		m68ki_trace_t0();              /* auto-disable (see m68kcpu.h) */
		m_stopped |= STOP_LEVEL_STOP;
		m68ki_set_sr(new_sr);
		m_remaining_cycles = 0;
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68000_base_device::m68k_op_sub_8_er_d()
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


void m68000_base_device::m68k_op_sub_8_er_ai()
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


void m68000_base_device::m68k_op_sub_8_er_pi()
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


void m68000_base_device::m68k_op_sub_8_er_pi7()
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


void m68000_base_device::m68k_op_sub_8_er_pd()
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


void m68000_base_device::m68k_op_sub_8_er_pd7()
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


void m68000_base_device::m68k_op_sub_8_er_di()
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


void m68000_base_device::m68k_op_sub_8_er_ix()
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


void m68000_base_device::m68k_op_sub_8_er_aw()
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


void m68000_base_device::m68k_op_sub_8_er_al()
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


void m68000_base_device::m68k_op_sub_8_er_pcdi()
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


void m68000_base_device::m68k_op_sub_8_er_pcix()
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


void m68000_base_device::m68k_op_sub_8_er_i()
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


void m68000_base_device::m68k_op_sub_16_er_d()
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


void m68000_base_device::m68k_op_sub_16_er_a()
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


void m68000_base_device::m68k_op_sub_16_er_ai()
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


void m68000_base_device::m68k_op_sub_16_er_pi()
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


void m68000_base_device::m68k_op_sub_16_er_pd()
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


void m68000_base_device::m68k_op_sub_16_er_di()
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


void m68000_base_device::m68k_op_sub_16_er_ix()
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


void m68000_base_device::m68k_op_sub_16_er_aw()
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


void m68000_base_device::m68k_op_sub_16_er_al()
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


void m68000_base_device::m68k_op_sub_16_er_pcdi()
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


void m68000_base_device::m68k_op_sub_16_er_pcix()
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


void m68000_base_device::m68k_op_sub_16_er_i()
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


void m68000_base_device::m68k_op_sub_32_er_d()
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


void m68000_base_device::m68k_op_sub_32_er_a()
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


void m68000_base_device::m68k_op_sub_32_er_ai()
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


void m68000_base_device::m68k_op_sub_32_er_pi()
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


void m68000_base_device::m68k_op_sub_32_er_pd()
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


void m68000_base_device::m68k_op_sub_32_er_di()
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


void m68000_base_device::m68k_op_sub_32_er_ix()
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


void m68000_base_device::m68k_op_sub_32_er_aw()
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


void m68000_base_device::m68k_op_sub_32_er_al()
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


void m68000_base_device::m68k_op_sub_32_er_pcdi()
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


void m68000_base_device::m68k_op_sub_32_er_pcix()
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


void m68000_base_device::m68k_op_sub_32_er_i()
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


void m68000_base_device::m68k_op_sub_8_re_ai()
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


void m68000_base_device::m68k_op_sub_8_re_pi()
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


void m68000_base_device::m68k_op_sub_8_re_pi7()
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


void m68000_base_device::m68k_op_sub_8_re_pd()
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


void m68000_base_device::m68k_op_sub_8_re_pd7()
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


void m68000_base_device::m68k_op_sub_8_re_di()
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


void m68000_base_device::m68k_op_sub_8_re_ix()
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


void m68000_base_device::m68k_op_sub_8_re_aw()
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


void m68000_base_device::m68k_op_sub_8_re_al()
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


void m68000_base_device::m68k_op_sub_16_re_ai()
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


void m68000_base_device::m68k_op_sub_16_re_pi()
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


void m68000_base_device::m68k_op_sub_16_re_pd()
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


void m68000_base_device::m68k_op_sub_16_re_di()
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


void m68000_base_device::m68k_op_sub_16_re_ix()
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


void m68000_base_device::m68k_op_sub_16_re_aw()
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


void m68000_base_device::m68k_op_sub_16_re_al()
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


void m68000_base_device::m68k_op_sub_32_re_ai()
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


void m68000_base_device::m68k_op_sub_32_re_pi()
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


void m68000_base_device::m68k_op_sub_32_re_pd()
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


void m68000_base_device::m68k_op_sub_32_re_di()
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


void m68000_base_device::m68k_op_sub_32_re_ix()
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


void m68000_base_device::m68k_op_sub_32_re_aw()
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


void m68000_base_device::m68k_op_sub_32_re_al()
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


void m68000_base_device::m68k_op_suba_16_d()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(DY()));
}


void m68000_base_device::m68k_op_suba_16_a()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(AY()));
}


void m68000_base_device::m68k_op_suba_16_ai()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_AI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_pi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_pd()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_PD_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_di()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_DI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_ix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AY_IX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_aw()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AW_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_al()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_AL_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_pcdi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCDI_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_pcix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_PCIX_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_16_i()
{
	uint32_t* r_dst = &AX();
	uint32_t src = MAKE_INT_16(OPER_I_16());

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_d()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - DY());
}


void m68000_base_device::m68k_op_suba_32_a()
{
	uint32_t* r_dst = &AX();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - AY());
}


void m68000_base_device::m68k_op_suba_32_ai()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_AI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_pi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_pd()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_PD_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_di()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_DI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_ix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AY_IX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_aw()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AW_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_al()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_AL_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_pcdi()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCDI_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_pcix()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_PCIX_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_suba_32_i()
{
	uint32_t* r_dst = &AX();
	uint32_t src = OPER_I_32();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


void m68000_base_device::m68k_op_subi_8_d()
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


void m68000_base_device::m68k_op_subi_8_ai()
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


void m68000_base_device::m68k_op_subi_8_pi()
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


void m68000_base_device::m68k_op_subi_8_pi7()
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


void m68000_base_device::m68k_op_subi_8_pd()
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


void m68000_base_device::m68k_op_subi_8_pd7()
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


void m68000_base_device::m68k_op_subi_8_di()
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


void m68000_base_device::m68k_op_subi_8_ix()
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


void m68000_base_device::m68k_op_subi_8_aw()
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


void m68000_base_device::m68k_op_subi_8_al()
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


void m68000_base_device::m68k_op_subi_16_d()
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


void m68000_base_device::m68k_op_subi_16_ai()
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


void m68000_base_device::m68k_op_subi_16_pi()
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


void m68000_base_device::m68k_op_subi_16_pd()
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


void m68000_base_device::m68k_op_subi_16_di()
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


void m68000_base_device::m68k_op_subi_16_ix()
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


void m68000_base_device::m68k_op_subi_16_aw()
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


void m68000_base_device::m68k_op_subi_16_al()
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


void m68000_base_device::m68k_op_subi_32_d()
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


void m68000_base_device::m68k_op_subi_32_ai()
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


void m68000_base_device::m68k_op_subi_32_pi()
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


void m68000_base_device::m68k_op_subi_32_pd()
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


void m68000_base_device::m68k_op_subi_32_di()
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


void m68000_base_device::m68k_op_subi_32_ix()
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


void m68000_base_device::m68k_op_subi_32_aw()
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


void m68000_base_device::m68k_op_subi_32_al()
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


void m68000_base_device::m68k_op_subq_8_d()
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


void m68000_base_device::m68k_op_subq_8_ai()
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


void m68000_base_device::m68k_op_subq_8_pi()
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


void m68000_base_device::m68k_op_subq_8_pi7()
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


void m68000_base_device::m68k_op_subq_8_pd()
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


void m68000_base_device::m68k_op_subq_8_pd7()
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


void m68000_base_device::m68k_op_subq_8_di()
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


void m68000_base_device::m68k_op_subq_8_ix()
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


void m68000_base_device::m68k_op_subq_8_aw()
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


void m68000_base_device::m68k_op_subq_8_al()
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


void m68000_base_device::m68k_op_subq_16_d()
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


void m68000_base_device::m68k_op_subq_16_a()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((m_ir >> 9) - 1) & 7) + 1));
}


void m68000_base_device::m68k_op_subq_16_ai()
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


void m68000_base_device::m68k_op_subq_16_pi()
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


void m68000_base_device::m68k_op_subq_16_pd()
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


void m68000_base_device::m68k_op_subq_16_di()
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


void m68000_base_device::m68k_op_subq_16_ix()
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


void m68000_base_device::m68k_op_subq_16_aw()
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


void m68000_base_device::m68k_op_subq_16_al()
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


void m68000_base_device::m68k_op_subq_32_d()
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


void m68000_base_device::m68k_op_subq_32_a()
{
	uint32_t* r_dst = &AY();

	*r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((m_ir >> 9) - 1) & 7) + 1));
}


void m68000_base_device::m68k_op_subq_32_ai()
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


void m68000_base_device::m68k_op_subq_32_pi()
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


void m68000_base_device::m68k_op_subq_32_pd()
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


void m68000_base_device::m68k_op_subq_32_di()
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


void m68000_base_device::m68k_op_subq_32_ix()
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


void m68000_base_device::m68k_op_subq_32_aw()
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


void m68000_base_device::m68k_op_subq_32_al()
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


void m68000_base_device::m68k_op_subx_8_rr()
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


void m68000_base_device::m68k_op_subx_16_rr()
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


void m68000_base_device::m68k_op_subx_32_rr()
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


void m68000_base_device::m68k_op_subx_8_mm_ax7()
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


void m68000_base_device::m68k_op_subx_8_mm_ay7()
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


void m68000_base_device::m68k_op_subx_8_mm_axy7()
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


void m68000_base_device::m68k_op_subx_8_mm()
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


void m68000_base_device::m68k_op_subx_16_mm()
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


void m68000_base_device::m68k_op_subx_32_mm()
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


void m68000_base_device::m68k_op_swap_32()
{
	uint32_t* r_dst = &DY();

	m_not_z_flag = MASK_OUT_ABOVE_32(*r_dst<<16);
	*r_dst = (*r_dst>>16) | m_not_z_flag;

	m_not_z_flag = *r_dst;
	m_n_flag = NFLAG_32(*r_dst);
	m_c_flag = CFLAG_CLEAR;
	m_v_flag = VFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tas_8_d()
{
	uint32_t* r_dst = &DY();

	m_not_z_flag = MASK_OUT_ABOVE_8(*r_dst);
	m_n_flag = NFLAG_8(*r_dst);
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
	*r_dst |= 0x80;
}


void m68000_base_device::m68k_op_tas_8_ai()
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


void m68000_base_device::m68k_op_tas_8_pi()
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


void m68000_base_device::m68k_op_tas_8_pi7()
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


void m68000_base_device::m68k_op_tas_8_pd()
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


void m68000_base_device::m68k_op_tas_8_pd7()
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


void m68000_base_device::m68k_op_tas_8_di()
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


void m68000_base_device::m68k_op_tas_8_ix()
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


void m68000_base_device::m68k_op_tas_8_aw()
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


void m68000_base_device::m68k_op_tas_8_al()
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


void m68000_base_device::m68k_op_trap()
{
	/* Trap#n stacks exception frame type 0 */
	m68ki_exception_trapN(EXCEPTION_TRAP_BASE + (m_ir & 0xf));    /* HJB 990403 */
}


void m68000_base_device::m68k_op_trapt()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapt_16()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapt_32()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapf()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapf_16()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m_pc += 2;
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapf_32()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		m_pc += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_traphi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_HI())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapls()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LS())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapcc()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CC())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapcs()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_CS())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapne()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_NE())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapeq()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_EQ())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapvc()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VC())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapvs()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_VS())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trappl()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_PL())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapmi()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_MI())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapge()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GE())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_traplt()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LT())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_trapgt()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_GT())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_traple()
{
	if(CPU_TYPE_IS_EC020_PLUS())
	{
		if(COND_LE())
			m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
		return;
	}
	m68ki_exception_illegal();
}


void m68000_base_device::m68k_op_traphi_16()
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


void m68000_base_device::m68k_op_trapls_16()
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


void m68000_base_device::m68k_op_trapcc_16()
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


void m68000_base_device::m68k_op_trapcs_16()
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


void m68000_base_device::m68k_op_trapne_16()
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


void m68000_base_device::m68k_op_trapeq_16()
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


void m68000_base_device::m68k_op_trapvc_16()
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


void m68000_base_device::m68k_op_trapvs_16()
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


void m68000_base_device::m68k_op_trappl_16()
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


void m68000_base_device::m68k_op_trapmi_16()
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


void m68000_base_device::m68k_op_trapge_16()
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


void m68000_base_device::m68k_op_traplt_16()
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


void m68000_base_device::m68k_op_trapgt_16()
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


void m68000_base_device::m68k_op_traple_16()
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


void m68000_base_device::m68k_op_traphi_32()
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


void m68000_base_device::m68k_op_trapls_32()
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


void m68000_base_device::m68k_op_trapcc_32()
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


void m68000_base_device::m68k_op_trapcs_32()
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


void m68000_base_device::m68k_op_trapne_32()
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


void m68000_base_device::m68k_op_trapeq_32()
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


void m68000_base_device::m68k_op_trapvc_32()
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


void m68000_base_device::m68k_op_trapvs_32()
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


void m68000_base_device::m68k_op_trappl_32()
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


void m68000_base_device::m68k_op_trapmi_32()
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


void m68000_base_device::m68k_op_trapge_32()
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


void m68000_base_device::m68k_op_traplt_32()
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


void m68000_base_device::m68k_op_trapgt_32()
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


void m68000_base_device::m68k_op_traple_32()
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


void m68000_base_device::m68k_op_trapv()
{
	if(COND_VC())
	{
		return;
	}
	m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
}


void m68000_base_device::m68k_op_tst_8_d()
{
	uint32_t res = MASK_OUT_ABOVE_8(DY());

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_ai()
{
	uint32_t res = OPER_AY_AI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_pi()
{
	uint32_t res = OPER_AY_PI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_pi7()
{
	uint32_t res = OPER_A7_PI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_pd()
{
	uint32_t res = OPER_AY_PD_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_pd7()
{
	uint32_t res = OPER_A7_PD_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_di()
{
	uint32_t res = OPER_AY_DI_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_ix()
{
	uint32_t res = OPER_AY_IX_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_aw()
{
	uint32_t res = OPER_AW_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_al()
{
	uint32_t res = OPER_AL_8();

	m_n_flag = NFLAG_8(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_8_pcdi()
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


void m68000_base_device::m68k_op_tst_8_pcix()
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


void m68000_base_device::m68k_op_tst_8_i()
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


void m68000_base_device::m68k_op_tst_16_d()
{
	uint32_t res = MASK_OUT_ABOVE_16(DY());

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_16_a()
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


void m68000_base_device::m68k_op_tst_16_ai()
{
	uint32_t res = OPER_AY_AI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_16_pi()
{
	uint32_t res = OPER_AY_PI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_16_pd()
{
	uint32_t res = OPER_AY_PD_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_16_di()
{
	uint32_t res = OPER_AY_DI_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_16_ix()
{
	uint32_t res = OPER_AY_IX_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_16_aw()
{
	uint32_t res = OPER_AW_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_16_al()
{
	uint32_t res = OPER_AL_16();

	m_n_flag = NFLAG_16(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_16_pcdi()
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


void m68000_base_device::m68k_op_tst_16_pcix()
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


void m68000_base_device::m68k_op_tst_16_i()
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


void m68000_base_device::m68k_op_tst_32_d()
{
	uint32_t res = DY();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_32_a()
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


void m68000_base_device::m68k_op_tst_32_ai()
{
	uint32_t res = OPER_AY_AI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_32_pi()
{
	uint32_t res = OPER_AY_PI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_32_pd()
{
	uint32_t res = OPER_AY_PD_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_32_di()
{
	uint32_t res = OPER_AY_DI_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_32_ix()
{
	uint32_t res = OPER_AY_IX_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_32_aw()
{
	uint32_t res = OPER_AW_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_32_al()
{
	uint32_t res = OPER_AL_32();

	m_n_flag = NFLAG_32(res);
	m_not_z_flag = res;
	m_v_flag = VFLAG_CLEAR;
	m_c_flag = CFLAG_CLEAR;
}


void m68000_base_device::m68k_op_tst_32_pcdi()
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


void m68000_base_device::m68k_op_tst_32_pcix()
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


void m68000_base_device::m68k_op_tst_32_i()
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


void m68000_base_device::m68k_op_unlk_32_a7()
{
	REG_A()[7] = m68ki_read_32(REG_A()[7]);
}


void m68000_base_device::m68k_op_unlk_32()
{
	uint32_t* r_dst = &AY();

	REG_A()[7] = *r_dst;
	*r_dst = m68ki_pull_32();
}


void m68000_base_device::m68k_op_unpk_16_rr()
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


void m68000_base_device::m68k_op_unpk_16_mm_ax7()
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


void m68000_base_device::m68k_op_unpk_16_mm_ay7()
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


void m68000_base_device::m68k_op_unpk_16_mm_axy7()
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


void m68000_base_device::m68k_op_unpk_16_mm()
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


void m68000_base_device::m68k_op_cinv_32()
{
	if(CPU_TYPE_IS_040_PLUS())
	{
		uint16_t ir = m_ir;
		uint8_t cache = (ir >> 6) & 3;
//      uint8_t scope = (ir >> 3) & 3;
//      logerror("68040 %s: pc=%08x ir=%04x cache=%d scope=%d register=%d\n", ir & 0x0020 ? "cpush" : "cinv", m_ppc, ir, cache, scope, ir & 7);
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


void m68000_base_device::m68k_op_cpush_32()
{
	if(CPU_TYPE_IS_040_PLUS())
	{
		logerror("%s at %08x: called unimplemented instruction %04x (cpush)\n",
						tag(), m_ppc, m_ir);
		return;
	}
	m68ki_exception_1111();
}


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */


/* ======================================================================== */
/* ========================= OPCODE TABLE BUILDER ========================= */
/* ======================================================================== */

m68000_base_device::opcode_handler_ptr m68000_base_device::m68ki_instruction_jump_table[NUM_CPU_TYPES][0x10000]; /* opcode handler jump table */
unsigned char m68000_base_device::m68ki_cycles[NUM_CPU_TYPES][0x10000]; /* Cycles used by CPU type */

/* Build the opcode handler jump table */

void m68000_base_device::m68ki_set_one(unsigned short opcode, const opcode_handler_struct *s)
{
	for(int i=0; i<NUM_CPU_TYPES; i++)
		if(s->cycles[i] != 0xff) {
			m68ki_cycles[i][opcode] = s->cycles[i];
			m68ki_instruction_jump_table[i][opcode] = s->opcode_handler;
		}
}

void m68000_base_device::m68ki_build_opcode_table(void)
{
/* Opcode handler table */
static const opcode_handler_struct m68k_opcode_handler_table[] =
{
/*   function                      mask    match    000  010  020  040 */


	{&m68000_base_device::m68k_op_1010, 0xf000, 0xa000, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_1111, 0xf000, 0xf000, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_moveq_32, 0xf100, 0x7000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cpbcc_32, 0xf180, 0xf080, {255, 255,   4,   4, 255, 255, 255}},
	{&m68000_base_device::m68k_op_cpgen_32, 0xf1c0, 0xf000, {255, 255,   4,   4, 255, 255, 255}},
	{&m68000_base_device::m68k_op_cpscc_32, 0xf1c0, 0xf040, {255, 255,   4,   4, 255, 255, 255}},
	{&m68000_base_device::m68k_op_pmmu_32, 0xfe00, 0xf000, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_bra_8, 0xff00, 0x6000, { 10,  10,  10,  10,  10,  10,  10}},
	{&m68000_base_device::m68k_op_bsr_8, 0xff00, 0x6100, { 18,  18,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_bhi_8, 0xff00, 0x6200, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bls_8, 0xff00, 0x6300, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bcc_8, 0xff00, 0x6400, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bcs_8, 0xff00, 0x6500, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bne_8, 0xff00, 0x6600, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_beq_8, 0xff00, 0x6700, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bvc_8, 0xff00, 0x6800, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bvs_8, 0xff00, 0x6900, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bpl_8, 0xff00, 0x6a00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bmi_8, 0xff00, 0x6b00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bge_8, 0xff00, 0x6c00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_blt_8, 0xff00, 0x6d00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bgt_8, 0xff00, 0x6e00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_ble_8, 0xff00, 0x6f00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_040fpu0_32, 0xff00, 0xf200, {255, 255,   0,   0,   0,   0, 255}},
	{&m68000_base_device::m68k_op_040fpu1_32, 0xff00, 0xf300, {255, 255,   0,   0,   0,   0, 255}},
	{&m68000_base_device::m68k_op_cinv_32, 0xff20, 0xf400, {255, 255, 255, 255,  16, 255, 255}},
	{&m68000_base_device::m68k_op_cpush_32, 0xff20, 0xf420, {255, 255, 255, 255,  16, 255, 255}},
	{&m68000_base_device::m68k_op_btst_32_r_d, 0xf1f8, 0x0100, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_movep_16_er, 0xf1f8, 0x0108, { 16,  16,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_btst_8_r_ai, 0xf1f8, 0x0110, {  8,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_pi, 0xf1f8, 0x0118, {  8,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_pd, 0xf1f8, 0x0120, { 10,  10,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_di, 0xf1f8, 0x0128, { 12,  12,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_ix, 0xf1f8, 0x0130, { 14,  14,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_bchg_32_r_d, 0xf1f8, 0x0140, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_movep_32_er, 0xf1f8, 0x0148, { 24,  24,  18,  18,  18,  18,  18}},
	{&m68000_base_device::m68k_op_bchg_8_r_ai, 0xf1f8, 0x0150, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bchg_8_r_pi, 0xf1f8, 0x0158, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bchg_8_r_pd, 0xf1f8, 0x0160, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bchg_8_r_di, 0xf1f8, 0x0168, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bchg_8_r_ix, 0xf1f8, 0x0170, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_bclr_32_r_d, 0xf1f8, 0x0180, { 10,  10,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_movep_16_re, 0xf1f8, 0x0188, { 16,  16,  11,  11,  11,  11,  11}},
	{&m68000_base_device::m68k_op_bclr_8_r_ai, 0xf1f8, 0x0190, { 12,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_r_pi, 0xf1f8, 0x0198, { 12,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_r_pd, 0xf1f8, 0x01a0, { 14,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bclr_8_r_di, 0xf1f8, 0x01a8, { 16,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bclr_8_r_ix, 0xf1f8, 0x01b0, { 18,  20,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_bset_32_r_d, 0xf1f8, 0x01c0, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_movep_32_re, 0xf1f8, 0x01c8, { 24,  24,  17,  17,  17,  17,  17}},
	{&m68000_base_device::m68k_op_bset_8_r_ai, 0xf1f8, 0x01d0, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_r_pi, 0xf1f8, 0x01d8, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_r_pd, 0xf1f8, 0x01e0, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bset_8_r_di, 0xf1f8, 0x01e8, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bset_8_r_ix, 0xf1f8, 0x01f0, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_d_d, 0xf1f8, 0x1000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_move_8_d_ai, 0xf1f8, 0x1010, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_8_d_pi, 0xf1f8, 0x1018, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_8_d_pd, 0xf1f8, 0x1020, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_8_d_di, 0xf1f8, 0x1028, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_8_d_ix, 0xf1f8, 0x1030, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_move_8_ai_d, 0xf1f8, 0x1080, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_ai, 0xf1f8, 0x1090, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_pi, 0xf1f8, 0x1098, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_pd, 0xf1f8, 0x10a0, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_di, 0xf1f8, 0x10a8, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_ix, 0xf1f8, 0x10b0, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_d, 0xf1f8, 0x10c0, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_ai, 0xf1f8, 0x10d0, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_pi, 0xf1f8, 0x10d8, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_pd, 0xf1f8, 0x10e0, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_di, 0xf1f8, 0x10e8, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_ix, 0xf1f8, 0x10f0, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_pd_d, 0xf1f8, 0x1100, {  8,   8,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_ai, 0xf1f8, 0x1110, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_pi, 0xf1f8, 0x1118, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_pd, 0xf1f8, 0x1120, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_di, 0xf1f8, 0x1128, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_ix, 0xf1f8, 0x1130, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_8_di_d, 0xf1f8, 0x1140, { 12,  12,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_8_di_ai, 0xf1f8, 0x1150, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_di_pi, 0xf1f8, 0x1158, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_di_pd, 0xf1f8, 0x1160, { 18,  18,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_di_di, 0xf1f8, 0x1168, { 20,  20,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_di_ix, 0xf1f8, 0x1170, { 22,  22,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_8_ix_d, 0xf1f8, 0x1180, { 14,  14,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_ai, 0xf1f8, 0x1190, { 18,  18,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_pi, 0xf1f8, 0x1198, { 18,  18,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_pd, 0xf1f8, 0x11a0, { 20,  20,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_di, 0xf1f8, 0x11a8, { 22,  22,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_ix, 0xf1f8, 0x11b0, { 24,  24,  14,  14,  14,  14,   7}},
	{&m68000_base_device::m68k_op_move_32_d_d, 0xf1f8, 0x2000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_move_32_d_a, 0xf1f8, 0x2008, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_move_32_d_ai, 0xf1f8, 0x2010, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_32_d_pi, 0xf1f8, 0x2018, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_32_d_pd, 0xf1f8, 0x2020, { 14,  14,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_32_d_di, 0xf1f8, 0x2028, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_32_d_ix, 0xf1f8, 0x2030, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_movea_32_d, 0xf1f8, 0x2040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_movea_32_a, 0xf1f8, 0x2048, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_movea_32_ai, 0xf1f8, 0x2050, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_32_pi, 0xf1f8, 0x2058, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_32_pd, 0xf1f8, 0x2060, { 14,  14,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_movea_32_di, 0xf1f8, 0x2068, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_movea_32_ix, 0xf1f8, 0x2070, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_move_32_ai_d, 0xf1f8, 0x2080, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_a, 0xf1f8, 0x2088, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_ai, 0xf1f8, 0x2090, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_pi, 0xf1f8, 0x2098, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_pd, 0xf1f8, 0x20a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_di, 0xf1f8, 0x20a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_ix, 0xf1f8, 0x20b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_d, 0xf1f8, 0x20c0, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_a, 0xf1f8, 0x20c8, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_ai, 0xf1f8, 0x20d0, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_pi, 0xf1f8, 0x20d8, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_pd, 0xf1f8, 0x20e0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_di, 0xf1f8, 0x20e8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_ix, 0xf1f8, 0x20f0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_32_pd_d, 0xf1f8, 0x2100, { 12,  14,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_a, 0xf1f8, 0x2108, { 12,  14,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_ai, 0xf1f8, 0x2110, { 20,  22,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_pi, 0xf1f8, 0x2118, { 20,  22,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_pd, 0xf1f8, 0x2120, { 22,  24,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_di, 0xf1f8, 0x2128, { 24,  26,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_ix, 0xf1f8, 0x2130, { 26,  28,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_32_di_d, 0xf1f8, 0x2140, { 16,  16,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_32_di_a, 0xf1f8, 0x2148, { 16,  16,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_32_di_ai, 0xf1f8, 0x2150, { 24,  24,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_di_pi, 0xf1f8, 0x2158, { 24,  24,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_di_pd, 0xf1f8, 0x2160, { 26,  26,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_32_di_di, 0xf1f8, 0x2168, { 28,  28,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_32_di_ix, 0xf1f8, 0x2170, { 30,  30,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_32_ix_d, 0xf1f8, 0x2180, { 18,  18,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_a, 0xf1f8, 0x2188, { 18,  18,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_ai, 0xf1f8, 0x2190, { 26,  26,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_pi, 0xf1f8, 0x2198, { 26,  26,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_pd, 0xf1f8, 0x21a0, { 28,  28,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_di, 0xf1f8, 0x21a8, { 30,  30,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_ix, 0xf1f8, 0x21b0, { 32,  32,  14,  14,  14,  14,   7}},
	{&m68000_base_device::m68k_op_move_16_d_d, 0xf1f8, 0x3000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_move_16_d_a, 0xf1f8, 0x3008, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_move_16_d_ai, 0xf1f8, 0x3010, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_16_d_pi, 0xf1f8, 0x3018, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_16_d_pd, 0xf1f8, 0x3020, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_16_d_di, 0xf1f8, 0x3028, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_16_d_ix, 0xf1f8, 0x3030, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_movea_16_d, 0xf1f8, 0x3040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_movea_16_a, 0xf1f8, 0x3048, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_movea_16_ai, 0xf1f8, 0x3050, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_16_pi, 0xf1f8, 0x3058, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_16_pd, 0xf1f8, 0x3060, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_movea_16_di, 0xf1f8, 0x3068, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_movea_16_ix, 0xf1f8, 0x3070, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_move_16_ai_d, 0xf1f8, 0x3080, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_a, 0xf1f8, 0x3088, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_ai, 0xf1f8, 0x3090, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_pi, 0xf1f8, 0x3098, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_pd, 0xf1f8, 0x30a0, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_di, 0xf1f8, 0x30a8, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_ix, 0xf1f8, 0x30b0, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_d, 0xf1f8, 0x30c0, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_a, 0xf1f8, 0x30c8, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_ai, 0xf1f8, 0x30d0, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_pi, 0xf1f8, 0x30d8, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_pd, 0xf1f8, 0x30e0, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_di, 0xf1f8, 0x30e8, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_ix, 0xf1f8, 0x30f0, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_pd_d, 0xf1f8, 0x3100, {  8,   8,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_a, 0xf1f8, 0x3108, {  8,   8,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_ai, 0xf1f8, 0x3110, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_pi, 0xf1f8, 0x3118, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_pd, 0xf1f8, 0x3120, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_di, 0xf1f8, 0x3128, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_ix, 0xf1f8, 0x3130, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_16_di_d, 0xf1f8, 0x3140, { 12,  12,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_16_di_a, 0xf1f8, 0x3148, { 12,  12,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_16_di_ai, 0xf1f8, 0x3150, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_16_di_pi, 0xf1f8, 0x3158, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_16_di_pd, 0xf1f8, 0x3160, { 18,  18,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_16_di_di, 0xf1f8, 0x3168, { 20,  20,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_16_di_ix, 0xf1f8, 0x3170, { 22,  22,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_16_ix_d, 0xf1f8, 0x3180, { 14,  14,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_a, 0xf1f8, 0x3188, { 14,  14,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_ai, 0xf1f8, 0x3190, { 18,  18,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_pi, 0xf1f8, 0x3198, { 18,  18,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_pd, 0xf1f8, 0x31a0, { 20,  20,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_di, 0xf1f8, 0x31a8, { 22,  22,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_ix, 0xf1f8, 0x31b0, { 24,  24,  14,  14,  14,  14,   7}},
	{&m68000_base_device::m68k_op_chk_32_d, 0xf1f8, 0x4100, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_chk_32_ai, 0xf1f8, 0x4110, {255, 255,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_32_pi, 0xf1f8, 0x4118, {255, 255,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_32_pd, 0xf1f8, 0x4120, {255, 255,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_chk_32_di, 0xf1f8, 0x4128, {255, 255,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_chk_32_ix, 0xf1f8, 0x4130, {255, 255,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_chk_16_d, 0xf1f8, 0x4180, { 10,   8,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_chk_16_ai, 0xf1f8, 0x4190, { 14,  12,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_16_pi, 0xf1f8, 0x4198, { 14,  12,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_16_pd, 0xf1f8, 0x41a0, { 16,  14,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_chk_16_di, 0xf1f8, 0x41a8, { 18,  16,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_chk_16_ix, 0xf1f8, 0x41b0, { 20,  18,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_lea_32_ai, 0xf1f8, 0x41d0, {  4,   4,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_lea_32_di, 0xf1f8, 0x41e8, {  8,   8,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_lea_32_ix, 0xf1f8, 0x41f0, { 12,  12,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_addq_8_d, 0xf1f8, 0x5000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addq_8_ai, 0xf1f8, 0x5010, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_8_pi, 0xf1f8, 0x5018, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_8_pd, 0xf1f8, 0x5020, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addq_8_di, 0xf1f8, 0x5028, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addq_8_ix, 0xf1f8, 0x5030, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_addq_16_d, 0xf1f8, 0x5040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addq_16_a, 0xf1f8, 0x5048, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addq_16_ai, 0xf1f8, 0x5050, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_16_pi, 0xf1f8, 0x5058, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_16_pd, 0xf1f8, 0x5060, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addq_16_di, 0xf1f8, 0x5068, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addq_16_ix, 0xf1f8, 0x5070, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_addq_32_d, 0xf1f8, 0x5080, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addq_32_a, 0xf1f8, 0x5088, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addq_32_ai, 0xf1f8, 0x5090, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_32_pi, 0xf1f8, 0x5098, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_32_pd, 0xf1f8, 0x50a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addq_32_di, 0xf1f8, 0x50a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addq_32_ix, 0xf1f8, 0x50b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_subq_8_d, 0xf1f8, 0x5100, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subq_8_ai, 0xf1f8, 0x5110, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_8_pi, 0xf1f8, 0x5118, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_8_pd, 0xf1f8, 0x5120, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subq_8_di, 0xf1f8, 0x5128, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subq_8_ix, 0xf1f8, 0x5130, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_subq_16_d, 0xf1f8, 0x5140, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subq_16_a, 0xf1f8, 0x5148, {  8,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subq_16_ai, 0xf1f8, 0x5150, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_16_pi, 0xf1f8, 0x5158, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_16_pd, 0xf1f8, 0x5160, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subq_16_di, 0xf1f8, 0x5168, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subq_16_ix, 0xf1f8, 0x5170, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_subq_32_d, 0xf1f8, 0x5180, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subq_32_a, 0xf1f8, 0x5188, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subq_32_ai, 0xf1f8, 0x5190, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_32_pi, 0xf1f8, 0x5198, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_32_pd, 0xf1f8, 0x51a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subq_32_di, 0xf1f8, 0x51a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subq_32_ix, 0xf1f8, 0x51b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_or_8_er_d, 0xf1f8, 0x8000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_or_8_er_ai, 0xf1f8, 0x8010, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_8_er_pi, 0xf1f8, 0x8018, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_8_er_pd, 0xf1f8, 0x8020, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_8_er_di, 0xf1f8, 0x8028, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_8_er_ix, 0xf1f8, 0x8030, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_or_16_er_d, 0xf1f8, 0x8040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_or_16_er_ai, 0xf1f8, 0x8050, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_16_er_pi, 0xf1f8, 0x8058, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_16_er_pd, 0xf1f8, 0x8060, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_16_er_di, 0xf1f8, 0x8068, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_16_er_ix, 0xf1f8, 0x8070, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_or_32_er_d, 0xf1f8, 0x8080, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_or_32_er_ai, 0xf1f8, 0x8090, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_32_er_pi, 0xf1f8, 0x8098, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_32_er_pd, 0xf1f8, 0x80a0, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_32_er_di, 0xf1f8, 0x80a8, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_32_er_ix, 0xf1f8, 0x80b0, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_divu_16_d, 0xf1f8, 0x80c0, {140, 108,  44,  44,  44,  44,  44}},
	{&m68000_base_device::m68k_op_divu_16_ai, 0xf1f8, 0x80d0, {144, 112,  48,  48,  48,  48,  44}},
	{&m68000_base_device::m68k_op_divu_16_pi, 0xf1f8, 0x80d8, {144, 112,  48,  48,  48,  48,  44}},
	{&m68000_base_device::m68k_op_divu_16_pd, 0xf1f8, 0x80e0, {146, 114,  49,  49,  49,  49,  44}},
	{&m68000_base_device::m68k_op_divu_16_di, 0xf1f8, 0x80e8, {148, 116,  49,  49,  49,  49,  44}},
	{&m68000_base_device::m68k_op_divu_16_ix, 0xf1f8, 0x80f0, {150, 118,  51,  51,  51,  51,  44}},
	{&m68000_base_device::m68k_op_sbcd_8_rr, 0xf1f8, 0x8100, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_sbcd_8_mm, 0xf1f8, 0x8108, { 18,  18,  16,  16,  16,  16,  16}},
	{&m68000_base_device::m68k_op_or_8_re_ai, 0xf1f8, 0x8110, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_8_re_pi, 0xf1f8, 0x8118, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_8_re_pd, 0xf1f8, 0x8120, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_or_8_re_di, 0xf1f8, 0x8128, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_or_8_re_ix, 0xf1f8, 0x8130, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_pack_16_rr, 0xf1f8, 0x8140, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_pack_16_mm, 0xf1f8, 0x8148, {255, 255,  13,  13,  13,  13,  13}},
	{&m68000_base_device::m68k_op_or_16_re_ai, 0xf1f8, 0x8150, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_16_re_pi, 0xf1f8, 0x8158, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_16_re_pd, 0xf1f8, 0x8160, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_or_16_re_di, 0xf1f8, 0x8168, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_or_16_re_ix, 0xf1f8, 0x8170, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_unpk_16_rr, 0xf1f8, 0x8180, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_unpk_16_mm, 0xf1f8, 0x8188, {255, 255,  13,  13,  13,  13,  13}},
	{&m68000_base_device::m68k_op_or_32_re_ai, 0xf1f8, 0x8190, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_32_re_pi, 0xf1f8, 0x8198, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_32_re_pd, 0xf1f8, 0x81a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_or_32_re_di, 0xf1f8, 0x81a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_or_32_re_ix, 0xf1f8, 0x81b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_divs_16_d, 0xf1f8, 0x81c0, {158, 122,  56,  56,  56,  56,  56}},
	{&m68000_base_device::m68k_op_divs_16_ai, 0xf1f8, 0x81d0, {162, 126,  60,  60,  60,  60,  56}},
	{&m68000_base_device::m68k_op_divs_16_pi, 0xf1f8, 0x81d8, {162, 126,  60,  60,  60,  60,  56}},
	{&m68000_base_device::m68k_op_divs_16_pd, 0xf1f8, 0x81e0, {164, 128,  61,  61,  61,  61,  56}},
	{&m68000_base_device::m68k_op_divs_16_di, 0xf1f8, 0x81e8, {166, 130,  61,  61,  61,  61,  56}},
	{&m68000_base_device::m68k_op_divs_16_ix, 0xf1f8, 0x81f0, {168, 132,  63,  63,  63,  63,  56}},
	{&m68000_base_device::m68k_op_sub_8_er_d, 0xf1f8, 0x9000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_ai, 0xf1f8, 0x9010, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_pi, 0xf1f8, 0x9018, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_pd, 0xf1f8, 0x9020, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_di, 0xf1f8, 0x9028, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_ix, 0xf1f8, 0x9030, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_d, 0xf1f8, 0x9040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_a, 0xf1f8, 0x9048, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_ai, 0xf1f8, 0x9050, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_pi, 0xf1f8, 0x9058, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_pd, 0xf1f8, 0x9060, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_di, 0xf1f8, 0x9068, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_ix, 0xf1f8, 0x9070, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_d, 0xf1f8, 0x9080, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_a, 0xf1f8, 0x9088, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_ai, 0xf1f8, 0x9090, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_pi, 0xf1f8, 0x9098, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_pd, 0xf1f8, 0x90a0, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_di, 0xf1f8, 0x90a8, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_ix, 0xf1f8, 0x90b0, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_suba_16_d, 0xf1f8, 0x90c0, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_suba_16_a, 0xf1f8, 0x90c8, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_suba_16_ai, 0xf1f8, 0x90d0, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_16_pi, 0xf1f8, 0x90d8, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_16_pd, 0xf1f8, 0x90e0, { 14,  14,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_suba_16_di, 0xf1f8, 0x90e8, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_suba_16_ix, 0xf1f8, 0x90f0, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_subx_8_rr, 0xf1f8, 0x9100, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subx_8_mm, 0xf1f8, 0x9108, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_sub_8_re_ai, 0xf1f8, 0x9110, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_8_re_pi, 0xf1f8, 0x9118, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_8_re_pd, 0xf1f8, 0x9120, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_sub_8_re_di, 0xf1f8, 0x9128, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_sub_8_re_ix, 0xf1f8, 0x9130, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_subx_16_rr, 0xf1f8, 0x9140, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subx_16_mm, 0xf1f8, 0x9148, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_sub_16_re_ai, 0xf1f8, 0x9150, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_16_re_pi, 0xf1f8, 0x9158, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_16_re_pd, 0xf1f8, 0x9160, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_sub_16_re_di, 0xf1f8, 0x9168, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_sub_16_re_ix, 0xf1f8, 0x9170, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_subx_32_rr, 0xf1f8, 0x9180, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subx_32_mm, 0xf1f8, 0x9188, { 30,  30,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_sub_32_re_ai, 0xf1f8, 0x9190, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_32_re_pi, 0xf1f8, 0x9198, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_32_re_pd, 0xf1f8, 0x91a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_sub_32_re_di, 0xf1f8, 0x91a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_sub_32_re_ix, 0xf1f8, 0x91b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_suba_32_d, 0xf1f8, 0x91c0, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_suba_32_a, 0xf1f8, 0x91c8, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_suba_32_ai, 0xf1f8, 0x91d0, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_32_pi, 0xf1f8, 0x91d8, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_32_pd, 0xf1f8, 0x91e0, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_suba_32_di, 0xf1f8, 0x91e8, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_suba_32_ix, 0xf1f8, 0x91f0, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmp_8_d, 0xf1f8, 0xb000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmp_8_ai, 0xf1f8, 0xb010, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_8_pi, 0xf1f8, 0xb018, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_8_pd, 0xf1f8, 0xb020, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_8_di, 0xf1f8, 0xb028, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_8_ix, 0xf1f8, 0xb030, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmp_16_d, 0xf1f8, 0xb040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmp_16_a, 0xf1f8, 0xb048, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmp_16_ai, 0xf1f8, 0xb050, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_16_pi, 0xf1f8, 0xb058, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_16_pd, 0xf1f8, 0xb060, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_16_di, 0xf1f8, 0xb068, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_16_ix, 0xf1f8, 0xb070, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmp_32_d, 0xf1f8, 0xb080, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmp_32_a, 0xf1f8, 0xb088, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmp_32_ai, 0xf1f8, 0xb090, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_32_pi, 0xf1f8, 0xb098, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_32_pd, 0xf1f8, 0xb0a0, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_32_di, 0xf1f8, 0xb0a8, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_32_ix, 0xf1f8, 0xb0b0, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmpa_16_d, 0xf1f8, 0xb0c0, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_a, 0xf1f8, 0xb0c8, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_ai, 0xf1f8, 0xb0d0, { 10,  10,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_pi, 0xf1f8, 0xb0d8, { 10,  10,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_pd, 0xf1f8, 0xb0e0, { 12,  12,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_di, 0xf1f8, 0xb0e8, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_ix, 0xf1f8, 0xb0f0, { 16,  16,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_eor_8_d, 0xf1f8, 0xb100, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmpm_8, 0xf1f8, 0xb108, { 12,  12,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_eor_8_ai, 0xf1f8, 0xb110, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_8_pi, 0xf1f8, 0xb118, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_8_pd, 0xf1f8, 0xb120, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eor_8_di, 0xf1f8, 0xb128, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eor_8_ix, 0xf1f8, 0xb130, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_eor_16_d, 0xf1f8, 0xb140, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmpm_16, 0xf1f8, 0xb148, { 12,  12,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_eor_16_ai, 0xf1f8, 0xb150, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_16_pi, 0xf1f8, 0xb158, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_16_pd, 0xf1f8, 0xb160, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eor_16_di, 0xf1f8, 0xb168, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eor_16_ix, 0xf1f8, 0xb170, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_eor_32_d, 0xf1f8, 0xb180, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmpm_32, 0xf1f8, 0xb188, { 20,  20,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_eor_32_ai, 0xf1f8, 0xb190, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_32_pi, 0xf1f8, 0xb198, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_32_pd, 0xf1f8, 0xb1a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eor_32_di, 0xf1f8, 0xb1a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eor_32_ix, 0xf1f8, 0xb1b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_d, 0xf1f8, 0xb1c0, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_a, 0xf1f8, 0xb1c8, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_ai, 0xf1f8, 0xb1d0, { 14,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_pi, 0xf1f8, 0xb1d8, { 14,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_pd, 0xf1f8, 0xb1e0, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_di, 0xf1f8, 0xb1e8, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_ix, 0xf1f8, 0xb1f0, { 20,  20,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_and_8_er_d, 0xf1f8, 0xc000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_and_8_er_ai, 0xf1f8, 0xc010, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_8_er_pi, 0xf1f8, 0xc018, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_8_er_pd, 0xf1f8, 0xc020, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_8_er_di, 0xf1f8, 0xc028, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_8_er_ix, 0xf1f8, 0xc030, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_and_16_er_d, 0xf1f8, 0xc040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_and_16_er_ai, 0xf1f8, 0xc050, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_16_er_pi, 0xf1f8, 0xc058, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_16_er_pd, 0xf1f8, 0xc060, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_16_er_di, 0xf1f8, 0xc068, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_16_er_ix, 0xf1f8, 0xc070, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_and_32_er_d, 0xf1f8, 0xc080, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_and_32_er_ai, 0xf1f8, 0xc090, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_32_er_pi, 0xf1f8, 0xc098, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_32_er_pd, 0xf1f8, 0xc0a0, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_32_er_di, 0xf1f8, 0xc0a8, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_32_er_ix, 0xf1f8, 0xc0b0, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_mulu_16_d, 0xf1f8, 0xc0c0, { 54,  30,  27,  27,  27,  27,  27}},
	{&m68000_base_device::m68k_op_mulu_16_ai, 0xf1f8, 0xc0d0, { 58,  34,  31,  31,  31,  31,  27}},
	{&m68000_base_device::m68k_op_mulu_16_pi, 0xf1f8, 0xc0d8, { 58,  34,  31,  31,  31,  31,  27}},
	{&m68000_base_device::m68k_op_mulu_16_pd, 0xf1f8, 0xc0e0, { 60,  36,  32,  32,  32,  32,  27}},
	{&m68000_base_device::m68k_op_mulu_16_di, 0xf1f8, 0xc0e8, { 62,  38,  32,  32,  32,  32,  27}},
	{&m68000_base_device::m68k_op_mulu_16_ix, 0xf1f8, 0xc0f0, { 64,  40,  34,  34,  34,  34,  27}},
	{&m68000_base_device::m68k_op_abcd_8_rr, 0xf1f8, 0xc100, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_abcd_8_mm, 0xf1f8, 0xc108, { 18,  18,  16,  16,  16,  16,  16}},
	{&m68000_base_device::m68k_op_and_8_re_ai, 0xf1f8, 0xc110, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_8_re_pi, 0xf1f8, 0xc118, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_8_re_pd, 0xf1f8, 0xc120, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_and_8_re_di, 0xf1f8, 0xc128, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_and_8_re_ix, 0xf1f8, 0xc130, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_exg_32_dd, 0xf1f8, 0xc140, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_exg_32_aa, 0xf1f8, 0xc148, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_and_16_re_ai, 0xf1f8, 0xc150, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_16_re_pi, 0xf1f8, 0xc158, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_16_re_pd, 0xf1f8, 0xc160, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_and_16_re_di, 0xf1f8, 0xc168, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_and_16_re_ix, 0xf1f8, 0xc170, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_exg_32_da, 0xf1f8, 0xc188, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_and_32_re_ai, 0xf1f8, 0xc190, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_32_re_pi, 0xf1f8, 0xc198, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_32_re_pd, 0xf1f8, 0xc1a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_and_32_re_di, 0xf1f8, 0xc1a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_and_32_re_ix, 0xf1f8, 0xc1b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_muls_16_d, 0xf1f8, 0xc1c0, { 54,  32,  27,  27,  27,  27,  27}},
	{&m68000_base_device::m68k_op_muls_16_ai, 0xf1f8, 0xc1d0, { 58,  36,  31,  31,  31,  31,  27}},
	{&m68000_base_device::m68k_op_muls_16_pi, 0xf1f8, 0xc1d8, { 58,  36,  31,  31,  31,  31,  27}},
	{&m68000_base_device::m68k_op_muls_16_pd, 0xf1f8, 0xc1e0, { 60,  38,  32,  32,  32,  32,  27}},
	{&m68000_base_device::m68k_op_muls_16_di, 0xf1f8, 0xc1e8, { 62,  40,  32,  32,  32,  32,  27}},
	{&m68000_base_device::m68k_op_muls_16_ix, 0xf1f8, 0xc1f0, { 64,  42,  34,  34,  34,  34,  27}},
	{&m68000_base_device::m68k_op_add_8_er_d, 0xf1f8, 0xd000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_add_8_er_ai, 0xf1f8, 0xd010, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_8_er_pi, 0xf1f8, 0xd018, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_8_er_pd, 0xf1f8, 0xd020, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_8_er_di, 0xf1f8, 0xd028, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_8_er_ix, 0xf1f8, 0xd030, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_add_16_er_d, 0xf1f8, 0xd040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_add_16_er_a, 0xf1f8, 0xd048, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_add_16_er_ai, 0xf1f8, 0xd050, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_16_er_pi, 0xf1f8, 0xd058, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_16_er_pd, 0xf1f8, 0xd060, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_16_er_di, 0xf1f8, 0xd068, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_16_er_ix, 0xf1f8, 0xd070, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_add_32_er_d, 0xf1f8, 0xd080, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_add_32_er_a, 0xf1f8, 0xd088, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_add_32_er_ai, 0xf1f8, 0xd090, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_32_er_pi, 0xf1f8, 0xd098, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_32_er_pd, 0xf1f8, 0xd0a0, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_32_er_di, 0xf1f8, 0xd0a8, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_32_er_ix, 0xf1f8, 0xd0b0, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_adda_16_d, 0xf1f8, 0xd0c0, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_adda_16_a, 0xf1f8, 0xd0c8, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_adda_16_ai, 0xf1f8, 0xd0d0, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_16_pi, 0xf1f8, 0xd0d8, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_16_pd, 0xf1f8, 0xd0e0, { 14,  14,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_adda_16_di, 0xf1f8, 0xd0e8, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_adda_16_ix, 0xf1f8, 0xd0f0, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_addx_8_rr, 0xf1f8, 0xd100, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addx_8_mm, 0xf1f8, 0xd108, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_add_8_re_ai, 0xf1f8, 0xd110, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_8_re_pi, 0xf1f8, 0xd118, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_8_re_pd, 0xf1f8, 0xd120, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_add_8_re_di, 0xf1f8, 0xd128, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_add_8_re_ix, 0xf1f8, 0xd130, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_addx_16_rr, 0xf1f8, 0xd140, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addx_16_mm, 0xf1f8, 0xd148, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_add_16_re_ai, 0xf1f8, 0xd150, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_16_re_pi, 0xf1f8, 0xd158, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_16_re_pd, 0xf1f8, 0xd160, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_add_16_re_di, 0xf1f8, 0xd168, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_add_16_re_ix, 0xf1f8, 0xd170, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_addx_32_rr, 0xf1f8, 0xd180, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addx_32_mm, 0xf1f8, 0xd188, { 30,  30,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_add_32_re_ai, 0xf1f8, 0xd190, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_32_re_pi, 0xf1f8, 0xd198, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_32_re_pd, 0xf1f8, 0xd1a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_add_32_re_di, 0xf1f8, 0xd1a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_add_32_re_ix, 0xf1f8, 0xd1b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_adda_32_d, 0xf1f8, 0xd1c0, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_adda_32_a, 0xf1f8, 0xd1c8, {  8,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_adda_32_ai, 0xf1f8, 0xd1d0, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_32_pi, 0xf1f8, 0xd1d8, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_32_pd, 0xf1f8, 0xd1e0, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_adda_32_di, 0xf1f8, 0xd1e8, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_adda_32_ix, 0xf1f8, 0xd1f0, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_asr_8_s, 0xf1f8, 0xe000, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_lsr_8_s, 0xf1f8, 0xe008, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_roxr_8_s, 0xf1f8, 0xe010, {  6,   6,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_ror_8_s, 0xf1f8, 0xe018, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asr_8_r, 0xf1f8, 0xe020, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_lsr_8_r, 0xf1f8, 0xe028, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_roxr_8_r, 0xf1f8, 0xe030, {  6,   6,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_ror_8_r, 0xf1f8, 0xe038, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asr_16_s, 0xf1f8, 0xe040, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_lsr_16_s, 0xf1f8, 0xe048, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_roxr_16_s, 0xf1f8, 0xe050, {  6,   6,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_ror_16_s, 0xf1f8, 0xe058, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asr_16_r, 0xf1f8, 0xe060, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_lsr_16_r, 0xf1f8, 0xe068, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_roxr_16_r, 0xf1f8, 0xe070, {  6,   6,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_ror_16_r, 0xf1f8, 0xe078, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asr_32_s, 0xf1f8, 0xe080, {  8,   8,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_lsr_32_s, 0xf1f8, 0xe088, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_roxr_32_s, 0xf1f8, 0xe090, {  8,   8,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_ror_32_s, 0xf1f8, 0xe098, {  8,   8,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asr_32_r, 0xf1f8, 0xe0a0, {  8,   8,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_lsr_32_r, 0xf1f8, 0xe0a8, {  8,   8,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_roxr_32_r, 0xf1f8, 0xe0b0, {  8,   8,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_ror_32_r, 0xf1f8, 0xe0b8, {  8,   8,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asl_8_s, 0xf1f8, 0xe100, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_lsl_8_s, 0xf1f8, 0xe108, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_roxl_8_s, 0xf1f8, 0xe110, {  6,   6,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_rol_8_s, 0xf1f8, 0xe118, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asl_8_r, 0xf1f8, 0xe120, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_lsl_8_r, 0xf1f8, 0xe128, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_roxl_8_r, 0xf1f8, 0xe130, {  6,   6,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_rol_8_r, 0xf1f8, 0xe138, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asl_16_s, 0xf1f8, 0xe140, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_lsl_16_s, 0xf1f8, 0xe148, {  6,   6,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_roxl_16_s, 0xf1f8, 0xe150, {  6,   6,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_rol_16_s, 0xf1f8, 0xe158, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asl_16_r, 0xf1f8, 0xe160, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_lsl_16_r, 0xf1f8, 0xe168, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_roxl_16_r, 0xf1f8, 0xe170, {  6,   6,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_rol_16_r, 0xf1f8, 0xe178, {  6,   6,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asl_32_s, 0xf1f8, 0xe180, {  8,   8,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_lsl_32_s, 0xf1f8, 0xe188, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_roxl_32_s, 0xf1f8, 0xe190, {  8,   8,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_rol_32_s, 0xf1f8, 0xe198, {  8,   8,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_asl_32_r, 0xf1f8, 0xe1a0, {  8,   8,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_lsl_32_r, 0xf1f8, 0xe1a8, {  8,   8,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_roxl_32_r, 0xf1f8, 0xe1b0, {  8,   8,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_rol_32_r, 0xf1f8, 0xe1b8, {  8,   8,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_cpdbcc_32, 0xf1f8, 0xf048, {255, 255,   4,   4, 255, 255, 255}},
	{&m68000_base_device::m68k_op_cptrapcc_32, 0xf1f8, 0xf078, {255, 255,   4,   4, 255, 255, 255}},
	{&m68000_base_device::m68k_op_ptest_32, 0xffd8, 0xf548, {255, 255, 255, 255,   8, 255, 255}},
	{&m68000_base_device::m68k_op_rtm_32, 0xfff0, 0x06c0, {255, 255,  19,  19,  19,  19,  19}},
	{&m68000_base_device::m68k_op_trap, 0xfff0, 0x4e40, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_pi7, 0xf1ff, 0x011f, {  8,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_pd7, 0xf1ff, 0x0127, { 10,  10,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_aw, 0xf1ff, 0x0138, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_al, 0xf1ff, 0x0139, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_pcdi, 0xf1ff, 0x013a, { 12,  12,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_pcix, 0xf1ff, 0x013b, { 14,  14,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_btst_8_r_i, 0xf1ff, 0x013c, {  8,   8,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_bchg_8_r_pi7, 0xf1ff, 0x015f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bchg_8_r_pd7, 0xf1ff, 0x0167, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bchg_8_r_aw, 0xf1ff, 0x0178, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bchg_8_r_al, 0xf1ff, 0x0179, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_r_pi7, 0xf1ff, 0x019f, { 12,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_r_pd7, 0xf1ff, 0x01a7, { 14,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bclr_8_r_aw, 0xf1ff, 0x01b8, { 16,  18,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_r_al, 0xf1ff, 0x01b9, { 20,  22,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_r_pi7, 0xf1ff, 0x01df, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_r_pd7, 0xf1ff, 0x01e7, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bset_8_r_aw, 0xf1ff, 0x01f8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_r_al, 0xf1ff, 0x01f9, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_d_pi7, 0xf1ff, 0x101f, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_8_d_pd7, 0xf1ff, 0x1027, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_8_d_aw, 0xf1ff, 0x1038, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_8_d_al, 0xf1ff, 0x1039, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_8_d_pcdi, 0xf1ff, 0x103a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_8_d_pcix, 0xf1ff, 0x103b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_move_8_d_i, 0xf1ff, 0x103c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_move_8_ai_pi7, 0xf1ff, 0x109f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_pd7, 0xf1ff, 0x10a7, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_aw, 0xf1ff, 0x10b8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_al, 0xf1ff, 0x10b9, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_pcdi, 0xf1ff, 0x10ba, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_pcix, 0xf1ff, 0x10bb, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_ai_i, 0xf1ff, 0x10bc, { 12,  12,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_pi7, 0xf1ff, 0x10df, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_pd7, 0xf1ff, 0x10e7, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_aw, 0xf1ff, 0x10f8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_al, 0xf1ff, 0x10f9, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_pcdi, 0xf1ff, 0x10fa, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_pcix, 0xf1ff, 0x10fb, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_pi_i, 0xf1ff, 0x10fc, { 12,  12,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_move_8_pd_pi7, 0xf1ff, 0x111f, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_pd7, 0xf1ff, 0x1127, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_aw, 0xf1ff, 0x1138, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_al, 0xf1ff, 0x1139, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_pcdi, 0xf1ff, 0x113a, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_pcix, 0xf1ff, 0x113b, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_8_pd_i, 0xf1ff, 0x113c, { 12,  12,   7,   7,   7,   7,   5}},
	{&m68000_base_device::m68k_op_move_8_di_pi7, 0xf1ff, 0x115f, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_di_pd7, 0xf1ff, 0x1167, { 18,  18,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_di_aw, 0xf1ff, 0x1178, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_di_al, 0xf1ff, 0x1179, { 24,  24,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_di_pcdi, 0xf1ff, 0x117a, { 20,  20,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_di_pcix, 0xf1ff, 0x117b, { 22,  22,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_8_di_i, 0xf1ff, 0x117c, { 16,  16,   7,   7,   7,   7,   5}},
	{&m68000_base_device::m68k_op_move_8_ix_pi7, 0xf1ff, 0x119f, { 18,  18,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_pd7, 0xf1ff, 0x11a7, { 20,  20,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_aw, 0xf1ff, 0x11b8, { 22,  22,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_al, 0xf1ff, 0x11b9, { 26,  26,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_pcdi, 0xf1ff, 0x11ba, { 22,  22,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_pcix, 0xf1ff, 0x11bb, { 24,  24,  14,  14,  14,  14,   7}},
	{&m68000_base_device::m68k_op_move_8_ix_i, 0xf1ff, 0x11bc, { 18,  18,   9,   9,   9,   9,   7}},
	{&m68000_base_device::m68k_op_move_32_d_aw, 0xf1ff, 0x2038, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_32_d_al, 0xf1ff, 0x2039, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_32_d_pcdi, 0xf1ff, 0x203a, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_32_d_pcix, 0xf1ff, 0x203b, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_move_32_d_i, 0xf1ff, 0x203c, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_32_aw, 0xf1ff, 0x2078, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_32_al, 0xf1ff, 0x2079, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_32_pcdi, 0xf1ff, 0x207a, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_movea_32_pcix, 0xf1ff, 0x207b, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_movea_32_i, 0xf1ff, 0x207c, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_32_ai_aw, 0xf1ff, 0x20b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_al, 0xf1ff, 0x20b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_pcdi, 0xf1ff, 0x20ba, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_pcix, 0xf1ff, 0x20bb, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_32_ai_i, 0xf1ff, 0x20bc, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_aw, 0xf1ff, 0x20f8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_al, 0xf1ff, 0x20f9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_pcdi, 0xf1ff, 0x20fa, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_pcix, 0xf1ff, 0x20fb, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_32_pi_i, 0xf1ff, 0x20fc, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_pd_aw, 0xf1ff, 0x2138, { 24,  26,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_al, 0xf1ff, 0x2139, { 28,  30,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_pcdi, 0xf1ff, 0x213a, { 24,  26,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_pcix, 0xf1ff, 0x213b, { 26,  28,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_32_pd_i, 0xf1ff, 0x213c, { 20,  22,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_di_aw, 0xf1ff, 0x2178, { 28,  28,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_di_al, 0xf1ff, 0x2179, { 32,  32,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_di_pcdi, 0xf1ff, 0x217a, { 28,  28,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_32_di_pcix, 0xf1ff, 0x217b, { 30,  30,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_32_di_i, 0xf1ff, 0x217c, { 24,  24,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_32_ix_aw, 0xf1ff, 0x21b8, { 30,  30,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_al, 0xf1ff, 0x21b9, { 34,  34,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_pcdi, 0xf1ff, 0x21ba, { 30,  30,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_pcix, 0xf1ff, 0x21bb, { 32,  32,  14,  14,  14,  14,   7}},
	{&m68000_base_device::m68k_op_move_32_ix_i, 0xf1ff, 0x21bc, { 26,  26,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_16_d_aw, 0xf1ff, 0x3038, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_16_d_al, 0xf1ff, 0x3039, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_move_16_d_pcdi, 0xf1ff, 0x303a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_move_16_d_pcix, 0xf1ff, 0x303b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_move_16_d_i, 0xf1ff, 0x303c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_movea_16_aw, 0xf1ff, 0x3078, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_16_al, 0xf1ff, 0x3079, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_movea_16_pcdi, 0xf1ff, 0x307a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_movea_16_pcix, 0xf1ff, 0x307b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_movea_16_i, 0xf1ff, 0x307c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_move_16_ai_aw, 0xf1ff, 0x30b8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_al, 0xf1ff, 0x30b9, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_pcdi, 0xf1ff, 0x30ba, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_pcix, 0xf1ff, 0x30bb, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_ai_i, 0xf1ff, 0x30bc, { 12,  12,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_aw, 0xf1ff, 0x30f8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_al, 0xf1ff, 0x30f9, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_pcdi, 0xf1ff, 0x30fa, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_pcix, 0xf1ff, 0x30fb, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_pi_i, 0xf1ff, 0x30fc, { 12,  12,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_move_16_pd_aw, 0xf1ff, 0x3138, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_al, 0xf1ff, 0x3139, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_pcdi, 0xf1ff, 0x313a, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_pcix, 0xf1ff, 0x313b, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_16_pd_i, 0xf1ff, 0x313c, { 12,  12,   7,   7,   7,   7,   5}},
	{&m68000_base_device::m68k_op_move_16_di_aw, 0xf1ff, 0x3178, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_16_di_al, 0xf1ff, 0x3179, { 24,  24,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_16_di_pcdi, 0xf1ff, 0x317a, { 20,  20,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_16_di_pcix, 0xf1ff, 0x317b, { 22,  22,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_16_di_i, 0xf1ff, 0x317c, { 16,  16,   7,   7,   7,   7,   5}},
	{&m68000_base_device::m68k_op_move_16_ix_aw, 0xf1ff, 0x31b8, { 22,  22,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_al, 0xf1ff, 0x31b9, { 26,  26,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_pcdi, 0xf1ff, 0x31ba, { 22,  22,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_pcix, 0xf1ff, 0x31bb, { 24,  24,  14,  14,  14,  14,   7}},
	{&m68000_base_device::m68k_op_move_16_ix_i, 0xf1ff, 0x31bc, { 18,  18,   9,   9,   9,   9,   7}},
	{&m68000_base_device::m68k_op_chk_32_aw, 0xf1ff, 0x4138, {255, 255,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_32_al, 0xf1ff, 0x4139, {255, 255,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_32_pcdi, 0xf1ff, 0x413a, {255, 255,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_chk_32_pcix, 0xf1ff, 0x413b, {255, 255,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_chk_32_i, 0xf1ff, 0x413c, {255, 255,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_16_aw, 0xf1ff, 0x41b8, { 18,  16,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_16_al, 0xf1ff, 0x41b9, { 22,  20,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_chk_16_pcdi, 0xf1ff, 0x41ba, { 18,  16,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_chk_16_pcix, 0xf1ff, 0x41bb, { 20,  18,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_chk_16_i, 0xf1ff, 0x41bc, { 14,  12,  10,  10,  10,  10,   8}},
	{&m68000_base_device::m68k_op_lea_32_aw, 0xf1ff, 0x41f8, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_lea_32_al, 0xf1ff, 0x41f9, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_lea_32_pcdi, 0xf1ff, 0x41fa, {  8,   8,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_lea_32_pcix, 0xf1ff, 0x41fb, { 12,  12,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_addq_8_pi7, 0xf1ff, 0x501f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_8_pd7, 0xf1ff, 0x5027, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addq_8_aw, 0xf1ff, 0x5038, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_8_al, 0xf1ff, 0x5039, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_16_aw, 0xf1ff, 0x5078, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_16_al, 0xf1ff, 0x5079, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_32_aw, 0xf1ff, 0x50b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addq_32_al, 0xf1ff, 0x50b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_8_pi7, 0xf1ff, 0x511f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_8_pd7, 0xf1ff, 0x5127, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subq_8_aw, 0xf1ff, 0x5138, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_8_al, 0xf1ff, 0x5139, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_16_aw, 0xf1ff, 0x5178, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_16_al, 0xf1ff, 0x5179, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_32_aw, 0xf1ff, 0x51b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subq_32_al, 0xf1ff, 0x51b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_8_er_pi7, 0xf1ff, 0x801f, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_8_er_pd7, 0xf1ff, 0x8027, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_8_er_aw, 0xf1ff, 0x8038, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_8_er_al, 0xf1ff, 0x8039, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_8_er_pcdi, 0xf1ff, 0x803a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_8_er_pcix, 0xf1ff, 0x803b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_or_8_er_i, 0xf1ff, 0x803c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_or_16_er_aw, 0xf1ff, 0x8078, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_16_er_al, 0xf1ff, 0x8079, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_16_er_pcdi, 0xf1ff, 0x807a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_16_er_pcix, 0xf1ff, 0x807b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_or_16_er_i, 0xf1ff, 0x807c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_or_32_er_aw, 0xf1ff, 0x80b8, { 18,  18,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_32_er_al, 0xf1ff, 0x80b9, { 22,  22,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_or_32_er_pcdi, 0xf1ff, 0x80ba, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_or_32_er_pcix, 0xf1ff, 0x80bb, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_or_32_er_i, 0xf1ff, 0x80bc, { 16,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_divu_16_aw, 0xf1ff, 0x80f8, {148, 116,  48,  48,  48,  48,  44}},
	{&m68000_base_device::m68k_op_divu_16_al, 0xf1ff, 0x80f9, {152, 120,  48,  48,  48,  48,  44}},
	{&m68000_base_device::m68k_op_divu_16_pcdi, 0xf1ff, 0x80fa, {148, 116,  49,  49,  49,  49,  44}},
	{&m68000_base_device::m68k_op_divu_16_pcix, 0xf1ff, 0x80fb, {150, 118,  51,  51,  51,  51,  44}},
	{&m68000_base_device::m68k_op_divu_16_i, 0xf1ff, 0x80fc, {144, 112,  46,  46,  46,  46,  44}},
	{&m68000_base_device::m68k_op_sbcd_8_mm_ay7, 0xf1ff, 0x810f, { 18,  18,  16,  16,  16,  16,  16}},
	{&m68000_base_device::m68k_op_or_8_re_pi7, 0xf1ff, 0x811f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_8_re_pd7, 0xf1ff, 0x8127, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_or_8_re_aw, 0xf1ff, 0x8138, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_8_re_al, 0xf1ff, 0x8139, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_pack_16_mm_ay7, 0xf1ff, 0x814f, {255, 255,  13,  13,  13,  13,  13}},
	{&m68000_base_device::m68k_op_or_16_re_aw, 0xf1ff, 0x8178, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_16_re_al, 0xf1ff, 0x8179, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_unpk_16_mm_ay7, 0xf1ff, 0x818f, {255, 255,  13,  13,  13,  13,  13}},
	{&m68000_base_device::m68k_op_or_32_re_aw, 0xf1ff, 0x81b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_or_32_re_al, 0xf1ff, 0x81b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_divs_16_aw, 0xf1ff, 0x81f8, {166, 130,  60,  60,  60,  60,  56}},
	{&m68000_base_device::m68k_op_divs_16_al, 0xf1ff, 0x81f9, {170, 134,  60,  60,  60,  60,  56}},
	{&m68000_base_device::m68k_op_divs_16_pcdi, 0xf1ff, 0x81fa, {166, 130,  61,  61,  61,  61,  56}},
	{&m68000_base_device::m68k_op_divs_16_pcix, 0xf1ff, 0x81fb, {168, 132,  63,  63,  63,  63,  56}},
	{&m68000_base_device::m68k_op_divs_16_i, 0xf1ff, 0x81fc, {162, 126,  58,  58,  58,  58,  56}},
	{&m68000_base_device::m68k_op_sub_8_er_pi7, 0xf1ff, 0x901f, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_pd7, 0xf1ff, 0x9027, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_aw, 0xf1ff, 0x9038, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_al, 0xf1ff, 0x9039, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_pcdi, 0xf1ff, 0x903a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_pcix, 0xf1ff, 0x903b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_sub_8_er_i, 0xf1ff, 0x903c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_aw, 0xf1ff, 0x9078, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_al, 0xf1ff, 0x9079, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_pcdi, 0xf1ff, 0x907a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_pcix, 0xf1ff, 0x907b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_sub_16_er_i, 0xf1ff, 0x907c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_aw, 0xf1ff, 0x90b8, { 18,  18,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_al, 0xf1ff, 0x90b9, { 22,  22,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_pcdi, 0xf1ff, 0x90ba, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_pcix, 0xf1ff, 0x90bb, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_sub_32_er_i, 0xf1ff, 0x90bc, { 16,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_16_aw, 0xf1ff, 0x90f8, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_16_al, 0xf1ff, 0x90f9, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_16_pcdi, 0xf1ff, 0x90fa, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_suba_16_pcix, 0xf1ff, 0x90fb, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_suba_16_i, 0xf1ff, 0x90fc, { 12,  12,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_subx_8_mm_ay7, 0xf1ff, 0x910f, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_sub_8_re_pi7, 0xf1ff, 0x911f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_8_re_pd7, 0xf1ff, 0x9127, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_sub_8_re_aw, 0xf1ff, 0x9138, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_8_re_al, 0xf1ff, 0x9139, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_16_re_aw, 0xf1ff, 0x9178, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_16_re_al, 0xf1ff, 0x9179, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_32_re_aw, 0xf1ff, 0x91b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_sub_32_re_al, 0xf1ff, 0x91b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_suba_32_aw, 0xf1ff, 0x91f8, { 18,  18,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_32_al, 0xf1ff, 0x91f9, { 22,  22,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_suba_32_pcdi, 0xf1ff, 0x91fa, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_suba_32_pcix, 0xf1ff, 0x91fb, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_suba_32_i, 0xf1ff, 0x91fc, { 16,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_8_pi7, 0xf1ff, 0xb01f, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_8_pd7, 0xf1ff, 0xb027, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_8_aw, 0xf1ff, 0xb038, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_8_al, 0xf1ff, 0xb039, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_8_pcdi, 0xf1ff, 0xb03a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_8_pcix, 0xf1ff, 0xb03b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmp_8_i, 0xf1ff, 0xb03c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_cmp_16_aw, 0xf1ff, 0xb078, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_16_al, 0xf1ff, 0xb079, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_16_pcdi, 0xf1ff, 0xb07a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_16_pcix, 0xf1ff, 0xb07b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmp_16_i, 0xf1ff, 0xb07c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_cmp_32_aw, 0xf1ff, 0xb0b8, { 18,  18,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_32_al, 0xf1ff, 0xb0b9, { 22,  22,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmp_32_pcdi, 0xf1ff, 0xb0ba, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmp_32_pcix, 0xf1ff, 0xb0bb, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmp_32_i, 0xf1ff, 0xb0bc, { 14,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpa_16_aw, 0xf1ff, 0xb0f8, { 14,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_al, 0xf1ff, 0xb0f9, { 18,  18,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_pcdi, 0xf1ff, 0xb0fa, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_pcix, 0xf1ff, 0xb0fb, { 16,  16,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_cmpa_16_i, 0xf1ff, 0xb0fc, { 10,  10,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_cmpm_8_ay7, 0xf1ff, 0xb10f, { 12,  12,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_eor_8_pi7, 0xf1ff, 0xb11f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_8_pd7, 0xf1ff, 0xb127, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eor_8_aw, 0xf1ff, 0xb138, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_8_al, 0xf1ff, 0xb139, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_16_aw, 0xf1ff, 0xb178, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_16_al, 0xf1ff, 0xb179, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_32_aw, 0xf1ff, 0xb1b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eor_32_al, 0xf1ff, 0xb1b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_aw, 0xf1ff, 0xb1f8, { 18,  18,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_al, 0xf1ff, 0xb1f9, { 22,  22,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_pcdi, 0xf1ff, 0xb1fa, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_pcix, 0xf1ff, 0xb1fb, { 20,  20,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_cmpa_32_i, 0xf1ff, 0xb1fc, { 14,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_8_er_pi7, 0xf1ff, 0xc01f, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_8_er_pd7, 0xf1ff, 0xc027, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_8_er_aw, 0xf1ff, 0xc038, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_8_er_al, 0xf1ff, 0xc039, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_8_er_pcdi, 0xf1ff, 0xc03a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_8_er_pcix, 0xf1ff, 0xc03b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_and_8_er_i, 0xf1ff, 0xc03c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_and_16_er_aw, 0xf1ff, 0xc078, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_16_er_al, 0xf1ff, 0xc079, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_16_er_pcdi, 0xf1ff, 0xc07a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_16_er_pcix, 0xf1ff, 0xc07b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_and_16_er_i, 0xf1ff, 0xc07c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_and_32_er_aw, 0xf1ff, 0xc0b8, { 18,  18,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_32_er_al, 0xf1ff, 0xc0b9, { 22,  22,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_and_32_er_pcdi, 0xf1ff, 0xc0ba, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_and_32_er_pcix, 0xf1ff, 0xc0bb, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_and_32_er_i, 0xf1ff, 0xc0bc, { 16,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_mulu_16_aw, 0xf1ff, 0xc0f8, { 62,  38,  31,  31,  31,  31,  27}},
	{&m68000_base_device::m68k_op_mulu_16_al, 0xf1ff, 0xc0f9, { 66,  42,  31,  31,  31,  31,  27}},
	{&m68000_base_device::m68k_op_mulu_16_pcdi, 0xf1ff, 0xc0fa, { 62,  38,  32,  32,  32,  32,  27}},
	{&m68000_base_device::m68k_op_mulu_16_pcix, 0xf1ff, 0xc0fb, { 64,  40,  34,  34,  34,  34,  27}},
	{&m68000_base_device::m68k_op_mulu_16_i, 0xf1ff, 0xc0fc, { 58,  34,  29,  29,  29,  29,  27}},
	{&m68000_base_device::m68k_op_abcd_8_mm_ay7, 0xf1ff, 0xc10f, { 18,  18,  16,  16,  16,  16,  16}},
	{&m68000_base_device::m68k_op_and_8_re_pi7, 0xf1ff, 0xc11f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_8_re_pd7, 0xf1ff, 0xc127, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_and_8_re_aw, 0xf1ff, 0xc138, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_8_re_al, 0xf1ff, 0xc139, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_16_re_aw, 0xf1ff, 0xc178, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_16_re_al, 0xf1ff, 0xc179, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_32_re_aw, 0xf1ff, 0xc1b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_and_32_re_al, 0xf1ff, 0xc1b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_muls_16_aw, 0xf1ff, 0xc1f8, { 62,  40,  31,  31,  31,  31,  27}},
	{&m68000_base_device::m68k_op_muls_16_al, 0xf1ff, 0xc1f9, { 66,  44,  31,  31,  31,  31,  27}},
	{&m68000_base_device::m68k_op_muls_16_pcdi, 0xf1ff, 0xc1fa, { 62,  40,  32,  32,  32,  32,  27}},
	{&m68000_base_device::m68k_op_muls_16_pcix, 0xf1ff, 0xc1fb, { 64,  42,  34,  34,  34,  34,  27}},
	{&m68000_base_device::m68k_op_muls_16_i, 0xf1ff, 0xc1fc, { 58,  36,  29,  29,  29,  29,  27}},
	{&m68000_base_device::m68k_op_add_8_er_pi7, 0xf1ff, 0xd01f, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_8_er_pd7, 0xf1ff, 0xd027, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_8_er_aw, 0xf1ff, 0xd038, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_8_er_al, 0xf1ff, 0xd039, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_8_er_pcdi, 0xf1ff, 0xd03a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_8_er_pcix, 0xf1ff, 0xd03b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_add_8_er_i, 0xf1ff, 0xd03c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_add_16_er_aw, 0xf1ff, 0xd078, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_16_er_al, 0xf1ff, 0xd079, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_16_er_pcdi, 0xf1ff, 0xd07a, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_16_er_pcix, 0xf1ff, 0xd07b, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_add_16_er_i, 0xf1ff, 0xd07c, {  8,   8,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_add_32_er_aw, 0xf1ff, 0xd0b8, { 18,  18,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_32_er_al, 0xf1ff, 0xd0b9, { 22,  22,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_add_32_er_pcdi, 0xf1ff, 0xd0ba, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_add_32_er_pcix, 0xf1ff, 0xd0bb, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_add_32_er_i, 0xf1ff, 0xd0bc, { 16,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_16_aw, 0xf1ff, 0xd0f8, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_16_al, 0xf1ff, 0xd0f9, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_16_pcdi, 0xf1ff, 0xd0fa, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_adda_16_pcix, 0xf1ff, 0xd0fb, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_adda_16_i, 0xf1ff, 0xd0fc, { 12,  12,   4,   4,   4,   4,   2}},
	{&m68000_base_device::m68k_op_addx_8_mm_ay7, 0xf1ff, 0xd10f, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_add_8_re_pi7, 0xf1ff, 0xd11f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_8_re_pd7, 0xf1ff, 0xd127, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_add_8_re_aw, 0xf1ff, 0xd138, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_8_re_al, 0xf1ff, 0xd139, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_16_re_aw, 0xf1ff, 0xd178, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_16_re_al, 0xf1ff, 0xd179, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_32_re_aw, 0xf1ff, 0xd1b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_add_32_re_al, 0xf1ff, 0xd1b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_adda_32_aw, 0xf1ff, 0xd1f8, { 18,  18,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_32_al, 0xf1ff, 0xd1f9, { 22,  22,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_adda_32_pcdi, 0xf1ff, 0xd1fa, { 18,  18,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_adda_32_pcix, 0xf1ff, 0xd1fb, { 20,  20,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_adda_32_i, 0xf1ff, 0xd1fc, { 16,  14,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_ori_8_d, 0xfff8, 0x0000, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_ori_8_ai, 0xfff8, 0x0010, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_8_pi, 0xfff8, 0x0018, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_8_pd, 0xfff8, 0x0020, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_ori_8_di, 0xfff8, 0x0028, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_ori_8_ix, 0xfff8, 0x0030, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_ori_16_d, 0xfff8, 0x0040, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_ori_16_ai, 0xfff8, 0x0050, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_16_pi, 0xfff8, 0x0058, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_16_pd, 0xfff8, 0x0060, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_ori_16_di, 0xfff8, 0x0068, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_ori_16_ix, 0xfff8, 0x0070, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_ori_32_d, 0xfff8, 0x0080, { 16,  14,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_ori_32_ai, 0xfff8, 0x0090, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_32_pi, 0xfff8, 0x0098, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_32_pd, 0xfff8, 0x00a0, { 30,  30,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_ori_32_di, 0xfff8, 0x00a8, { 32,  32,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_ori_32_ix, 0xfff8, 0x00b0, { 34,  34,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_chk2cmp2_8_ai, 0xfff8, 0x00d0, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_8_di, 0xfff8, 0x00e8, {255, 255,  23,  23,  23,  23,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_8_ix, 0xfff8, 0x00f0, {255, 255,  25,  25,  25,  25,  18}},
	{&m68000_base_device::m68k_op_andi_8_d, 0xfff8, 0x0200, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_andi_8_ai, 0xfff8, 0x0210, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_8_pi, 0xfff8, 0x0218, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_8_pd, 0xfff8, 0x0220, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_andi_8_di, 0xfff8, 0x0228, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_andi_8_ix, 0xfff8, 0x0230, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_andi_16_d, 0xfff8, 0x0240, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_andi_16_ai, 0xfff8, 0x0250, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_16_pi, 0xfff8, 0x0258, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_16_pd, 0xfff8, 0x0260, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_andi_16_di, 0xfff8, 0x0268, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_andi_16_ix, 0xfff8, 0x0270, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_andi_32_d, 0xfff8, 0x0280, { 14,  14,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_andi_32_ai, 0xfff8, 0x0290, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_32_pi, 0xfff8, 0x0298, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_32_pd, 0xfff8, 0x02a0, { 30,  30,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_andi_32_di, 0xfff8, 0x02a8, { 32,  32,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_andi_32_ix, 0xfff8, 0x02b0, { 34,  34,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_chk2cmp2_16_ai, 0xfff8, 0x02d0, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_16_di, 0xfff8, 0x02e8, {255, 255,  23,  23,  23,  23,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_16_ix, 0xfff8, 0x02f0, {255, 255,  25,  25,  25,  25,  18}},
	{&m68000_base_device::m68k_op_subi_8_d, 0xfff8, 0x0400, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subi_8_ai, 0xfff8, 0x0410, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_8_pi, 0xfff8, 0x0418, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_8_pd, 0xfff8, 0x0420, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subi_8_di, 0xfff8, 0x0428, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subi_8_ix, 0xfff8, 0x0430, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_subi_16_d, 0xfff8, 0x0440, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subi_16_ai, 0xfff8, 0x0450, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_16_pi, 0xfff8, 0x0458, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_16_pd, 0xfff8, 0x0460, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subi_16_di, 0xfff8, 0x0468, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subi_16_ix, 0xfff8, 0x0470, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_subi_32_d, 0xfff8, 0x0480, { 16,  14,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_subi_32_ai, 0xfff8, 0x0490, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_32_pi, 0xfff8, 0x0498, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_32_pd, 0xfff8, 0x04a0, { 30,  30,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subi_32_di, 0xfff8, 0x04a8, { 32,  32,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subi_32_ix, 0xfff8, 0x04b0, { 34,  34,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_chk2cmp2_32_ai, 0xfff8, 0x04d0, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_32_di, 0xfff8, 0x04e8, {255, 255,  23,  23,  23,  23,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_32_ix, 0xfff8, 0x04f0, {255, 255,  25,  25,  25,  25,  18}},
	{&m68000_base_device::m68k_op_addi_8_d, 0xfff8, 0x0600, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addi_8_ai, 0xfff8, 0x0610, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_8_pi, 0xfff8, 0x0618, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_8_pd, 0xfff8, 0x0620, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addi_8_di, 0xfff8, 0x0628, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addi_8_ix, 0xfff8, 0x0630, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_addi_16_d, 0xfff8, 0x0640, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addi_16_ai, 0xfff8, 0x0650, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_16_pi, 0xfff8, 0x0658, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_16_pd, 0xfff8, 0x0660, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addi_16_di, 0xfff8, 0x0668, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addi_16_ix, 0xfff8, 0x0670, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_addi_32_d, 0xfff8, 0x0680, { 16,  14,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_addi_32_ai, 0xfff8, 0x0690, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_32_pi, 0xfff8, 0x0698, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_32_pd, 0xfff8, 0x06a0, { 30,  30,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addi_32_di, 0xfff8, 0x06a8, { 32,  32,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addi_32_ix, 0xfff8, 0x06b0, { 34,  34,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_callm_32_ai, 0xfff8, 0x06d0, {255, 255,  64,  64,  64,  64,  60}},
	{&m68000_base_device::m68k_op_callm_32_di, 0xfff8, 0x06e8, {255, 255,  65,  65,  65,  65,  60}},
	{&m68000_base_device::m68k_op_callm_32_ix, 0xfff8, 0x06f0, {255, 255,  67,  67,  67,  67,  60}},
	{&m68000_base_device::m68k_op_btst_32_s_d, 0xfff8, 0x0800, { 10,  10,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_ai, 0xfff8, 0x0810, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_pi, 0xfff8, 0x0818, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_pd, 0xfff8, 0x0820, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_di, 0xfff8, 0x0828, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_ix, 0xfff8, 0x0830, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_bchg_32_s_d, 0xfff8, 0x0840, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_ai, 0xfff8, 0x0850, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_pi, 0xfff8, 0x0858, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_pd, 0xfff8, 0x0860, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_di, 0xfff8, 0x0868, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_ix, 0xfff8, 0x0870, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_bclr_32_s_d, 0xfff8, 0x0880, { 14,  14,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_ai, 0xfff8, 0x0890, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_pi, 0xfff8, 0x0898, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_pd, 0xfff8, 0x08a0, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_di, 0xfff8, 0x08a8, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_ix, 0xfff8, 0x08b0, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_bset_32_s_d, 0xfff8, 0x08c0, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_ai, 0xfff8, 0x08d0, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_pi, 0xfff8, 0x08d8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_pd, 0xfff8, 0x08e0, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_di, 0xfff8, 0x08e8, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_ix, 0xfff8, 0x08f0, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_eori_8_d, 0xfff8, 0x0a00, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_eori_8_ai, 0xfff8, 0x0a10, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_8_pi, 0xfff8, 0x0a18, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_8_pd, 0xfff8, 0x0a20, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eori_8_di, 0xfff8, 0x0a28, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eori_8_ix, 0xfff8, 0x0a30, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_eori_16_d, 0xfff8, 0x0a40, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_eori_16_ai, 0xfff8, 0x0a50, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_16_pi, 0xfff8, 0x0a58, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_16_pd, 0xfff8, 0x0a60, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eori_16_di, 0xfff8, 0x0a68, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eori_16_ix, 0xfff8, 0x0a70, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_eori_32_d, 0xfff8, 0x0a80, { 16,  14,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_eori_32_ai, 0xfff8, 0x0a90, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_32_pi, 0xfff8, 0x0a98, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_32_pd, 0xfff8, 0x0aa0, { 30,  30,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eori_32_di, 0xfff8, 0x0aa8, { 32,  32,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eori_32_ix, 0xfff8, 0x0ab0, { 34,  34,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_cas_8_ai, 0xfff8, 0x0ad0, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_8_pi, 0xfff8, 0x0ad8, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_8_pd, 0xfff8, 0x0ae0, {255, 255,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_cas_8_di, 0xfff8, 0x0ae8, {255, 255,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_cas_8_ix, 0xfff8, 0x0af0, {255, 255,  19,  19,  19,  19,  12}},
	{&m68000_base_device::m68k_op_cmpi_8_d, 0xfff8, 0x0c00, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_ai, 0xfff8, 0x0c10, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_pi, 0xfff8, 0x0c18, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_pd, 0xfff8, 0x0c20, { 14,  14,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_di, 0xfff8, 0x0c28, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_ix, 0xfff8, 0x0c30, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmpi_16_d, 0xfff8, 0x0c40, {  8,   8,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmpi_16_ai, 0xfff8, 0x0c50, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_16_pi, 0xfff8, 0x0c58, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_16_pd, 0xfff8, 0x0c60, { 14,  14,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmpi_16_di, 0xfff8, 0x0c68, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmpi_16_ix, 0xfff8, 0x0c70, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cmpi_32_d, 0xfff8, 0x0c80, { 14,  12,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_cmpi_32_ai, 0xfff8, 0x0c90, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_32_pi, 0xfff8, 0x0c98, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_32_pd, 0xfff8, 0x0ca0, { 22,  22,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmpi_32_di, 0xfff8, 0x0ca8, { 24,  24,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmpi_32_ix, 0xfff8, 0x0cb0, { 26,  26,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_cas_16_ai, 0xfff8, 0x0cd0, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_16_pi, 0xfff8, 0x0cd8, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_16_pd, 0xfff8, 0x0ce0, {255, 255,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_cas_16_di, 0xfff8, 0x0ce8, {255, 255,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_cas_16_ix, 0xfff8, 0x0cf0, {255, 255,  19,  19,  19,  19,  12}},
	{&m68000_base_device::m68k_op_moves_8_ai, 0xfff8, 0x0e10, {255,  18,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_8_pi, 0xfff8, 0x0e18, {255,  18,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_8_pd, 0xfff8, 0x0e20, {255,  20,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_moves_8_di, 0xfff8, 0x0e28, {255,  26,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_moves_8_ix, 0xfff8, 0x0e30, {255,  30,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_moves_16_ai, 0xfff8, 0x0e50, {255,  18,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_16_pi, 0xfff8, 0x0e58, {255,  18,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_16_pd, 0xfff8, 0x0e60, {255,  20,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_moves_16_di, 0xfff8, 0x0e68, {255,  26,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_moves_16_ix, 0xfff8, 0x0e70, {255,  30,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_moves_32_ai, 0xfff8, 0x0e90, {255,  22,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_32_pi, 0xfff8, 0x0e98, {255,  22,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_32_pd, 0xfff8, 0x0ea0, {255,  28,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_moves_32_di, 0xfff8, 0x0ea8, {255,  32,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_moves_32_ix, 0xfff8, 0x0eb0, {255,  36,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_cas_32_ai, 0xfff8, 0x0ed0, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_32_pi, 0xfff8, 0x0ed8, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_32_pd, 0xfff8, 0x0ee0, {255, 255,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_cas_32_di, 0xfff8, 0x0ee8, {255, 255,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_cas_32_ix, 0xfff8, 0x0ef0, {255, 255,  19,  19,  19,  19,  12}},
	{&m68000_base_device::m68k_op_move_8_aw_d, 0xfff8, 0x11c0, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_ai, 0xfff8, 0x11d0, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_pi, 0xfff8, 0x11d8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_pd, 0xfff8, 0x11e0, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_di, 0xfff8, 0x11e8, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_ix, 0xfff8, 0x11f0, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_al_d, 0xfff8, 0x13c0, { 16,  16,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_move_8_al_ai, 0xfff8, 0x13d0, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_8_al_pi, 0xfff8, 0x13d8, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_8_al_pd, 0xfff8, 0x13e0, { 22,  22,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_8_al_di, 0xfff8, 0x13e8, { 24,  24,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_8_al_ix, 0xfff8, 0x13f0, { 26,  26,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_move_8_pi7_d, 0xfff8, 0x1ec0, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_ai, 0xfff8, 0x1ed0, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_pi, 0xfff8, 0x1ed8, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_pd, 0xfff8, 0x1ee0, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_di, 0xfff8, 0x1ee8, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_ix, 0xfff8, 0x1ef0, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_pd7_d, 0xfff8, 0x1f00, {  8,   8,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_ai, 0xfff8, 0x1f10, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_pi, 0xfff8, 0x1f18, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_pd, 0xfff8, 0x1f20, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_di, 0xfff8, 0x1f28, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_ix, 0xfff8, 0x1f30, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_32_aw_d, 0xfff8, 0x21c0, { 16,  16,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_a, 0xfff8, 0x21c8, { 16,  16,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_ai, 0xfff8, 0x21d0, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_pi, 0xfff8, 0x21d8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_pd, 0xfff8, 0x21e0, { 26,  26,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_di, 0xfff8, 0x21e8, { 28,  28,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_ix, 0xfff8, 0x21f0, { 30,  30,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_32_al_d, 0xfff8, 0x23c0, { 20,  20,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_move_32_al_a, 0xfff8, 0x23c8, { 20,  20,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_move_32_al_ai, 0xfff8, 0x23d0, { 28,  28,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_32_al_pi, 0xfff8, 0x23d8, { 28,  28,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_32_al_pd, 0xfff8, 0x23e0, { 30,  30,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_32_al_di, 0xfff8, 0x23e8, { 32,  32,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_32_al_ix, 0xfff8, 0x23f0, { 34,  34,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_move_16_aw_d, 0xfff8, 0x31c0, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_a, 0xfff8, 0x31c8, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_ai, 0xfff8, 0x31d0, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_pi, 0xfff8, 0x31d8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_pd, 0xfff8, 0x31e0, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_di, 0xfff8, 0x31e8, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_ix, 0xfff8, 0x31f0, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_al_d, 0xfff8, 0x33c0, { 16,  16,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_move_16_al_a, 0xfff8, 0x33c8, { 16,  16,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_move_16_al_ai, 0xfff8, 0x33d0, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_16_al_pi, 0xfff8, 0x33d8, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_16_al_pd, 0xfff8, 0x33e0, { 22,  22,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_16_al_di, 0xfff8, 0x33e8, { 24,  24,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_16_al_ix, 0xfff8, 0x33f0, { 26,  26,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_negx_8_d, 0xfff8, 0x4000, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_negx_8_ai, 0xfff8, 0x4010, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_8_pi, 0xfff8, 0x4018, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_8_pd, 0xfff8, 0x4020, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_negx_8_di, 0xfff8, 0x4028, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_negx_8_ix, 0xfff8, 0x4030, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_negx_16_d, 0xfff8, 0x4040, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_negx_16_ai, 0xfff8, 0x4050, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_16_pi, 0xfff8, 0x4058, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_16_pd, 0xfff8, 0x4060, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_negx_16_di, 0xfff8, 0x4068, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_negx_16_ix, 0xfff8, 0x4070, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_negx_32_d, 0xfff8, 0x4080, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_negx_32_ai, 0xfff8, 0x4090, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_32_pi, 0xfff8, 0x4098, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_32_pd, 0xfff8, 0x40a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_negx_32_di, 0xfff8, 0x40a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_negx_32_ix, 0xfff8, 0x40b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_frs_d, 0xfff8, 0x40c0, {  6,   4,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_move_16_frs_ai, 0xfff8, 0x40d0, { 12,  12,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_move_16_frs_pi, 0xfff8, 0x40d8, { 12,  12,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_move_16_frs_pd, 0xfff8, 0x40e0, { 14,  14,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_move_16_frs_di, 0xfff8, 0x40e8, { 16,  16,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_move_16_frs_ix, 0xfff8, 0x40f0, { 18,  18,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_clr_8_d, 0xfff8, 0x4200, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_clr_8_ai, 0xfff8, 0x4210, { 12,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_8_pi, 0xfff8, 0x4218, { 12,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_8_pd, 0xfff8, 0x4220, { 14,  10,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_clr_8_di, 0xfff8, 0x4228, { 16,  12,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_clr_8_ix, 0xfff8, 0x4230, { 18,  14,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_clr_16_d, 0xfff8, 0x4240, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_clr_16_ai, 0xfff8, 0x4250, { 12,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_16_pi, 0xfff8, 0x4258, { 12,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_16_pd, 0xfff8, 0x4260, { 14,  10,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_clr_16_di, 0xfff8, 0x4268, { 16,  12,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_clr_16_ix, 0xfff8, 0x4270, { 18,  14,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_clr_32_d, 0xfff8, 0x4280, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_clr_32_ai, 0xfff8, 0x4290, { 20,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_32_pi, 0xfff8, 0x4298, { 20,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_32_pd, 0xfff8, 0x42a0, { 22,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_clr_32_di, 0xfff8, 0x42a8, { 24,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_clr_32_ix, 0xfff8, 0x42b0, { 26,  20,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_frc_d, 0xfff8, 0x42c0, {255,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_16_frc_ai, 0xfff8, 0x42d0, {255,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_frc_pi, 0xfff8, 0x42d8, {255,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_frc_pd, 0xfff8, 0x42e0, {255,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_frc_di, 0xfff8, 0x42e8, {255,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_frc_ix, 0xfff8, 0x42f0, {255,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_neg_8_d, 0xfff8, 0x4400, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_neg_8_ai, 0xfff8, 0x4410, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_8_pi, 0xfff8, 0x4418, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_8_pd, 0xfff8, 0x4420, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_neg_8_di, 0xfff8, 0x4428, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_neg_8_ix, 0xfff8, 0x4430, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_neg_16_d, 0xfff8, 0x4440, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_neg_16_ai, 0xfff8, 0x4450, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_16_pi, 0xfff8, 0x4458, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_16_pd, 0xfff8, 0x4460, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_neg_16_di, 0xfff8, 0x4468, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_neg_16_ix, 0xfff8, 0x4470, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_neg_32_d, 0xfff8, 0x4480, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_neg_32_ai, 0xfff8, 0x4490, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_32_pi, 0xfff8, 0x4498, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_32_pd, 0xfff8, 0x44a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_neg_32_di, 0xfff8, 0x44a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_neg_32_ix, 0xfff8, 0x44b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_d, 0xfff8, 0x44c0, { 12,  12,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_ai, 0xfff8, 0x44d0, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_pi, 0xfff8, 0x44d8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_pd, 0xfff8, 0x44e0, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_di, 0xfff8, 0x44e8, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_ix, 0xfff8, 0x44f0, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_not_8_d, 0xfff8, 0x4600, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_not_8_ai, 0xfff8, 0x4610, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_8_pi, 0xfff8, 0x4618, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_8_pd, 0xfff8, 0x4620, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_not_8_di, 0xfff8, 0x4628, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_not_8_ix, 0xfff8, 0x4630, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_not_16_d, 0xfff8, 0x4640, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_not_16_ai, 0xfff8, 0x4650, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_16_pi, 0xfff8, 0x4658, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_16_pd, 0xfff8, 0x4660, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_not_16_di, 0xfff8, 0x4668, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_not_16_ix, 0xfff8, 0x4670, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_not_32_d, 0xfff8, 0x4680, {  6,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_not_32_ai, 0xfff8, 0x4690, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_32_pi, 0xfff8, 0x4698, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_32_pd, 0xfff8, 0x46a0, { 22,  22,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_not_32_di, 0xfff8, 0x46a8, { 24,  24,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_not_32_ix, 0xfff8, 0x46b0, { 26,  26,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_tos_d, 0xfff8, 0x46c0, { 12,  12,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_ai, 0xfff8, 0x46d0, { 16,  16,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_pi, 0xfff8, 0x46d8, { 16,  16,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_pd, 0xfff8, 0x46e0, { 18,  18,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_di, 0xfff8, 0x46e8, { 20,  20,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_ix, 0xfff8, 0x46f0, { 22,  22,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_nbcd_8_d, 0xfff8, 0x4800, {  6,   6,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_link_32, 0xfff8, 0x4808, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_ai, 0xfff8, 0x4810, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_pi, 0xfff8, 0x4818, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_pd, 0xfff8, 0x4820, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_di, 0xfff8, 0x4828, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_ix, 0xfff8, 0x4830, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_swap_32, 0xfff8, 0x4840, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_bkpt, 0xfff8, 0x4848, {255,  10,  10,  10,  10,  10,  10}},
	{&m68000_base_device::m68k_op_pea_32_ai, 0xfff8, 0x4850, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_pea_32_di, 0xfff8, 0x4868, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_pea_32_ix, 0xfff8, 0x4870, { 20,  20,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_ext_16, 0xfff8, 0x4880, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_movem_16_re_ai, 0xfff8, 0x4890, {  8,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_movem_16_re_pd, 0xfff8, 0x48a0, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_movem_16_re_di, 0xfff8, 0x48a8, { 12,  12,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_movem_16_re_ix, 0xfff8, 0x48b0, { 14,  14,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_ext_32, 0xfff8, 0x48c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_movem_32_re_ai, 0xfff8, 0x48d0, {  8,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_movem_32_re_pd, 0xfff8, 0x48e0, {  8,   8,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_movem_32_re_di, 0xfff8, 0x48e8, { 12,  12,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_movem_32_re_ix, 0xfff8, 0x48f0, { 14,  14,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_extb_32, 0xfff8, 0x49c0, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_tst_8_d, 0xfff8, 0x4a00, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_tst_8_ai, 0xfff8, 0x4a10, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_8_pi, 0xfff8, 0x4a18, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_8_pd, 0xfff8, 0x4a20, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_tst_8_di, 0xfff8, 0x4a28, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_tst_8_ix, 0xfff8, 0x4a30, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_tst_16_d, 0xfff8, 0x4a40, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_tst_16_a, 0xfff8, 0x4a48, {255, 255,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_tst_16_ai, 0xfff8, 0x4a50, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_16_pi, 0xfff8, 0x4a58, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_16_pd, 0xfff8, 0x4a60, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_tst_16_di, 0xfff8, 0x4a68, { 12,  12,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_tst_16_ix, 0xfff8, 0x4a70, { 14,  14,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_tst_32_d, 0xfff8, 0x4a80, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_tst_32_a, 0xfff8, 0x4a88, {255, 255,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_tst_32_ai, 0xfff8, 0x4a90, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_32_pi, 0xfff8, 0x4a98, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_32_pd, 0xfff8, 0x4aa0, { 14,  14,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_tst_32_di, 0xfff8, 0x4aa8, { 16,  16,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_tst_32_ix, 0xfff8, 0x4ab0, { 18,  18,   9,   9,   9,   9,   2}},
	{&m68000_base_device::m68k_op_tas_8_d, 0xfff8, 0x4ac0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_tas_8_ai, 0xfff8, 0x4ad0, { 18,  18,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_tas_8_pi, 0xfff8, 0x4ad8, { 18,  18,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_tas_8_pd, 0xfff8, 0x4ae0, { 20,  20,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_tas_8_di, 0xfff8, 0x4ae8, { 22,  22,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_tas_8_ix, 0xfff8, 0x4af0, { 24,  24,  19,  19,  19,  19,  12}},
	{&m68000_base_device::m68k_op_mull_32_d, 0xfff8, 0x4c00, {255, 255,  43,  43,  43,  43,  43}},
	{&m68000_base_device::m68k_op_mull_32_ai, 0xfff8, 0x4c10, {255, 255,  47,  47,  47,  47,  43}},
	{&m68000_base_device::m68k_op_mull_32_pi, 0xfff8, 0x4c18, {255, 255,  47,  47,  47,  47,  43}},
	{&m68000_base_device::m68k_op_mull_32_pd, 0xfff8, 0x4c20, {255, 255,  48,  48,  48,  48,  43}},
	{&m68000_base_device::m68k_op_mull_32_di, 0xfff8, 0x4c28, {255, 255,  48,  48,  48,  48,  43}},
	{&m68000_base_device::m68k_op_mull_32_ix, 0xfff8, 0x4c30, {255, 255,  50,  50,  50,  50,  43}},
	{&m68000_base_device::m68k_op_divl_32_d, 0xfff8, 0x4c40, {255, 255,  84,  84,  84,  84,  84}},
	{&m68000_base_device::m68k_op_divl_32_ai, 0xfff8, 0x4c50, {255, 255,  88,  88,  88,  88,  84}},
	{&m68000_base_device::m68k_op_divl_32_pi, 0xfff8, 0x4c58, {255, 255,  88,  88,  88,  88,  84}},
	{&m68000_base_device::m68k_op_divl_32_pd, 0xfff8, 0x4c60, {255, 255,  89,  89,  89,  89,  84}},
	{&m68000_base_device::m68k_op_divl_32_di, 0xfff8, 0x4c68, {255, 255,  89,  89,  89,  89,  84}},
	{&m68000_base_device::m68k_op_divl_32_ix, 0xfff8, 0x4c70, {255, 255,  91,  91,  91,  91,  84}},
	{&m68000_base_device::m68k_op_movem_16_er_ai, 0xfff8, 0x4c90, { 12,  12,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_movem_16_er_pi, 0xfff8, 0x4c98, { 12,  12,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_movem_16_er_di, 0xfff8, 0x4ca8, { 16,  16,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_movem_16_er_ix, 0xfff8, 0x4cb0, { 18,  18,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_movem_32_er_ai, 0xfff8, 0x4cd0, { 12,  12,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_movem_32_er_pi, 0xfff8, 0x4cd8, { 12,  12,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_movem_32_er_di, 0xfff8, 0x4ce8, { 16,  16,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_movem_32_er_ix, 0xfff8, 0x4cf0, { 18,  18,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_link_16, 0xfff8, 0x4e50, { 16,  16,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_unlk_32, 0xfff8, 0x4e58, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_move_32_tou, 0xfff8, 0x4e60, {  4,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_move_32_fru, 0xfff8, 0x4e68, {  4,   6,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_jsr_32_ai, 0xfff8, 0x4e90, { 16,  16,   4,   4,   4,   4,   0}},
	{&m68000_base_device::m68k_op_jsr_32_di, 0xfff8, 0x4ea8, { 18,  18,   5,   5,   5,   5,   0}},
	{&m68000_base_device::m68k_op_jsr_32_ix, 0xfff8, 0x4eb0, { 22,  22,   7,   7,   7,   7,   0}},
	{&m68000_base_device::m68k_op_jmp_32_ai, 0xfff8, 0x4ed0, {  8,   8,   4,   4,   4,   4,   0}},
	{&m68000_base_device::m68k_op_jmp_32_di, 0xfff8, 0x4ee8, { 10,  10,   5,   5,   5,   5,   0}},
	{&m68000_base_device::m68k_op_jmp_32_ix, 0xfff8, 0x4ef0, { 14,  14,   7,   7,   7,   7,   0}},
	{&m68000_base_device::m68k_op_st_8_d, 0xfff8, 0x50c0, {  6,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbt_16, 0xfff8, 0x50c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_st_8_ai, 0xfff8, 0x50d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_st_8_pi, 0xfff8, 0x50d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_st_8_pd, 0xfff8, 0x50e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_st_8_di, 0xfff8, 0x50e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_st_8_ix, 0xfff8, 0x50f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_sf_8_d, 0xfff8, 0x51c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbf_16, 0xfff8, 0x51c8, { 12,  12,   6,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_sf_8_ai, 0xfff8, 0x51d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sf_8_pi, 0xfff8, 0x51d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sf_8_pd, 0xfff8, 0x51e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sf_8_di, 0xfff8, 0x51e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sf_8_ix, 0xfff8, 0x51f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_shi_8_d, 0xfff8, 0x52c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbhi_16, 0xfff8, 0x52c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_shi_8_ai, 0xfff8, 0x52d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_shi_8_pi, 0xfff8, 0x52d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_shi_8_pd, 0xfff8, 0x52e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_shi_8_di, 0xfff8, 0x52e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_shi_8_ix, 0xfff8, 0x52f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_sls_8_d, 0xfff8, 0x53c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbls_16, 0xfff8, 0x53c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_sls_8_ai, 0xfff8, 0x53d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sls_8_pi, 0xfff8, 0x53d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sls_8_pd, 0xfff8, 0x53e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sls_8_di, 0xfff8, 0x53e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sls_8_ix, 0xfff8, 0x53f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_scc_8_d, 0xfff8, 0x54c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbcc_16, 0xfff8, 0x54c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_scc_8_ai, 0xfff8, 0x54d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_scc_8_pi, 0xfff8, 0x54d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_scc_8_pd, 0xfff8, 0x54e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_scc_8_di, 0xfff8, 0x54e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_scc_8_ix, 0xfff8, 0x54f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_scs_8_d, 0xfff8, 0x55c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbcs_16, 0xfff8, 0x55c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_scs_8_ai, 0xfff8, 0x55d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_scs_8_pi, 0xfff8, 0x55d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_scs_8_pd, 0xfff8, 0x55e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_scs_8_di, 0xfff8, 0x55e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_scs_8_ix, 0xfff8, 0x55f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_sne_8_d, 0xfff8, 0x56c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbne_16, 0xfff8, 0x56c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_sne_8_ai, 0xfff8, 0x56d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sne_8_pi, 0xfff8, 0x56d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sne_8_pd, 0xfff8, 0x56e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sne_8_di, 0xfff8, 0x56e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sne_8_ix, 0xfff8, 0x56f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_seq_8_d, 0xfff8, 0x57c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbeq_16, 0xfff8, 0x57c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_seq_8_ai, 0xfff8, 0x57d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_seq_8_pi, 0xfff8, 0x57d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_seq_8_pd, 0xfff8, 0x57e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_seq_8_di, 0xfff8, 0x57e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_seq_8_ix, 0xfff8, 0x57f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_svc_8_d, 0xfff8, 0x58c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbvc_16, 0xfff8, 0x58c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_svc_8_ai, 0xfff8, 0x58d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_svc_8_pi, 0xfff8, 0x58d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_svc_8_pd, 0xfff8, 0x58e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_svc_8_di, 0xfff8, 0x58e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_svc_8_ix, 0xfff8, 0x58f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_svs_8_d, 0xfff8, 0x59c0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbvs_16, 0xfff8, 0x59c8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_svs_8_ai, 0xfff8, 0x59d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_svs_8_pi, 0xfff8, 0x59d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_svs_8_pd, 0xfff8, 0x59e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_svs_8_di, 0xfff8, 0x59e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_svs_8_ix, 0xfff8, 0x59f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_spl_8_d, 0xfff8, 0x5ac0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbpl_16, 0xfff8, 0x5ac8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_spl_8_ai, 0xfff8, 0x5ad0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_spl_8_pi, 0xfff8, 0x5ad8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_spl_8_pd, 0xfff8, 0x5ae0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_spl_8_di, 0xfff8, 0x5ae8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_spl_8_ix, 0xfff8, 0x5af0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_smi_8_d, 0xfff8, 0x5bc0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbmi_16, 0xfff8, 0x5bc8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_smi_8_ai, 0xfff8, 0x5bd0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_smi_8_pi, 0xfff8, 0x5bd8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_smi_8_pd, 0xfff8, 0x5be0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_smi_8_di, 0xfff8, 0x5be8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_smi_8_ix, 0xfff8, 0x5bf0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_sge_8_d, 0xfff8, 0x5cc0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbge_16, 0xfff8, 0x5cc8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_sge_8_ai, 0xfff8, 0x5cd0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sge_8_pi, 0xfff8, 0x5cd8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sge_8_pd, 0xfff8, 0x5ce0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sge_8_di, 0xfff8, 0x5ce8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sge_8_ix, 0xfff8, 0x5cf0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_slt_8_d, 0xfff8, 0x5dc0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dblt_16, 0xfff8, 0x5dc8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_slt_8_ai, 0xfff8, 0x5dd0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_slt_8_pi, 0xfff8, 0x5dd8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_slt_8_pd, 0xfff8, 0x5de0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_slt_8_di, 0xfff8, 0x5de8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_slt_8_ix, 0xfff8, 0x5df0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_sgt_8_d, 0xfff8, 0x5ec0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dbgt_16, 0xfff8, 0x5ec8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_sgt_8_ai, 0xfff8, 0x5ed0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sgt_8_pi, 0xfff8, 0x5ed8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sgt_8_pd, 0xfff8, 0x5ee0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sgt_8_di, 0xfff8, 0x5ee8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sgt_8_ix, 0xfff8, 0x5ef0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_sle_8_d, 0xfff8, 0x5fc0, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_dble_16, 0xfff8, 0x5fc8, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_sle_8_ai, 0xfff8, 0x5fd0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sle_8_pi, 0xfff8, 0x5fd8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sle_8_pd, 0xfff8, 0x5fe0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sle_8_di, 0xfff8, 0x5fe8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sle_8_ix, 0xfff8, 0x5ff0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_sbcd_8_mm_ax7, 0xfff8, 0x8f08, { 18,  18,  16,  16,  16,  16,  16}},
	{&m68000_base_device::m68k_op_pack_16_mm_ax7, 0xfff8, 0x8f48, {255, 255,  13,  13,  13,  13,  13}},
	{&m68000_base_device::m68k_op_unpk_16_mm_ax7, 0xfff8, 0x8f88, {255, 255,  13,  13,  13,  13,  13}},
	{&m68000_base_device::m68k_op_subx_8_mm_ax7, 0xfff8, 0x9f08, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_cmpm_8_ax7, 0xfff8, 0xbf08, { 12,  12,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_abcd_8_mm_ax7, 0xfff8, 0xcf08, { 18,  18,  16,  16,  16,  16,  16}},
	{&m68000_base_device::m68k_op_addx_8_mm_ax7, 0xfff8, 0xdf08, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_asr_16_ai, 0xfff8, 0xe0d0, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_asr_16_pi, 0xfff8, 0xe0d8, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_asr_16_pd, 0xfff8, 0xe0e0, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_asr_16_di, 0xfff8, 0xe0e8, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_asr_16_ix, 0xfff8, 0xe0f0, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_asl_16_ai, 0xfff8, 0xe1d0, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_asl_16_pi, 0xfff8, 0xe1d8, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_asl_16_pd, 0xfff8, 0xe1e0, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_asl_16_di, 0xfff8, 0xe1e8, { 16,  16,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_asl_16_ix, 0xfff8, 0xe1f0, { 18,  18,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_lsr_16_ai, 0xfff8, 0xe2d0, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_lsr_16_pi, 0xfff8, 0xe2d8, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_lsr_16_pd, 0xfff8, 0xe2e0, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_lsr_16_di, 0xfff8, 0xe2e8, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_lsr_16_ix, 0xfff8, 0xe2f0, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_lsl_16_ai, 0xfff8, 0xe3d0, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_lsl_16_pi, 0xfff8, 0xe3d8, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_lsl_16_pd, 0xfff8, 0xe3e0, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_lsl_16_di, 0xfff8, 0xe3e8, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_lsl_16_ix, 0xfff8, 0xe3f0, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_roxr_16_ai, 0xfff8, 0xe4d0, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_roxr_16_pi, 0xfff8, 0xe4d8, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_roxr_16_pd, 0xfff8, 0xe4e0, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_roxr_16_di, 0xfff8, 0xe4e8, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_roxr_16_ix, 0xfff8, 0xe4f0, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_roxl_16_ai, 0xfff8, 0xe5d0, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_roxl_16_pi, 0xfff8, 0xe5d8, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_roxl_16_pd, 0xfff8, 0xe5e0, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_roxl_16_di, 0xfff8, 0xe5e8, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_roxl_16_ix, 0xfff8, 0xe5f0, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_ror_16_ai, 0xfff8, 0xe6d0, { 12,  12,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_ror_16_pi, 0xfff8, 0xe6d8, { 12,  12,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_ror_16_pd, 0xfff8, 0xe6e0, { 14,  14,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_ror_16_di, 0xfff8, 0xe6e8, { 16,  16,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_ror_16_ix, 0xfff8, 0xe6f0, { 18,  18,  14,  14,  14,  14,   7}},
	{&m68000_base_device::m68k_op_rol_16_ai, 0xfff8, 0xe7d0, { 12,  12,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_rol_16_pi, 0xfff8, 0xe7d8, { 12,  12,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_rol_16_pd, 0xfff8, 0xe7e0, { 14,  14,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_rol_16_di, 0xfff8, 0xe7e8, { 16,  16,  12,  12,  12,  12,   7}},
	{&m68000_base_device::m68k_op_rol_16_ix, 0xfff8, 0xe7f0, { 18,  18,  14,  14,  14,  14,   7}},
	{&m68000_base_device::m68k_op_bftst_32_d, 0xfff8, 0xe8c0, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bftst_32_ai, 0xfff8, 0xe8d0, {255, 255,  17,  17,  17,  17,  13}},
	{&m68000_base_device::m68k_op_bftst_32_di, 0xfff8, 0xe8e8, {255, 255,  18,  18,  18,  18,  13}},
	{&m68000_base_device::m68k_op_bftst_32_ix, 0xfff8, 0xe8f0, {255, 255,  20,  20,  20,  20,  13}},
	{&m68000_base_device::m68k_op_bfextu_32_d, 0xfff8, 0xe9c0, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_bfextu_32_ai, 0xfff8, 0xe9d0, {255, 255,  19,  19,  19,  19,  15}},
	{&m68000_base_device::m68k_op_bfextu_32_di, 0xfff8, 0xe9e8, {255, 255,  20,  20,  20,  20,  15}},
	{&m68000_base_device::m68k_op_bfextu_32_ix, 0xfff8, 0xe9f0, {255, 255,  22,  22,  22,  22,  15}},
	{&m68000_base_device::m68k_op_bfchg_32_d, 0xfff8, 0xeac0, {255, 255,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_bfchg_32_ai, 0xfff8, 0xead0, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfchg_32_di, 0xfff8, 0xeae8, {255, 255,  25,  25,  25,  25,  20}},
	{&m68000_base_device::m68k_op_bfchg_32_ix, 0xfff8, 0xeaf0, {255, 255,  27,  27,  27,  27,  20}},
	{&m68000_base_device::m68k_op_bfexts_32_d, 0xfff8, 0xebc0, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_bfexts_32_ai, 0xfff8, 0xebd0, {255, 255,  19,  19,  19,  19,  15}},
	{&m68000_base_device::m68k_op_bfexts_32_di, 0xfff8, 0xebe8, {255, 255,  20,  20,  20,  20,  15}},
	{&m68000_base_device::m68k_op_bfexts_32_ix, 0xfff8, 0xebf0, {255, 255,  22,  22,  22,  22,  15}},
	{&m68000_base_device::m68k_op_bfclr_32_d, 0xfff8, 0xecc0, {255, 255,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_bfclr_32_ai, 0xfff8, 0xecd0, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfclr_32_di, 0xfff8, 0xece8, {255, 255,  25,  25,  25,  25,  20}},
	{&m68000_base_device::m68k_op_bfclr_32_ix, 0xfff8, 0xecf0, {255, 255,  27,  27,  27,  27,  20}},
	{&m68000_base_device::m68k_op_bfffo_32_d, 0xfff8, 0xedc0, {255, 255,  18,  18,  18,  18,  18}},
	{&m68000_base_device::m68k_op_bfffo_32_ai, 0xfff8, 0xedd0, {255, 255,  32,  32,  32,  32,  28}},
	{&m68000_base_device::m68k_op_bfffo_32_di, 0xfff8, 0xede8, {255, 255,  33,  33,  33,  33,  28}},
	{&m68000_base_device::m68k_op_bfffo_32_ix, 0xfff8, 0xedf0, {255, 255,  35,  35,  35,  35,  28}},
	{&m68000_base_device::m68k_op_bfset_32_d, 0xfff8, 0xeec0, {255, 255,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_bfset_32_ai, 0xfff8, 0xeed0, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfset_32_di, 0xfff8, 0xeee8, {255, 255,  25,  25,  25,  25,  20}},
	{&m68000_base_device::m68k_op_bfset_32_ix, 0xfff8, 0xeef0, {255, 255,  27,  27,  27,  27,  20}},
	{&m68000_base_device::m68k_op_bfins_32_d, 0xfff8, 0xefc0, {255, 255,  10,  10,  10,  10,  10}},
	{&m68000_base_device::m68k_op_bfins_32_ai, 0xfff8, 0xefd0, {255, 255,  21,  21,  21,  21,  17}},
	{&m68000_base_device::m68k_op_bfins_32_di, 0xfff8, 0xefe8, {255, 255,  22,  22,  22,  22,  17}},
	{&m68000_base_device::m68k_op_bfins_32_ix, 0xfff8, 0xeff0, {255, 255,  24,  24,  24,  24,  17}},
	{&m68000_base_device::m68k_op_ftrapcc_32, 0xfff8, 0xf278, {255, 255,   4,   4, 255, 255, 255}},
	{&m68000_base_device::m68k_op_pflushan_32, 0xfff8, 0xf510, {255, 255, 255, 255,   4,   4,   4}},
	{&m68000_base_device::m68k_op_pflusha_32, 0xfff8, 0xf518, {255, 255, 255, 255,   4,   4,   4}},
	{&m68000_base_device::m68k_op_move16_32, 0xfff8, 0xf620, {255, 255, 255, 255,   4,   4,   4}},
	{&m68000_base_device::m68k_op_ori_8_pi7, 0xffff, 0x001f, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_8_pd7, 0xffff, 0x0027, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_ori_8_aw, 0xffff, 0x0038, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_8_al, 0xffff, 0x0039, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_8_toc, 0xffff, 0x003c, { 20,  16,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_ori_16_aw, 0xffff, 0x0078, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_16_al, 0xffff, 0x0079, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_16_tos, 0xffff, 0x007c, { 20,  16,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_ori_32_aw, 0xffff, 0x00b8, { 32,  32,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_ori_32_al, 0xffff, 0x00b9, { 36,  36,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_chk2cmp2_8_aw, 0xffff, 0x00f8, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_8_al, 0xffff, 0x00f9, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_8_pcdi, 0xffff, 0x00fa, {255, 255,  23,  23,  23,  23,  23}},
	{&m68000_base_device::m68k_op_chk2cmp2_8_pcix, 0xffff, 0x00fb, {255, 255,  23,  23,  23,  23,  23}},
	{&m68000_base_device::m68k_op_andi_8_pi7, 0xffff, 0x021f, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_8_pd7, 0xffff, 0x0227, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_andi_8_aw, 0xffff, 0x0238, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_8_al, 0xffff, 0x0239, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_8_toc, 0xffff, 0x023c, { 20,  16,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_andi_16_aw, 0xffff, 0x0278, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_16_al, 0xffff, 0x0279, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_16_tos, 0xffff, 0x027c, { 20,  16,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_andi_32_aw, 0xffff, 0x02b8, { 32,  32,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_andi_32_al, 0xffff, 0x02b9, { 36,  36,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_chk2cmp2_16_aw, 0xffff, 0x02f8, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_16_al, 0xffff, 0x02f9, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_16_pcdi, 0xffff, 0x02fa, {255, 255,  23,  23,  23,  23,  23}},
	{&m68000_base_device::m68k_op_chk2cmp2_16_pcix, 0xffff, 0x02fb, {255, 255,  23,  23,  23,  23,  23}},
	{&m68000_base_device::m68k_op_subi_8_pi7, 0xffff, 0x041f, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_8_pd7, 0xffff, 0x0427, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_subi_8_aw, 0xffff, 0x0438, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_8_al, 0xffff, 0x0439, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_16_aw, 0xffff, 0x0478, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_16_al, 0xffff, 0x0479, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_32_aw, 0xffff, 0x04b8, { 32,  32,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_subi_32_al, 0xffff, 0x04b9, { 36,  36,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_chk2cmp2_32_aw, 0xffff, 0x04f8, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_32_al, 0xffff, 0x04f9, {255, 255,  22,  22,  22,  22,  18}},
	{&m68000_base_device::m68k_op_chk2cmp2_32_pcdi, 0xffff, 0x04fa, {255, 255,  23,  23,  23,  23,  23}},
	{&m68000_base_device::m68k_op_chk2cmp2_32_pcix, 0xffff, 0x04fb, {255, 255,  23,  23,  23,  23,  23}},
	{&m68000_base_device::m68k_op_addi_8_pi7, 0xffff, 0x061f, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_8_pd7, 0xffff, 0x0627, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_addi_8_aw, 0xffff, 0x0638, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_8_al, 0xffff, 0x0639, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_16_aw, 0xffff, 0x0678, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_16_al, 0xffff, 0x0679, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_32_aw, 0xffff, 0x06b8, { 32,  32,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_addi_32_al, 0xffff, 0x06b9, { 36,  36,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_callm_32_aw, 0xffff, 0x06f8, {255, 255,  64,  64,  64,  64,  60}},
	{&m68000_base_device::m68k_op_callm_32_al, 0xffff, 0x06f9, {255, 255,  64,  64,  64,  64,  60}},
	{&m68000_base_device::m68k_op_callm_32_pcdi, 0xffff, 0x06fa, {255, 255,  65,  65,  65,  65,  60}},
	{&m68000_base_device::m68k_op_callm_32_pcix, 0xffff, 0x06fb, {255, 255,  67,  67,  67,  67,  60}},
	{&m68000_base_device::m68k_op_btst_8_s_pi7, 0xffff, 0x081f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_pd7, 0xffff, 0x0827, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_aw, 0xffff, 0x0838, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_al, 0xffff, 0x0839, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_pcdi, 0xffff, 0x083a, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_btst_8_s_pcix, 0xffff, 0x083b, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_pi7, 0xffff, 0x085f, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_pd7, 0xffff, 0x0867, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_aw, 0xffff, 0x0878, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bchg_8_s_al, 0xffff, 0x0879, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_pi7, 0xffff, 0x089f, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_pd7, 0xffff, 0x08a7, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_aw, 0xffff, 0x08b8, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bclr_8_s_al, 0xffff, 0x08b9, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_pi7, 0xffff, 0x08df, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_pd7, 0xffff, 0x08e7, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_aw, 0xffff, 0x08f8, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_bset_8_s_al, 0xffff, 0x08f9, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_8_pi7, 0xffff, 0x0a1f, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_8_pd7, 0xffff, 0x0a27, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_eori_8_aw, 0xffff, 0x0a38, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_8_al, 0xffff, 0x0a39, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_8_toc, 0xffff, 0x0a3c, { 20,  16,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_eori_16_aw, 0xffff, 0x0a78, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_16_al, 0xffff, 0x0a79, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_16_tos, 0xffff, 0x0a7c, { 20,  16,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_eori_32_aw, 0xffff, 0x0ab8, { 32,  32,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_eori_32_al, 0xffff, 0x0ab9, { 36,  36,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_cas_8_pi7, 0xffff, 0x0adf, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_8_pd7, 0xffff, 0x0ae7, {255, 255,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_cas_8_aw, 0xffff, 0x0af8, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_8_al, 0xffff, 0x0af9, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cmpi_8_pi7, 0xffff, 0x0c1f, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_pd7, 0xffff, 0x0c27, { 14,  14,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_aw, 0xffff, 0x0c38, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_al, 0xffff, 0x0c39, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_8_pcdi, 0xffff, 0x0c3a, {255, 255,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_cmpi_8_pcix, 0xffff, 0x0c3b, {255, 255,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_cmpi_16_aw, 0xffff, 0x0c78, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_16_al, 0xffff, 0x0c79, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_16_pcdi, 0xffff, 0x0c7a, {255, 255,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_cmpi_16_pcix, 0xffff, 0x0c7b, {255, 255,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_cmpi_32_aw, 0xffff, 0x0cb8, { 24,  24,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_32_al, 0xffff, 0x0cb9, { 28,  28,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_cmpi_32_pcdi, 0xffff, 0x0cba, {255, 255,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_cmpi_32_pcix, 0xffff, 0x0cbb, {255, 255,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_cas_16_aw, 0xffff, 0x0cf8, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_16_al, 0xffff, 0x0cf9, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas2_16, 0xffff, 0x0cfc, {255, 255,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_moves_8_pi7, 0xffff, 0x0e1f, {255,  18,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_8_pd7, 0xffff, 0x0e27, {255,  20,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_moves_8_aw, 0xffff, 0x0e38, {255,  26,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_8_al, 0xffff, 0x0e39, {255,  30,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_16_aw, 0xffff, 0x0e78, {255,  26,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_16_al, 0xffff, 0x0e79, {255,  30,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_32_aw, 0xffff, 0x0eb8, {255,  32,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_moves_32_al, 0xffff, 0x0eb9, {255,  36,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_cas_32_aw, 0xffff, 0x0ef8, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas_32_al, 0xffff, 0x0ef9, {255, 255,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_cas2_32, 0xffff, 0x0efc, {255, 255,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_move_8_aw_pi7, 0xffff, 0x11df, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_pd7, 0xffff, 0x11e7, { 18,  18,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_aw, 0xffff, 0x11f8, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_al, 0xffff, 0x11f9, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_pcdi, 0xffff, 0x11fa, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_pcix, 0xffff, 0x11fb, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_aw_i, 0xffff, 0x11fc, { 16,  16,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_move_8_al_pi7, 0xffff, 0x13df, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_8_al_pd7, 0xffff, 0x13e7, { 22,  22,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_8_al_aw, 0xffff, 0x13f8, { 24,  24,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_8_al_al, 0xffff, 0x13f9, { 28,  28,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_8_al_pcdi, 0xffff, 0x13fa, { 24,  24,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_8_al_pcix, 0xffff, 0x13fb, { 26,  26,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_move_8_al_i, 0xffff, 0x13fc, { 20,  20,   8,   8,   8,   8,   6}},
	{&m68000_base_device::m68k_op_move_8_pi7_pi7, 0xffff, 0x1edf, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_pd7, 0xffff, 0x1ee7, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_aw, 0xffff, 0x1ef8, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_al, 0xffff, 0x1ef9, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_pcdi, 0xffff, 0x1efa, { 16,  16,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_pcix, 0xffff, 0x1efb, { 18,  18,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_8_pi7_i, 0xffff, 0x1efc, { 12,  12,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_move_8_pd7_pi7, 0xffff, 0x1f1f, { 12,  12,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_pd7, 0xffff, 0x1f27, { 14,  14,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_aw, 0xffff, 0x1f38, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_al, 0xffff, 0x1f39, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_pcdi, 0xffff, 0x1f3a, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_pcix, 0xffff, 0x1f3b, { 18,  18,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_move_8_pd7_i, 0xffff, 0x1f3c, { 12,  12,   7,   7,   7,   7,   5}},
	{&m68000_base_device::m68k_op_move_32_aw_aw, 0xffff, 0x21f8, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_al, 0xffff, 0x21f9, { 32,  32,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_pcdi, 0xffff, 0x21fa, { 28,  28,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_pcix, 0xffff, 0x21fb, { 30,  30,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_32_aw_i, 0xffff, 0x21fc, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_32_al_aw, 0xffff, 0x23f8, { 32,  32,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_32_al_al, 0xffff, 0x23f9, { 36,  36,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_32_al_pcdi, 0xffff, 0x23fa, { 32,  32,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_32_al_pcix, 0xffff, 0x23fb, { 34,  34,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_move_32_al_i, 0xffff, 0x23fc, { 28,  28,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_16_aw_aw, 0xffff, 0x31f8, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_al, 0xffff, 0x31f9, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_pcdi, 0xffff, 0x31fa, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_pcix, 0xffff, 0x31fb, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_aw_i, 0xffff, 0x31fc, { 16,  16,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_move_16_al_aw, 0xffff, 0x33f8, { 24,  24,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_16_al_al, 0xffff, 0x33f9, { 28,  28,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_move_16_al_pcdi, 0xffff, 0x33fa, { 24,  24,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_move_16_al_pcix, 0xffff, 0x33fb, { 26,  26,  13,  13,  13,  13,   6}},
	{&m68000_base_device::m68k_op_move_16_al_i, 0xffff, 0x33fc, { 20,  20,   8,   8,   8,   8,   6}},
	{&m68000_base_device::m68k_op_negx_8_pi7, 0xffff, 0x401f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_8_pd7, 0xffff, 0x4027, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_negx_8_aw, 0xffff, 0x4038, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_8_al, 0xffff, 0x4039, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_16_aw, 0xffff, 0x4078, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_16_al, 0xffff, 0x4079, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_32_aw, 0xffff, 0x40b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_negx_32_al, 0xffff, 0x40b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_frs_aw, 0xffff, 0x40f8, { 16,  16,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_move_16_frs_al, 0xffff, 0x40f9, { 20,  20,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_clr_8_pi7, 0xffff, 0x421f, { 12,   8,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_8_pd7, 0xffff, 0x4227, { 14,  10,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_clr_8_aw, 0xffff, 0x4238, { 16,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_8_al, 0xffff, 0x4239, { 20,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_16_aw, 0xffff, 0x4278, { 16,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_16_al, 0xffff, 0x4279, { 20,  14,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_32_aw, 0xffff, 0x42b8, { 24,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_clr_32_al, 0xffff, 0x42b9, { 28,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_frc_aw, 0xffff, 0x42f8, {255,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_frc_al, 0xffff, 0x42f9, {255,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_8_pi7, 0xffff, 0x441f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_8_pd7, 0xffff, 0x4427, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_neg_8_aw, 0xffff, 0x4438, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_8_al, 0xffff, 0x4439, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_16_aw, 0xffff, 0x4478, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_16_al, 0xffff, 0x4479, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_32_aw, 0xffff, 0x44b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_neg_32_al, 0xffff, 0x44b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_aw, 0xffff, 0x44f8, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_al, 0xffff, 0x44f9, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_pcdi, 0xffff, 0x44fa, { 20,  20,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_pcix, 0xffff, 0x44fb, { 22,  22,  11,  11,  11,  11,   4}},
	{&m68000_base_device::m68k_op_move_16_toc_i, 0xffff, 0x44fc, { 16,  16,   6,   6,   6,   6,   4}},
	{&m68000_base_device::m68k_op_not_8_pi7, 0xffff, 0x461f, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_8_pd7, 0xffff, 0x4627, { 14,  14,   9,   9,   9,   9,   4}},
	{&m68000_base_device::m68k_op_not_8_aw, 0xffff, 0x4638, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_8_al, 0xffff, 0x4639, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_16_aw, 0xffff, 0x4678, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_16_al, 0xffff, 0x4679, { 20,  20,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_32_aw, 0xffff, 0x46b8, { 24,  24,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_not_32_al, 0xffff, 0x46b9, { 28,  28,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_move_16_tos_aw, 0xffff, 0x46f8, { 20,  20,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_al, 0xffff, 0x46f9, { 24,  24,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_pcdi, 0xffff, 0x46fa, { 20,  20,  13,  13,  13,  13,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_pcix, 0xffff, 0x46fb, { 22,  22,  15,  15,  15,  15,   8}},
	{&m68000_base_device::m68k_op_move_16_tos_i, 0xffff, 0x46fc, { 16,  16,  10,  10,  10,  10,   8}},
	{&m68000_base_device::m68k_op_link_32_a7, 0xffff, 0x480f, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_pi7, 0xffff, 0x481f, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_pd7, 0xffff, 0x4827, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_aw, 0xffff, 0x4838, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_nbcd_8_al, 0xffff, 0x4839, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_pea_32_aw, 0xffff, 0x4878, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_pea_32_al, 0xffff, 0x4879, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_pea_32_pcdi, 0xffff, 0x487a, { 16,  16,  10,  10,  10,  10,   5}},
	{&m68000_base_device::m68k_op_pea_32_pcix, 0xffff, 0x487b, { 20,  20,  12,  12,  12,  12,   5}},
	{&m68000_base_device::m68k_op_movem_16_re_aw, 0xffff, 0x48b8, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_movem_16_re_al, 0xffff, 0x48b9, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_movem_32_re_aw, 0xffff, 0x48f8, { 12,  12,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_movem_32_re_al, 0xffff, 0x48f9, { 16,  16,   8,   8,   8,   8,   4}},
	{&m68000_base_device::m68k_op_tst_8_pi7, 0xffff, 0x4a1f, {  8,   8,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_8_pd7, 0xffff, 0x4a27, { 10,  10,   7,   7,   7,   7,   2}},
	{&m68000_base_device::m68k_op_tst_8_aw, 0xffff, 0x4a38, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_8_al, 0xffff, 0x4a39, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_8_pcdi, 0xffff, 0x4a3a, {255, 255,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_tst_8_pcix, 0xffff, 0x4a3b, {255, 255,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_tst_8_i, 0xffff, 0x4a3c, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_tst_16_aw, 0xffff, 0x4a78, { 12,  12,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_16_al, 0xffff, 0x4a79, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_16_pcdi, 0xffff, 0x4a7a, {255, 255,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_tst_16_pcix, 0xffff, 0x4a7b, {255, 255,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_tst_16_i, 0xffff, 0x4a7c, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_tst_32_aw, 0xffff, 0x4ab8, { 16,  16,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_32_al, 0xffff, 0x4ab9, { 20,  20,   6,   6,   6,   6,   2}},
	{&m68000_base_device::m68k_op_tst_32_pcdi, 0xffff, 0x4aba, {255, 255,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_tst_32_pcix, 0xffff, 0x4abb, {255, 255,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_tst_32_i, 0xffff, 0x4abc, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_tas_8_pi7, 0xffff, 0x4adf, { 18,  18,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_tas_8_pd7, 0xffff, 0x4ae7, { 20,  20,  17,  17,  17,  17,  12}},
	{&m68000_base_device::m68k_op_tas_8_aw, 0xffff, 0x4af8, { 22,  22,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_tas_8_al, 0xffff, 0x4af9, { 26,  26,  16,  16,  16,  16,  12}},
	{&m68000_base_device::m68k_op_illegal, 0xffff, 0x4afc, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_mull_32_aw, 0xffff, 0x4c38, {255, 255,  47,  47,  47,  47,  43}},
	{&m68000_base_device::m68k_op_mull_32_al, 0xffff, 0x4c39, {255, 255,  47,  47,  47,  47,  43}},
	{&m68000_base_device::m68k_op_mull_32_pcdi, 0xffff, 0x4c3a, {255, 255,  48,  48,  48,  48,  43}},
	{&m68000_base_device::m68k_op_mull_32_pcix, 0xffff, 0x4c3b, {255, 255,  50,  50,  50,  50,  43}},
	{&m68000_base_device::m68k_op_mull_32_i, 0xffff, 0x4c3c, {255, 255,  47,  47,  47,  47,  43}},
	{&m68000_base_device::m68k_op_divl_32_aw, 0xffff, 0x4c78, {255, 255,  88,  88,  88,  88,  84}},
	{&m68000_base_device::m68k_op_divl_32_al, 0xffff, 0x4c79, {255, 255,  88,  88,  88,  88,  84}},
	{&m68000_base_device::m68k_op_divl_32_pcdi, 0xffff, 0x4c7a, {255, 255,  89,  89,  89,  89,  84}},
	{&m68000_base_device::m68k_op_divl_32_pcix, 0xffff, 0x4c7b, {255, 255,  91,  91,  91,  91,  84}},
	{&m68000_base_device::m68k_op_divl_32_i, 0xffff, 0x4c7c, {255, 255,  88,  88,  88,  88,  84}},
	{&m68000_base_device::m68k_op_movem_16_er_aw, 0xffff, 0x4cb8, { 16,  16,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_movem_16_er_al, 0xffff, 0x4cb9, { 20,  20,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_movem_16_er_pcdi, 0xffff, 0x4cba, { 16,  16,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_movem_16_er_pcix, 0xffff, 0x4cbb, { 18,  18,  11,  11,  11,  11,  11}},
	{&m68000_base_device::m68k_op_movem_32_er_aw, 0xffff, 0x4cf8, { 16,  16,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_movem_32_er_al, 0xffff, 0x4cf9, { 20,  20,  12,  12,  12,  12,   8}},
	{&m68000_base_device::m68k_op_movem_32_er_pcdi, 0xffff, 0x4cfa, { 16,  16,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_movem_32_er_pcix, 0xffff, 0x4cfb, { 18,  18,  11,  11,  11,  11,  11}},
	{&m68000_base_device::m68k_op_link_16_a7, 0xffff, 0x4e57, { 16,  16,   5,   5,   5,   5,   5}},
	{&m68000_base_device::m68k_op_unlk_32_a7, 0xffff, 0x4e5f, { 12,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_reset, 0xffff, 0x4e70, {  0,   0,   0,   0,   0,   0,   0}},
	{&m68000_base_device::m68k_op_nop, 0xffff, 0x4e71, {  4,   4,   2,   2,   2,   2,   2}},
	{&m68000_base_device::m68k_op_stop, 0xffff, 0x4e72, {  4,   4,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_rte_32, 0xffff, 0x4e73, { 20,  24,  20,  20,  20,  20,  20}},
	{&m68000_base_device::m68k_op_rtd_32, 0xffff, 0x4e74, {255,  16,  10,  10,  10,  10,  10}},
	{&m68000_base_device::m68k_op_rts_32, 0xffff, 0x4e75, { 16,  16,  10,  10,  10,  10,  10}},
	{&m68000_base_device::m68k_op_trapv, 0xffff, 0x4e76, {  4,   4,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_rtr_32, 0xffff, 0x4e77, { 20,  20,  14,  14,  14,  14,  14}},
	{&m68000_base_device::m68k_op_movec_32_cr, 0xffff, 0x4e7a, {255,  12,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_movec_32_rc, 0xffff, 0x4e7b, {255,  10,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_jsr_32_aw, 0xffff, 0x4eb8, { 18,  18,   4,   4,   4,   4,   0}},
	{&m68000_base_device::m68k_op_jsr_32_al, 0xffff, 0x4eb9, { 20,  20,   4,   4,   4,   4,   0}},
	{&m68000_base_device::m68k_op_jsr_32_pcdi, 0xffff, 0x4eba, { 18,  18,   5,   5,   5,   5,   0}},
	{&m68000_base_device::m68k_op_jsr_32_pcix, 0xffff, 0x4ebb, { 22,  22,   7,   7,   7,   7,   0}},
	{&m68000_base_device::m68k_op_jmp_32_aw, 0xffff, 0x4ef8, { 10,  10,   4,   4,   4,   4,   0}},
	{&m68000_base_device::m68k_op_jmp_32_al, 0xffff, 0x4ef9, { 12,  12,   4,   4,   4,   4,   0}},
	{&m68000_base_device::m68k_op_jmp_32_pcdi, 0xffff, 0x4efa, { 10,  10,   5,   5,   5,   5,   0}},
	{&m68000_base_device::m68k_op_jmp_32_pcix, 0xffff, 0x4efb, { 14,  14,   7,   7,   7,   7,   0}},
	{&m68000_base_device::m68k_op_st_8_pi7, 0xffff, 0x50df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_st_8_pd7, 0xffff, 0x50e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_st_8_aw, 0xffff, 0x50f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_st_8_al, 0xffff, 0x50f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapt_16, 0xffff, 0x50fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapt_32, 0xffff, 0x50fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapt, 0xffff, 0x50fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_sf_8_pi7, 0xffff, 0x51df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sf_8_pd7, 0xffff, 0x51e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sf_8_aw, 0xffff, 0x51f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sf_8_al, 0xffff, 0x51f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapf_16, 0xffff, 0x51fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapf_32, 0xffff, 0x51fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapf, 0xffff, 0x51fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_shi_8_pi7, 0xffff, 0x52df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_shi_8_pd7, 0xffff, 0x52e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_shi_8_aw, 0xffff, 0x52f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_shi_8_al, 0xffff, 0x52f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_traphi_16, 0xffff, 0x52fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_traphi_32, 0xffff, 0x52fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_traphi, 0xffff, 0x52fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_sls_8_pi7, 0xffff, 0x53df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sls_8_pd7, 0xffff, 0x53e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sls_8_aw, 0xffff, 0x53f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sls_8_al, 0xffff, 0x53f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapls_16, 0xffff, 0x53fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapls_32, 0xffff, 0x53fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapls, 0xffff, 0x53fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_scc_8_pi7, 0xffff, 0x54df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_scc_8_pd7, 0xffff, 0x54e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_scc_8_aw, 0xffff, 0x54f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_scc_8_al, 0xffff, 0x54f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapcc_16, 0xffff, 0x54fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapcc_32, 0xffff, 0x54fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapcc, 0xffff, 0x54fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_scs_8_pi7, 0xffff, 0x55df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_scs_8_pd7, 0xffff, 0x55e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_scs_8_aw, 0xffff, 0x55f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_scs_8_al, 0xffff, 0x55f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapcs_16, 0xffff, 0x55fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapcs_32, 0xffff, 0x55fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapcs, 0xffff, 0x55fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_sne_8_pi7, 0xffff, 0x56df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sne_8_pd7, 0xffff, 0x56e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sne_8_aw, 0xffff, 0x56f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sne_8_al, 0xffff, 0x56f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapne_16, 0xffff, 0x56fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapne_32, 0xffff, 0x56fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapne, 0xffff, 0x56fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_seq_8_pi7, 0xffff, 0x57df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_seq_8_pd7, 0xffff, 0x57e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_seq_8_aw, 0xffff, 0x57f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_seq_8_al, 0xffff, 0x57f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapeq_16, 0xffff, 0x57fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapeq_32, 0xffff, 0x57fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapeq, 0xffff, 0x57fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_svc_8_pi7, 0xffff, 0x58df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_svc_8_pd7, 0xffff, 0x58e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_svc_8_aw, 0xffff, 0x58f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_svc_8_al, 0xffff, 0x58f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapvc_16, 0xffff, 0x58fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapvc_32, 0xffff, 0x58fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapvc, 0xffff, 0x58fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_svs_8_pi7, 0xffff, 0x59df, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_svs_8_pd7, 0xffff, 0x59e7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_svs_8_aw, 0xffff, 0x59f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_svs_8_al, 0xffff, 0x59f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapvs_16, 0xffff, 0x59fa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapvs_32, 0xffff, 0x59fb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapvs, 0xffff, 0x59fc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_spl_8_pi7, 0xffff, 0x5adf, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_spl_8_pd7, 0xffff, 0x5ae7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_spl_8_aw, 0xffff, 0x5af8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_spl_8_al, 0xffff, 0x5af9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trappl_16, 0xffff, 0x5afa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trappl_32, 0xffff, 0x5afb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trappl, 0xffff, 0x5afc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_smi_8_pi7, 0xffff, 0x5bdf, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_smi_8_pd7, 0xffff, 0x5be7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_smi_8_aw, 0xffff, 0x5bf8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_smi_8_al, 0xffff, 0x5bf9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapmi_16, 0xffff, 0x5bfa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapmi_32, 0xffff, 0x5bfb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapmi, 0xffff, 0x5bfc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_sge_8_pi7, 0xffff, 0x5cdf, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sge_8_pd7, 0xffff, 0x5ce7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sge_8_aw, 0xffff, 0x5cf8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sge_8_al, 0xffff, 0x5cf9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapge_16, 0xffff, 0x5cfa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapge_32, 0xffff, 0x5cfb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapge, 0xffff, 0x5cfc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_slt_8_pi7, 0xffff, 0x5ddf, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_slt_8_pd7, 0xffff, 0x5de7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_slt_8_aw, 0xffff, 0x5df8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_slt_8_al, 0xffff, 0x5df9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_traplt_16, 0xffff, 0x5dfa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_traplt_32, 0xffff, 0x5dfb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_traplt, 0xffff, 0x5dfc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_sgt_8_pi7, 0xffff, 0x5edf, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sgt_8_pd7, 0xffff, 0x5ee7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sgt_8_aw, 0xffff, 0x5ef8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sgt_8_al, 0xffff, 0x5ef9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_trapgt_16, 0xffff, 0x5efa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_trapgt_32, 0xffff, 0x5efb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_trapgt, 0xffff, 0x5efc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_sle_8_pi7, 0xffff, 0x5fdf, { 12,  12,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sle_8_pd7, 0xffff, 0x5fe7, { 14,  14,  11,  11,  11,  11,   6}},
	{&m68000_base_device::m68k_op_sle_8_aw, 0xffff, 0x5ff8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_sle_8_al, 0xffff, 0x5ff9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_traple_16, 0xffff, 0x5ffa, {255, 255,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_traple_32, 0xffff, 0x5ffb, {255, 255,   8,   8,   8,   8,   8}},
	{&m68000_base_device::m68k_op_traple, 0xffff, 0x5ffc, {255, 255,   4,   4,   4,   4,   4}},
	{&m68000_base_device::m68k_op_bra_16, 0xffff, 0x6000, { 10,  10,  10,  10,  10,  10,  10}},
	{&m68000_base_device::m68k_op_bra_32, 0xffff, 0x60ff, { 10,  10,  10,  10,  10,  10,  10}},
	{&m68000_base_device::m68k_op_bsr_16, 0xffff, 0x6100, { 18,  18,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_bsr_32, 0xffff, 0x61ff, { 18,  18,   7,   7,   7,   7,   7}},
	{&m68000_base_device::m68k_op_bhi_16, 0xffff, 0x6200, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bhi_32, 0xffff, 0x62ff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bls_16, 0xffff, 0x6300, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bls_32, 0xffff, 0x63ff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bcc_16, 0xffff, 0x6400, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bcc_32, 0xffff, 0x64ff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bcs_16, 0xffff, 0x6500, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bcs_32, 0xffff, 0x65ff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bne_16, 0xffff, 0x6600, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bne_32, 0xffff, 0x66ff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_beq_16, 0xffff, 0x6700, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_beq_32, 0xffff, 0x67ff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bvc_16, 0xffff, 0x6800, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bvc_32, 0xffff, 0x68ff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bvs_16, 0xffff, 0x6900, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bvs_32, 0xffff, 0x69ff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bpl_16, 0xffff, 0x6a00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bpl_32, 0xffff, 0x6aff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bmi_16, 0xffff, 0x6b00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bmi_32, 0xffff, 0x6bff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bge_16, 0xffff, 0x6c00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bge_32, 0xffff, 0x6cff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_blt_16, 0xffff, 0x6d00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_blt_32, 0xffff, 0x6dff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bgt_16, 0xffff, 0x6e00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_bgt_32, 0xffff, 0x6eff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_ble_16, 0xffff, 0x6f00, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_ble_32, 0xffff, 0x6fff, { 10,  10,   6,   6,   6,   6,   6}},
	{&m68000_base_device::m68k_op_sbcd_8_mm_axy7, 0xffff, 0x8f0f, { 18,  18,  16,  16,  16,  16,  16}},
	{&m68000_base_device::m68k_op_pack_16_mm_axy7, 0xffff, 0x8f4f, {255, 255,  13,  13,  13,  13,  13}},
	{&m68000_base_device::m68k_op_unpk_16_mm_axy7, 0xffff, 0x8f8f, {255, 255,  13,  13,  13,  13,  13}},
	{&m68000_base_device::m68k_op_subx_8_mm_axy7, 0xffff, 0x9f0f, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_cmpm_8_axy7, 0xffff, 0xbf0f, { 12,  12,   9,   9,   9,   9,   9}},
	{&m68000_base_device::m68k_op_abcd_8_mm_axy7, 0xffff, 0xcf0f, { 18,  18,  16,  16,  16,  16,  16}},
	{&m68000_base_device::m68k_op_addx_8_mm_axy7, 0xffff, 0xdf0f, { 18,  18,  12,  12,  12,  12,  12}},
	{&m68000_base_device::m68k_op_asr_16_aw, 0xffff, 0xe0f8, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_asr_16_al, 0xffff, 0xe0f9, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_asl_16_aw, 0xffff, 0xe1f8, { 16,  16,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_asl_16_al, 0xffff, 0xe1f9, { 20,  20,  10,  10,  10,  10,   6}},
	{&m68000_base_device::m68k_op_lsr_16_aw, 0xffff, 0xe2f8, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_lsr_16_al, 0xffff, 0xe2f9, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_lsl_16_aw, 0xffff, 0xe3f8, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_lsl_16_al, 0xffff, 0xe3f9, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_roxr_16_aw, 0xffff, 0xe4f8, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_roxr_16_al, 0xffff, 0xe4f9, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_roxl_16_aw, 0xffff, 0xe5f8, { 16,  16,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_roxl_16_al, 0xffff, 0xe5f9, { 20,  20,   9,   9,   9,   9,   5}},
	{&m68000_base_device::m68k_op_ror_16_aw, 0xffff, 0xe6f8, { 16,  16,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_ror_16_al, 0xffff, 0xe6f9, { 20,  20,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_rol_16_aw, 0xffff, 0xe7f8, { 16,  16,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_rol_16_al, 0xffff, 0xe7f9, { 20,  20,  11,  11,  11,  11,   7}},
	{&m68000_base_device::m68k_op_bftst_32_aw, 0xffff, 0xe8f8, {255, 255,  17,  17,  17,  17,  13}},
	{&m68000_base_device::m68k_op_bftst_32_al, 0xffff, 0xe8f9, {255, 255,  17,  17,  17,  17,  13}},
	{&m68000_base_device::m68k_op_bftst_32_pcdi, 0xffff, 0xe8fa, {255, 255,  18,  18,  18,  18,  13}},
	{&m68000_base_device::m68k_op_bftst_32_pcix, 0xffff, 0xe8fb, {255, 255,  20,  20,  20,  20,  13}},
	{&m68000_base_device::m68k_op_bfextu_32_aw, 0xffff, 0xe9f8, {255, 255,  19,  19,  19,  19,  15}},
	{&m68000_base_device::m68k_op_bfextu_32_al, 0xffff, 0xe9f9, {255, 255,  19,  19,  19,  19,  15}},
	{&m68000_base_device::m68k_op_bfextu_32_pcdi, 0xffff, 0xe9fa, {255, 255,  20,  20,  20,  20,  15}},
	{&m68000_base_device::m68k_op_bfextu_32_pcix, 0xffff, 0xe9fb, {255, 255,  22,  22,  22,  22,  15}},
	{&m68000_base_device::m68k_op_bfchg_32_aw, 0xffff, 0xeaf8, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfchg_32_al, 0xffff, 0xeaf9, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfexts_32_aw, 0xffff, 0xebf8, {255, 255,  19,  19,  19,  19,  15}},
	{&m68000_base_device::m68k_op_bfexts_32_al, 0xffff, 0xebf9, {255, 255,  19,  19,  19,  19,  15}},
	{&m68000_base_device::m68k_op_bfexts_32_pcdi, 0xffff, 0xebfa, {255, 255,  20,  20,  20,  20,  15}},
	{&m68000_base_device::m68k_op_bfexts_32_pcix, 0xffff, 0xebfb, {255, 255,  22,  22,  22,  22,  15}},
	{&m68000_base_device::m68k_op_bfclr_32_aw, 0xffff, 0xecf8, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfclr_32_al, 0xffff, 0xecf9, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfffo_32_aw, 0xffff, 0xedf8, {255, 255,  32,  32,  32,  32,  28}},
	{&m68000_base_device::m68k_op_bfffo_32_al, 0xffff, 0xedf9, {255, 255,  32,  32,  32,  32,  28}},
	{&m68000_base_device::m68k_op_bfffo_32_pcdi, 0xffff, 0xedfa, {255, 255,  33,  33,  33,  33,  28}},
	{&m68000_base_device::m68k_op_bfffo_32_pcix, 0xffff, 0xedfb, {255, 255,  35,  35,  35,  35,  28}},
	{&m68000_base_device::m68k_op_bfset_32_aw, 0xffff, 0xeef8, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfset_32_al, 0xffff, 0xeef9, {255, 255,  24,  24,  24,  24,  20}},
	{&m68000_base_device::m68k_op_bfins_32_aw, 0xffff, 0xeff8, {255, 255,  21,  21,  21,  21,  17}},
	{&m68000_base_device::m68k_op_bfins_32_al, 0xffff, 0xeff9, {255, 255,  21,  21,  21,  21,  17}},
	{nullptr, 0, 0, {0, 0, 0, 0, 0}}
};


	const opcode_handler_struct *ostruct;
	int i;
	int j;
	int k;

	for(i = 0; i < 0x10000; i++)
	{
		/* default to illegal */
		for(k=0;k<NUM_CPU_TYPES;k++)
		{
			m68ki_instruction_jump_table[k][i] = &m68000_base_device::m68k_op_illegal;
			m68ki_cycles[k][i] = 0;
		}
	}

	ostruct = m68k_opcode_handler_table;
	while(ostruct->mask != 0xff00)
	{
		for(i = 0;i < 0x10000;i++)
		{
			if((i & ostruct->mask) == ostruct->match)
				m68ki_set_one(i, ostruct);
		}
		ostruct++;
	}
	while(ostruct->mask == 0xff00)
	{
		for(i = 0;i <= 0xff;i++)
			m68ki_set_one(ostruct->match | i, ostruct);
		ostruct++;
	}
	while(ostruct->mask == 0xff20)
	{
		for(i = 0;i < 4;i++)
		{
			for(j = 0;j < 32;j++)
			{
				m68ki_set_one(ostruct->match | (i << 6) | j, ostruct);
			}
		}
		ostruct++;
	}
	while(ostruct->mask == 0xf1f8)
	{
		for(i = 0;i < 8;i++)
		{
			for(j = 0;j < 8;j++)
				m68ki_set_one(ostruct->match | (i << 9) | j, ostruct);
		}
		ostruct++;
	}
	while(ostruct->mask == 0xffd8)
	{
		for(i = 0;i < 2;i++)
		{
			for(j = 0;j < 8;j++)
			{
				m68ki_set_one(ostruct->match | (i << 5) | j, ostruct);
			}
		}
		ostruct++;
	}
	while(ostruct->mask == 0xfff0)
	{
		for(i = 0;i <= 0x0f;i++)
			m68ki_set_one(ostruct->match | i, ostruct);
		ostruct++;
	}
	while(ostruct->mask == 0xf1ff)
	{
		for(i = 0;i <= 0x07;i++)
			m68ki_set_one(ostruct->match | (i << 9), ostruct);
		ostruct++;
	}
	while(ostruct->mask == 0xfff8)
	{
		for(i = 0;i <= 0x07;i++)
			m68ki_set_one(ostruct->match | i, ostruct);
		ostruct++;
	}
	while(ostruct->mask == 0xffff)
	{
		m68ki_set_one(ostruct->match, ostruct);
		ostruct++;
	}

	// if we fell all the way through with a non-zero mask, the opcode table wasn't built properly
	if (ostruct->mask != 0)
	{
		fatalerror("m68ki_build_opcode_table: unhandled opcode mask %x (match %x), m68k core will not function!\n", ostruct->mask, ostruct->match);
	}
}


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */


