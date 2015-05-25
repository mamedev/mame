// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
** Model 1 coprocessor TGP simulation
*/

#include "emu.h"
#include "debugger.h"
#include "cpu/mb86233/mb86233.h"
#include "cpu/v60/v60.h"
#include "includes/model1.h"

#define TGP_FUNCTION(name) void name()


UINT32 model1_state::fifoout_pop()
{
	if(m_fifoout_wpos == m_fifoout_rpos) {
		fatalerror("TGP FIFOOUT underflow (%x)\n", safe_pc());
	}
	UINT32 v = m_fifoout_data[m_fifoout_rpos++];
	if(m_fifoout_rpos == FIFO_SIZE)
		m_fifoout_rpos = 0;
	return v;
}


void model1_state::fifoout_push(UINT32 data)
{
	if(!m_puuu)
		logerror("TGP: Push %d\n", data);
	else
		m_puuu = 0;
	m_fifoout_data[m_fifoout_wpos++] = data;
	if(m_fifoout_wpos == FIFO_SIZE)
		m_fifoout_wpos = 0;
	if(m_fifoout_wpos == m_fifoout_rpos)
		logerror("TGP FIFOOUT overflow\n");
}

void model1_state::fifoout_push_f(float data)
{
	m_puuu = 1;

	logerror("TGP: Push %f\n", (double) data);
	fifoout_push(f2u(data));
}

UINT32 model1_state::fifoin_pop()
{
	if(m_fifoin_wpos == m_fifoin_rpos)
		logerror("TGP FIFOIN underflow\n");
	UINT32 v = m_fifoin_data[m_fifoin_rpos++];
	if(m_fifoin_rpos == FIFO_SIZE)
		m_fifoin_rpos = 0;
	return v;
}

void model1_state::fifoin_push(UINT32 data)
{
	//  logerror("TGP FIFOIN write %08x (%x)\n", data, safe_pc());
	m_fifoin_data[m_fifoin_wpos++] = data;
	if(m_fifoin_wpos == FIFO_SIZE)
		m_fifoin_wpos = 0;
	if(m_fifoin_wpos == m_fifoin_rpos)
		logerror("TGP FIFOIN overflow\n");
	m_fifoin_cbcount--;
	if(!m_fifoin_cbcount)
		(this->*m_fifoin_cb)();
}

float model1_state::fifoin_pop_f()
{
	return u2f(fifoin_pop());
}

void model1_state::next_fn()
{
	m_fifoin_cbcount = 1;
	m_fifoin_cb = m_swa ? &model1_state::function_get_swa : &model1_state::function_get_vf;
}

static float tcos(INT16 a)
{
	if(a == 16384 || a == -16384)
		return 0;
	else if(a == -32768)
		return -1;
	else if(a == 0)
		return 1;
	else
		return cos(a*(2*M_PI/65536.0));
}

static float tsin(INT16 a)
{
	if(a == 0 || a == -32768)
		return 0;
	else if(a == 16384)
		return 1;
	else if(a == -16384)
		return -1;
	else
		return sin(a*(2*M_PI/65536.0));
}

UINT16 model1_state::ram_get_i()
{
	return m_ram_data[m_ram_scanadr++];
}

float model1_state::ram_get_f()
{
	return u2f(m_ram_data[m_ram_scanadr++]);
}

TGP_FUNCTION( model1_state::fadd )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float r = a+b;
	logerror("TGP fadd %f+%f=%f (%x)\n", a, b, r, m_pushpc);
	fifoout_push_f(r);
	next_fn();
}

TGP_FUNCTION( model1_state::fsub )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float r = a-b;
	m_dump = 1;
	logerror("TGP fsub %f-%f=%f (%x)\n", a, b, r, m_pushpc);
	fifoout_push_f(r);
	next_fn();
}

TGP_FUNCTION( model1_state::fmul )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float r = a*b;
	logerror("TGP fmul %f*%f=%f (%x)\n", a, b, r, m_pushpc);
	fifoout_push_f(r);
	next_fn();
}

TGP_FUNCTION( model1_state::fdiv )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
//  float r = !b ? 1e39 : a/b;
	float r = !b ? 0 : a * (1/b);
	logerror("TGP fdiv %f/%f=%f (%x)\n", a, b, r, m_pushpc);
	fifoout_push_f(r);
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_push )
{
	if(m_mat_stack_pos != MAT_STACK_SIZE) {
		memcpy(m_mat_stack[m_mat_stack_pos], m_cmat, sizeof(m_cmat));
		m_mat_stack_pos++;
	}
	logerror("TGP matrix_push (depth=%d, pc=%x)\n", m_mat_stack_pos, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_pop )
{
	if(m_mat_stack_pos) {
		m_mat_stack_pos--;
		memcpy(m_cmat, m_mat_stack[m_mat_stack_pos], sizeof(m_cmat));
	}
	logerror("TGP matrix_pop (depth=%d, pc=%x)\n", m_mat_stack_pos, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_write )
{
	int i;
	for(i=0; i<12; i++)
		m_cmat[i] = fifoin_pop_f();
	logerror("TGP matrix_write %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f) (%x)\n",
				m_cmat[0], m_cmat[1], m_cmat[2], m_cmat[3], m_cmat[4], m_cmat[5], m_cmat[6], m_cmat[7], m_cmat[8], m_cmat[9], m_cmat[10], m_cmat[11],
				m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::clear_stack )
{
	logerror("TGP clear_stack (%x)\n", m_pushpc);
	m_mat_stack_pos = 0;
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_mul )
{
	float m[12];
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	float g = fifoin_pop_f();
	float h = fifoin_pop_f();
	float i = fifoin_pop_f();
	float j = fifoin_pop_f();
	float k = fifoin_pop_f();
	float l = fifoin_pop_f();
	logerror("TGP matrix_mul %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, m_pushpc);
	m[0]  = a*m_cmat[0] + b*m_cmat[3] + c*m_cmat[6];
	m[1]  = a*m_cmat[1] + b*m_cmat[4] + c*m_cmat[7];
	m[2]  = a*m_cmat[2] + b*m_cmat[5] + c*m_cmat[8];
	m[3]  = d*m_cmat[0] + e*m_cmat[3] + f*m_cmat[6];
	m[4]  = d*m_cmat[1] + e*m_cmat[4] + f*m_cmat[7];
	m[5]  = d*m_cmat[2] + e*m_cmat[5] + f*m_cmat[8];
	m[6]  = g*m_cmat[0] + h*m_cmat[3] + i*m_cmat[6];
	m[7]  = g*m_cmat[1] + h*m_cmat[4] + i*m_cmat[7];
	m[8]  = g*m_cmat[2] + h*m_cmat[5] + i*m_cmat[8];
	m[9]  = j*m_cmat[0] + k*m_cmat[3] + l*m_cmat[6] + m_cmat[9];
	m[10] = j*m_cmat[1] + k*m_cmat[4] + l*m_cmat[7] + m_cmat[10];
	m[11] = j*m_cmat[2] + k*m_cmat[5] + l*m_cmat[8] + m_cmat[11];

	memcpy(m_cmat, m, sizeof(m));
	next_fn();
}

TGP_FUNCTION( model1_state::anglev )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	logerror("TGP anglev %f, %f (%x)\n", a, b, m_pushpc);
	if(!b) {
		if(a>=0)
			fifoout_push(0);
		else
			fifoout_push((UINT32)-32768);
	} else if(!a) {
		if(b>=0)
			fifoout_push(16384);
		else
			fifoout_push((UINT32)-16384);
	} else
		fifoout_push((INT16)(atan2(b, a)*32768/M_PI));
	next_fn();
}

TGP_FUNCTION( model1_state::f11 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	float g = fifoin_pop_f();
	float h = fifoin_pop_f();
	float i = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	(void)g;
	(void)h;
	(void)i;
	logerror("TGP f11 %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, m_pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

TGP_FUNCTION( model1_state::normalize )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float n = (a*a+b*b+c*c) / sqrtf(a*a+b*b+c*c);
	logerror("TGP normalize %f, %f, %f (%x)\n", a, b, c, m_pushpc);
	fifoout_push_f(a/n);
	fifoout_push_f(b/n);
	fifoout_push_f(c/n);
	next_fn();
}

TGP_FUNCTION( model1_state::acc_seti )
{
	INT32 a = fifoin_pop();
	m_dump = 1;
	logerror("TGP acc_seti %d (%x)\n", a, m_pushpc);
	m_acc = a;
	next_fn();
}

TGP_FUNCTION( model1_state::track_select )
{
	INT32 a = fifoin_pop();
	logerror("TGP track_select %d (%x)\n", a, m_pushpc);
	m_tgp_vr_select = a;
	next_fn();
}

TGP_FUNCTION( model1_state::f14 )
{
	m_tgp_vr_base[0] = fifoin_pop_f();
	m_tgp_vr_base[1] = fifoin_pop_f();
	m_tgp_vr_base[2] = fifoin_pop_f();
	m_tgp_vr_base[3] = fifoin_pop_f();

	next_fn();
}

TGP_FUNCTION( model1_state::f15_swa )
{
	logerror("TGP f15_swa (%x)\n", m_pushpc);

	next_fn();
}

TGP_FUNCTION( model1_state::anglep )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	logerror("TGP anglep %f, %f, %f, %f (%x)\n", a, b, c, d, m_pushpc);
	c = a - c;
	d = b - d;
	if(!d) {
		if(c>=0)
			fifoout_push(0);
		else
			fifoout_push((UINT32)-32768);
	} else if(!c) {
		if(d>=0)
			fifoout_push(16384);
		else
			fifoout_push((UINT32)-16384);
	} else
		fifoout_push((INT16)(atan2(d, c)*32768/M_PI));
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_ident )
{
	logerror("TGP matrix_ident (%x)\n", m_pushpc);
	memset(m_cmat, 0, sizeof(m_cmat));
	m_cmat[0] = 1.0;
	m_cmat[4] = 1.0;
	m_cmat[8] = 1.0;
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_read )
{
	int i;
	logerror("TGP matrix_read (%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f) (%x)\n",
				m_cmat[0], m_cmat[1], m_cmat[2], m_cmat[3], m_cmat[4], m_cmat[5], m_cmat[6], m_cmat[7], m_cmat[8], m_cmat[9], m_cmat[10], m_cmat[11], m_pushpc);
	for(i=0; i<12; i++)
		fifoout_push_f(m_cmat[i]);
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_trans )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();

	m_cmat[ 9] += m_cmat[0]*a+m_cmat[3]*b+m_cmat[6]*c;
	m_cmat[10] += m_cmat[1]*a+m_cmat[4]*b+m_cmat[7]*c;
	m_cmat[11] += m_cmat[2]*a+m_cmat[5]*b+m_cmat[8]*c;
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_scale )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	logerror("TGP matrix_scale %f, %f, %f (%x)\n", a, b, c, m_pushpc);
	m_cmat[0] *= a;
	m_cmat[1] *= a;
	m_cmat[2] *= a;
	m_cmat[3] *= b;
	m_cmat[4] *= b;
	m_cmat[5] *= b;
	m_cmat[6] *= c;
	m_cmat[7] *= c;
	m_cmat[8] *= c;
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_rotx )
{
	INT16 a = fifoin_pop();
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;
	logerror("TGP matrix_rotx %d (%x)\n", a, m_pushpc);
	t1 = m_cmat[3];
	t2 = m_cmat[6];
	m_cmat[3] = c*t1-s*t2;
	m_cmat[6] = s*t1+c*t2;
	t1 = m_cmat[4];
	t2 = m_cmat[7];
	m_cmat[4] = c*t1-s*t2;
	m_cmat[7] = s*t1+c*t2;
	t1 = m_cmat[5];
	t2 = m_cmat[8];
	m_cmat[5] = c*t1-s*t2;
	m_cmat[8] = s*t1+c*t2;
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_roty )
{
	INT16 a = fifoin_pop();
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;

	logerror("TGP matrix_roty %d (%x)\n", a, m_pushpc);
	t1 = m_cmat[6];
	t2 = m_cmat[0];
	m_cmat[6] = c*t1-s*t2;
	m_cmat[0] = s*t1+c*t2;
	t1 = m_cmat[7];
	t2 = m_cmat[1];
	m_cmat[7] = c*t1-s*t2;
	m_cmat[1] = s*t1+c*t2;
	t1 = m_cmat[8];
	t2 = m_cmat[2];
	m_cmat[8] = c*t1-s*t2;
	m_cmat[2] = s*t1+c*t2;
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_rotz )
{
	INT16 a = fifoin_pop();
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;

	logerror("TGP matrix_rotz %d (%x)\n", a, m_pushpc);
	t1 = m_cmat[0];
	t2 = m_cmat[3];
	m_cmat[0] = c*t1-s*t2;
	m_cmat[3] = s*t1+c*t2;
	t1 = m_cmat[1];
	t2 = m_cmat[4];
	m_cmat[1] = c*t1-s*t2;
	m_cmat[4] = s*t1+c*t2;
	t1 = m_cmat[2];
	t2 = m_cmat[5];
	m_cmat[2] = c*t1-s*t2;
	m_cmat[5] = s*t1+c*t2;
	next_fn();
}

TGP_FUNCTION( model1_state::track_read_quad )
{
	const UINT32 *tgp_data = (const UINT32 *)memregion("user2")->base();
	UINT32 a = fifoin_pop();
	int offd;

	logerror("TGP track_read_quad %d (%x)\n", a, m_pushpc);

	offd = tgp_data[0x20+m_tgp_vr_select] + 16*a;
	fifoout_push(tgp_data[offd]);
	fifoout_push(tgp_data[offd+1]);
	fifoout_push(tgp_data[offd+2]);
	fifoout_push(tgp_data[offd+3]);
	fifoout_push(tgp_data[offd+4]);
	fifoout_push(tgp_data[offd+5]);
	fifoout_push(tgp_data[offd+6]);
	fifoout_push(tgp_data[offd+7]);
	fifoout_push(tgp_data[offd+8]);
	fifoout_push(tgp_data[offd+9]);
	fifoout_push(tgp_data[offd+10]);
	fifoout_push(tgp_data[offd+11]);
	next_fn();
}

TGP_FUNCTION( model1_state::f24_swa )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	UINT32 g = fifoin_pop();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	(void)g;
	logerror("TGP f24_swa %f, %f, %f, %f, %f, %f, %x (%x)\n", a, b, c, d, e, f, g, m_pushpc);
	fifoout_push_f(0);
	next_fn();
}

TGP_FUNCTION( model1_state::transform_point )
{
	float x = fifoin_pop_f();
	float y = fifoin_pop_f();
	float z = fifoin_pop_f();
	logerror("TGP transform_point %f, %f, %f (%x)\n", x, y, z, m_pushpc);

	fifoout_push_f(m_cmat[0]*x+m_cmat[3]*y+m_cmat[6]*z+m_cmat[9]);
	fifoout_push_f(m_cmat[1]*x+m_cmat[4]*y+m_cmat[7]*z+m_cmat[10]);
	fifoout_push_f(m_cmat[2]*x+m_cmat[5]*y+m_cmat[8]*z+m_cmat[11]);
	next_fn();
}

TGP_FUNCTION( model1_state::fcos_m1 )
{
	INT16 a = fifoin_pop();
	logerror("TGP fcos %d (%x)\n", a, m_pushpc);
	fifoout_push_f(tcos(a));
	next_fn();
}

TGP_FUNCTION( model1_state::fsin_m1 )
{
	INT16 a = fifoin_pop();
	logerror("TGP fsin %d (%x)\n", a, m_pushpc);
	fifoout_push_f(tsin(a));
	next_fn();
}

TGP_FUNCTION( model1_state::fcosm_m1 )
{
	INT16 a = fifoin_pop();
	float b = fifoin_pop_f();
	logerror("TGP fcosm %d, %f (%x)\n", a, b, m_pushpc);
	fifoout_push_f(b*tcos(a));
	next_fn();
}

TGP_FUNCTION( model1_state::fsinm_m1 )
{
	INT16 a = fifoin_pop();
	float b = fifoin_pop_f();
	m_dump = 1;
	logerror("TGP fsinm %d, %f (%x)\n", a, b, m_pushpc);
	fifoout_push_f(b*tsin(a));
	next_fn();
}

TGP_FUNCTION( model1_state::distance3 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	logerror("TGP distance3 (%f, %f, %f), (%f, %f, %f) (%x)\n", a, b, c, d, e, f, m_pushpc);
	a -= d;
	b -= e;
	c -= f;
	fifoout_push_f((a*a+b*b+c*c)/sqrtf(a*a+b*b+c*c));
	next_fn();
}

TGP_FUNCTION( model1_state::ftoi )
{
	float a = fifoin_pop_f();
	logerror("TGP ftoi %f (%x)\n", a, m_pushpc);
	fifoout_push((int)a);
	next_fn();
}

TGP_FUNCTION( model1_state::itof )
{
	INT32 a = fifoin_pop();
	logerror("TGP itof %d (%x)\n", a, m_pushpc);
	fifoout_push_f(a);
	next_fn();
}

TGP_FUNCTION( model1_state::acc_set )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_set %f (%x)\n", a, m_pushpc);
	m_acc = a;
	next_fn();
}

TGP_FUNCTION( model1_state::acc_get )
{
	logerror("TGP acc_get (%x)\n", m_pushpc);
	fifoout_push_f(m_acc);
	next_fn();
}

TGP_FUNCTION( model1_state::acc_add )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_add %f (%x)\n", a, m_pushpc);
	m_acc += a;
	next_fn();
}

TGP_FUNCTION( model1_state::acc_sub )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_sub %f (%x)\n", a, m_pushpc);
	m_acc -= a;
	next_fn();
}

TGP_FUNCTION( model1_state::acc_mul )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_mul %f (%x)\n", a, m_pushpc);
	m_acc *= a;
	next_fn();
}

TGP_FUNCTION( model1_state::acc_div )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_div %f (%x)\n", a, m_pushpc);
	m_acc /= a;
	next_fn();
}

TGP_FUNCTION( model1_state::f42 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP f42 %f, %f, %f (%x)\n", a, b, c, m_pushpc);
	//  fifoout_push_f((machine().rand() % 1000) - 500);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}


// r = (x2 + y2 + z2)1/2,     f = tan-1(y/(x2+z2)1/2),     q = tan-1(z/x)

TGP_FUNCTION( model1_state::xyz2rqf )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float norm;
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP xyz2rqf %f, %f, %f (%x)\n", a, b, c, m_pushpc);
	fifoout_push_f((a*a+b*b+c*c)/sqrtf(a*a+b*b+c*c));
	norm = sqrt(a*a+c*c);
	if(!c) {
		if(a>=0)
			fifoout_push(0);
		else
			fifoout_push((UINT32)-32768);
	} else if(!a) {
		if(c>=0)
			fifoout_push(16384);
		else
			fifoout_push((UINT32)-16384);
	} else
		fifoout_push((INT16)(atan2(c, a)*32768/M_PI));

	if(!b)
		fifoout_push(0);
	else if(!norm) {
		if(b>=0)
			fifoout_push(16384);
		else
			fifoout_push((UINT32)-16384);
	} else
		fifoout_push((INT16)(atan2(b, norm)*32768/M_PI));

	next_fn();
}

TGP_FUNCTION( model1_state::f43 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	logerror("TGP f43 %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, m_pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

TGP_FUNCTION( model1_state::f43_swa )
{
	float a = fifoin_pop_f();
	int b = fifoin_pop();
	int c = fifoin_pop();
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP f43_swa %f, %d, %d (%x)\n", a, b, c, m_pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

TGP_FUNCTION( model1_state::f44 )
{
	float a = fifoin_pop_f();
	(void)a;
	logerror("TGP f44 %f (%x)\n", a, m_pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_sdir )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float norm = sqrt(a*a+b*b+c*c);
	float t[9], m[9];
	logerror("TGP matrix_sdir %f, %f, %f (%x)\n", a, b, c, m_pushpc);

	memset(t, 0, sizeof(t));

	if(!norm) {
		memset(t, 0, 9*sizeof(t[0]));
		t[0] = 1.0;
		t[4] = 1.0;
		t[8] = 1.0;
	} else {
		t[0] = -c / norm;
		t[1] = b / norm;
		t[2] = a / norm;

		norm = sqrt(a*a+c*c);
		t[6] = a/norm;
		t[7] = 0;
		t[8] = c/norm;

		t[3] = -b*c;
		t[4] = a*a+c*c;
		t[5] = -b*a;
		norm = sqrt(t[3]*t[3]+t[4]*t[4]+t[5]*t[5]);
		t[3] /= norm;
		t[4] /= norm;
		t[5] /= norm;
	}

	m[0]  = t[0]*m_cmat[0] + t[1]*m_cmat[3] + t[2]*m_cmat[6];
	m[1]  = t[0]*m_cmat[1] + t[1]*m_cmat[4] + t[2]*m_cmat[7];
	m[2]  = t[0]*m_cmat[2] + t[1]*m_cmat[5] + t[2]*m_cmat[8];
	m[3]  = t[3]*m_cmat[0] + t[4]*m_cmat[3] + t[5]*m_cmat[6];
	m[4]  = t[3]*m_cmat[1] + t[4]*m_cmat[4] + t[5]*m_cmat[7];
	m[5]  = t[3]*m_cmat[2] + t[4]*m_cmat[5] + t[5]*m_cmat[8];
	m[6]  = t[6]*m_cmat[0] + t[7]*m_cmat[3] + t[8]*m_cmat[6];
	m[7]  = t[6]*m_cmat[1] + t[7]*m_cmat[4] + t[8]*m_cmat[7];
	m[8]  = t[6]*m_cmat[2] + t[7]*m_cmat[5] + t[8]*m_cmat[8];

	memcpy(m_cmat, m, sizeof(m));

	next_fn();
}

TGP_FUNCTION( model1_state::f45 )
{
	float a = fifoin_pop_f();
	(void)a;
	logerror("TGP f45 %f (%x)\n", a, m_pushpc);
	fifoout_push_f(0);
	next_fn();
}

TGP_FUNCTION( model1_state::vlength )
{
	float a = fifoin_pop_f() - m_tgp_vr_base[0];
	float b = fifoin_pop_f() - m_tgp_vr_base[1];
	float c = fifoin_pop_f() - m_tgp_vr_base[2];
	logerror("TGP vlength %f, %f, %f (%x)\n", a, b, c, m_pushpc);

	a = (a*a+b*b+c*c);
	b = 1/sqrt(a);
	c = a * b;
	c -= m_tgp_vr_base[3];
	fifoout_push_f(c);
	next_fn();
}

TGP_FUNCTION( model1_state::f47 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	logerror("TGP f47 %f, %f, %f (%x)\n", a, b, c, m_pushpc);
	fifoout_push_f(a+c);
	fifoout_push_f(b+c);
	next_fn();
}

TGP_FUNCTION( model1_state::track_read_info )
{
	const UINT32 *tgp_data = (const UINT32 *)memregion("user2")->base();
	UINT16 a = fifoin_pop();
	int offd;

	logerror("TGP track_read_info %d (%x)\n", a, m_pushpc);

	offd = tgp_data[0x20+m_tgp_vr_select] + 16*a;
	fifoout_push(tgp_data[offd+15]);
	next_fn();
}

TGP_FUNCTION( model1_state::colbox_set )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	float g = fifoin_pop_f();
	float h = fifoin_pop_f();
	float i = fifoin_pop_f();
	float j = fifoin_pop_f();
	float k = fifoin_pop_f();
	float l = fifoin_pop_f();
	logerror("TGP colbox_set %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, m_pushpc);
	m_tgp_vr_cbox[ 0] = a;
	m_tgp_vr_cbox[ 1] = b;
	m_tgp_vr_cbox[ 2] = c;
	m_tgp_vr_cbox[ 3] = d;
	m_tgp_vr_cbox[ 4] = e;
	m_tgp_vr_cbox[ 5] = f;
	m_tgp_vr_cbox[ 6] = g;
	m_tgp_vr_cbox[ 7] = h;
	m_tgp_vr_cbox[ 8] = i;
	m_tgp_vr_cbox[ 9] = j;
	m_tgp_vr_cbox[10] = k;
	m_tgp_vr_cbox[11] = l;
	next_fn();
}

TGP_FUNCTION( model1_state::colbox_test )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP colbox_test %f, %f, %f (%x)\n", a, b, c, m_pushpc);

	// #### Wrong, need to check with the tgp_vr_cbox coordinates
	// Game only test sign, negative = collision
	fifoout_push_f(-1);
	next_fn();
}

TGP_FUNCTION( model1_state::f49_swa )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	logerror("TGP f49_swa %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::f50_swa )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	logerror("TGP f50_swa %f, %f, %f, %f (%x)\n", a, b, c, d, m_pushpc);
	fifoout_push_f(d);
	next_fn();
}

TGP_FUNCTION( model1_state::f52 )
{
	logerror("TGP f52 (%x)\n", m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_rdir )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float norm = sqrt(a*a+c*c);
	float t1, t2;
	(void)b;

	logerror("TGP matrix_rdir %f, %f, %f (%x)\n", a, b, c, m_pushpc);

	if(!norm) {
		c = 1;
		a = 0;
	} else {
		c /= norm;
		a /= norm;
	}

	t1 = m_cmat[6];
	t2 = m_cmat[0];
	m_cmat[6] = c*t1-a*t2;
	m_cmat[0] = a*t1+c*t2;
	t1 = m_cmat[7];
	t2 = m_cmat[1];
	m_cmat[7] = c*t1-a*t2;
	m_cmat[1] = a*t1+c*t2;
	t1 = m_cmat[8];
	t2 = m_cmat[2];
	m_cmat[8] = c*t1-a*t2;
	m_cmat[2] = a*t1+c*t2;
	next_fn();
}

// A+(B-A)*t1 + (C-A)*t2 = P
static void tri_calc_pq(float ax, float ay, float bx, float by, float cx, float cy, float px, float py, float *t1, float *t2)
{
	float d;
	bx -= ax;
	cx -= ax;
	px -= ax;
	by -= ay;
	cy -= ay;
	py -= ay;
	d = bx*cy-by*cx;
	*t1 = (px*cy-py*cx)/d;
	*t2 = (bx*py-by*px)/d;
}

TGP_FUNCTION( model1_state::track_lookup )
{
	const UINT32 *tgp_data = (const UINT32 *)memregion("user2")->base();
	float a = fifoin_pop_f();
	UINT32 b = fifoin_pop();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	int offi, offd, len;
	float dist;
	int i;
	UINT32 entry;
	float height;

	logerror("TGP track_lookup %f, 0x%x, %f, %f (%x)\n", a, b, c, d, m_pushpc);

	offi = tgp_data[0x10+m_tgp_vr_select] + b;
	offd = tgp_data[0x20+m_tgp_vr_select];

	len = tgp_data[offi++];

	dist = -1;

//  behaviour = 0;
	height = 0.0;
	entry = 0;

	for(i=0; i<len; i++) {
		int j;
		int bpos = tgp_data[offi++];
		int posd = offd + bpos*0x10;
		const float *pts = (const float *)(tgp_data+posd);
		float ax = pts[12];
		float ay = pts[14];
		float az = pts[13];
		for(j=0; j<4; j++) {
			float t1, t2;
			int k = (j+1) & 3;
			tri_calc_pq(ax, ay, pts[3*j], pts[3*j+2], pts[3*k], pts[3*k+2], c, d, &t1, &t2);
			if(t1 >= 0 && t2 >= 0 && t1+t2 <= 1) {
				float z = az+t1*(pts[3*j+1]-az)+t2*(pts[3*k+1]-az);
				float d = (a-z)*(a-z);
				if(dist == -1 || d<dist) {
					dist = d;
//                  behaviour = tgp_data[posd+15];
					height = z;
					entry = bpos+i;
				}
			}
		}
	}

	m_ram_data[0x0000] = 0; // non zero = still computing
	m_ram_data[0x8001] = f2u(height);
	m_ram_data[0x8002] = entry;

	next_fn();
}

TGP_FUNCTION( model1_state::f56 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	UINT32 g = fifoin_pop();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	(void)g;

	logerror("TGP f56 %f, %f, %f, %f, %f, %f, %d (%x)\n", a, b, c, d, e, f, g, m_pushpc);
	fifoout_push(0);
	next_fn();
}

TGP_FUNCTION( model1_state::f57 )
{
	logerror("TGP f57 (%x)\n", m_pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_readt )
{
	logerror("TGP matrix_readt (%x)\n", m_pushpc);
	fifoout_push_f(m_cmat[9]);
	fifoout_push_f(m_cmat[10]);
	fifoout_push_f(m_cmat[11]);
	next_fn();
}

TGP_FUNCTION( model1_state::acc_geti )
{
	logerror("TGP acc_geti (%x)\n", m_pushpc);
	fifoout_push((int)m_acc);
	next_fn();
}

TGP_FUNCTION( model1_state::f60 )
{
	logerror("TGP f60 (%x)\n", m_pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

TGP_FUNCTION( model1_state::col_setcirc )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	logerror("TGP col_setcirc %f, %f, %f (%x)\n", a, b, c, m_pushpc);
	m_tgp_vr_circx = a;
	m_tgp_vr_circy = b;
	m_tgp_vr_circrad = c;
	next_fn();
}

TGP_FUNCTION( model1_state::col_testpt )
{
	float x, y;
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	logerror("TGP col_testpt %f, %f (%x)\n", a, b, m_pushpc);
	x = a - m_tgp_vr_circx;
	y = b - m_tgp_vr_circy;
	fifoout_push_f(((x*x+y*y)/sqrtf(x*x+y*y)) - m_tgp_vr_circrad);
	next_fn();
}

TGP_FUNCTION( model1_state::push_and_ident )
{
	if(m_mat_stack_pos != MAT_STACK_SIZE) {
		memcpy(m_mat_stack[m_mat_stack_pos], m_cmat, sizeof(m_cmat));
		m_mat_stack_pos++;
	}
	logerror("TGP push_and_ident (depth=%d, pc=%x)\n", m_mat_stack_pos, m_pushpc);
	memset(m_cmat, 0, sizeof(m_cmat));
	m_cmat[0] = 1.0;
	m_cmat[4] = 1.0;
	m_cmat[8] = 1.0;
	next_fn();
}

TGP_FUNCTION( model1_state::catmull_rom )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	float g = fifoin_pop_f();
	float h = fifoin_pop_f();
	float i = fifoin_pop_f();
	float j = fifoin_pop_f();
	float k = fifoin_pop_f();
	float l = fifoin_pop_f();
	float m = fifoin_pop_f();
	float m2, m3;
	float w1, w2, w3, w4;

	logerror("TGP catmull_rom %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, m, m_pushpc);

	m2 = m*m;
	m3 = m*m*m;

	w1 = 0.5f*(-m3+2*m2-m);
	w2 = 0.5f*(3*m3-5*m2+2);
	w3 = 0.5f*(-3*m3+4*m2+m);
	w4 = 0.5f*(m3-m2);

	fifoout_push_f(a*w1+d*w2+g*w3+j*w4);
	fifoout_push_f(b*w1+e*w2+h*w3+k*w4);
	fifoout_push_f(c*w1+f*w2+i*w3+l*w4);
	next_fn();
}

TGP_FUNCTION( model1_state::distance )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	logerror("TGP distance (%f, %f), (%f, %f) (%x)\n", a, b, c, d, m_pushpc);
	c -= a;
	d -= b;
	fifoout_push_f((c*c+d*d)/sqrtf(c*c+d*d));
	next_fn();
}

TGP_FUNCTION( model1_state::car_move )
{
	INT16 a = fifoin_pop();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float dx, dy;
	logerror("TGP car_move (%d, %f), (%f, %f) (%x)\n", a, b, c, d, m_pushpc);

	dx = b*tsin(a);
	dy = b*tcos(a);

	fifoout_push_f(dx);
	fifoout_push_f(dy);
	fifoout_push_f(c+dx);
	fifoout_push_f(d+dy);
	next_fn();
}

TGP_FUNCTION( model1_state::cpa )
{
	float dv_x, dv_y, dv_z, dv2, dw_x, dw_y, dw_z, dt;

	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	float g = fifoin_pop_f();
	float h = fifoin_pop_f();
	float i = fifoin_pop_f();
	float j = fifoin_pop_f();
	float k = fifoin_pop_f();
	float l = fifoin_pop_f();
	logerror("TGP cpa %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, m_pushpc);

	dv_x = (b-a) - (d-c);
	dv_y = (f-e) - (h-g);
	dv_z = (j-i) - (l-k);
	dv2 = dv_x*dv_x + dv_y*dv_y + dv_z*dv_z;
	if(dv2 < 0.001f)
		dt = 0;
	else {
		dw_x = a-c;
		dw_y = e-g;
		dw_z = i-k;
		dt = -(dw_x*dv_x + dw_y*dv_y + dw_z*dv_z)/dv2;
	}
	if(dt < 0)
		dt = 0;
	else if(dt > 1.0f)
		dt = 1.0f;

	dv_x = (a-c)*(1-dt) + (b-d)*dt;
	dv_y = (e-g)*(1-dt) + (f-h)*dt;
	dv_z = (i-k)*(1-dt) + (j-l)*dt;
	dv2 = dv_x*dv_x + dv_y*dv_y + dv_z*dv_z;

	fifoout_push_f(sqrt(dv2));
	next_fn();
}

TGP_FUNCTION( model1_state::vmat_store )
{
	UINT32 a = fifoin_pop();
	if(a<21)
		memcpy(m_mat_vector[a], m_cmat, sizeof(m_cmat));
	else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_store %d (%x)\n", a, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::vmat_restore )
{
	UINT32 a = fifoin_pop();
	if(a<21)
		memcpy(m_cmat, m_mat_vector[a], sizeof(m_cmat));
	else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_restore %d (%x)\n", a, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::vmat_mul )
{
	UINT32 a = fifoin_pop();
	UINT32 b = fifoin_pop();
	if(a<21 && b<21) {
		m_mat_vector[b][0]  = m_mat_vector[a][ 0]*m_cmat[0] + m_mat_vector[a][ 1]*m_cmat[3] + m_mat_vector[a][ 2]*m_cmat[6];
		m_mat_vector[b][1]  = m_mat_vector[a][ 0]*m_cmat[1] + m_mat_vector[a][ 1]*m_cmat[4] + m_mat_vector[a][ 2]*m_cmat[7];
		m_mat_vector[b][2]  = m_mat_vector[a][ 0]*m_cmat[2] + m_mat_vector[a][ 1]*m_cmat[5] + m_mat_vector[a][ 2]*m_cmat[8];
		m_mat_vector[b][3]  = m_mat_vector[a][ 3]*m_cmat[0] + m_mat_vector[a][ 4]*m_cmat[3] + m_mat_vector[a][ 5]*m_cmat[6];
		m_mat_vector[b][4]  = m_mat_vector[a][ 3]*m_cmat[1] + m_mat_vector[a][ 4]*m_cmat[4] + m_mat_vector[a][ 5]*m_cmat[7];
		m_mat_vector[b][5]  = m_mat_vector[a][ 3]*m_cmat[2] + m_mat_vector[a][ 4]*m_cmat[5] + m_mat_vector[a][ 5]*m_cmat[8];
		m_mat_vector[b][6]  = m_mat_vector[a][ 6]*m_cmat[0] + m_mat_vector[a][ 7]*m_cmat[3] + m_mat_vector[a][ 8]*m_cmat[6];
		m_mat_vector[b][7]  = m_mat_vector[a][ 6]*m_cmat[1] + m_mat_vector[a][ 7]*m_cmat[4] + m_mat_vector[a][ 8]*m_cmat[7];
		m_mat_vector[b][8]  = m_mat_vector[a][ 6]*m_cmat[2] + m_mat_vector[a][ 7]*m_cmat[5] + m_mat_vector[a][ 8]*m_cmat[8];
		m_mat_vector[b][9]  = m_mat_vector[a][ 9]*m_cmat[0] + m_mat_vector[a][10]*m_cmat[3] + m_mat_vector[a][11]*m_cmat[6] + m_cmat[9];
		m_mat_vector[b][10] = m_mat_vector[a][ 9]*m_cmat[1] + m_mat_vector[a][10]*m_cmat[4] + m_mat_vector[a][11]*m_cmat[7] + m_cmat[10];
		m_mat_vector[b][11] = m_mat_vector[a][ 9]*m_cmat[2] + m_mat_vector[a][10]*m_cmat[5] + m_mat_vector[a][11]*m_cmat[8] + m_cmat[11];
	} else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_mul %d, %d (%x)\n", a, b, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::vmat_read )
{
	UINT32 a = fifoin_pop();
	logerror("TGP vmat_read %d (%x)\n", a, m_pushpc);
	if(a<21) {
		int i;
		for(i=0; i<12; i++)
			fifoout_push_f(m_mat_vector[a][i]);
	} else {
		int i;
		logerror("TGP ERROR bad vector index\n");
		for(i=0; i<12; i++)
			fifoout_push_f(0);
	}
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_rtrans )
{
	logerror("TGP matrix_rtrans (%x)\n", m_pushpc);
	fifoout_push_f(m_cmat[ 9]);
	fifoout_push_f(m_cmat[10]);
	fifoout_push_f(m_cmat[11]);
	next_fn();
}

TGP_FUNCTION( model1_state::matrix_unrot )
{
	logerror("TGP matrix_unrot (%x)\n", m_pushpc);
	memset(m_cmat, 0, 9*sizeof(m_cmat[0]));
	m_cmat[0] = 1.0;
	m_cmat[4] = 1.0;
	m_cmat[8] = 1.0;
	next_fn();
}

TGP_FUNCTION( model1_state::f80 )
{
	logerror("TGP f80 (%x)\n", m_pushpc);
	//  m_cmat[9] = m_cmat[10] = m_cmat[11] = 0;
	next_fn();
}

TGP_FUNCTION( model1_state::vmat_save )
{
	UINT32 a = fifoin_pop();
	int i;
	logerror("TGP vmat_save 0x%x (%x)\n", a, m_pushpc);
	for(i=0; i<16; i++)
		memcpy(m_ram_data+a+0x10*i, m_mat_vector[i], sizeof(m_cmat));
	next_fn();
}

TGP_FUNCTION( model1_state::vmat_load )
{
	UINT32 a = fifoin_pop();
	int i;
	logerror("TGP vmat_load 0x%x (%x)\n", a, m_pushpc);
	for(i=0; i<16; i++)
		memcpy(m_mat_vector[i], m_ram_data+a+0x10*i, sizeof(m_cmat));
	next_fn();
}

TGP_FUNCTION( model1_state::ram_setadr )
{
	m_ram_scanadr = fifoin_pop() - 0x8000;
	logerror("TGP f0 ram_setadr 0x%x (%x)\n", m_ram_scanadr+0x8000, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::groundbox_test )
{
	int out_x, out_y, out_z;
	float x, /*y,*/ z;
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();

	logerror("TGP groundbox_test %f, %f, %f (%x)\n", a, b, c, m_pushpc);
	x = m_cmat[0]*a+m_cmat[3]*b+m_cmat[6]*c+m_cmat[9];
	//y = m_cmat[1]*a+m_cmat[4]*b+m_cmat[7]*c+m_cmat[10];
	z = m_cmat[2]*a+m_cmat[5]*b+m_cmat[8]*c+m_cmat[11];

	out_x = x < m_tgp_vf_xmin || x > m_tgp_vf_xmax;
	out_z = z < m_tgp_vf_zmin || z > m_tgp_vf_zmax;
	out_y = 1; // Wrong, but untestable it seems.

	fifoout_push(out_x);
	fifoout_push(out_y);
	fifoout_push(out_z);
	next_fn();
}

TGP_FUNCTION( model1_state::f89 )
{
	UINT32 a = fifoin_pop();
	UINT32 b = fifoin_pop();
	UINT32 c = fifoin_pop();
	UINT32 d = fifoin_pop();
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP list set base 0x%x, 0x%x, %d, length=%d (%x)\n", a, b, c, d, m_pushpc);
	m_list_length = d;
	next_fn();
}

TGP_FUNCTION( model1_state::f92 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	logerror("TGP f92 %f, %f, %f, %f (%x)\n", a, b, c, d, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::f93 )
{
	float a = fifoin_pop_f();
	(void)a;
	logerror("TGP f93 %f (%x)\n", a, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::f94 )
{
	UINT32 a = fifoin_pop();
	(void)a;
	logerror("TGP f94 %d (%x)\n", a, m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::vmat_flatten )
{
	int i;
	float m[12];
	logerror("TGP vmat_flatten (%x)\n", m_pushpc);

	for(i=0; i<16; i++) {
		memcpy(m, m_mat_vector[i], sizeof(m_cmat));
		m[1] = m[4] = m[7] = m[10] = 0;

		m_mat_vector[i][0]  = m[ 0]*m_cmat[0] + m[ 1]*m_cmat[3] + m[ 2]*m_cmat[6];
		m_mat_vector[i][1]  = m[ 0]*m_cmat[1] + m[ 1]*m_cmat[4] + m[ 2]*m_cmat[7];
		m_mat_vector[i][2]  = m[ 0]*m_cmat[2] + m[ 1]*m_cmat[5] + m[ 2]*m_cmat[8];
		m_mat_vector[i][3]  = m[ 3]*m_cmat[0] + m[ 4]*m_cmat[3] + m[ 5]*m_cmat[6];
		m_mat_vector[i][4]  = m[ 3]*m_cmat[1] + m[ 4]*m_cmat[4] + m[ 5]*m_cmat[7];
		m_mat_vector[i][5]  = m[ 3]*m_cmat[2] + m[ 4]*m_cmat[5] + m[ 5]*m_cmat[8];
		m_mat_vector[i][6]  = m[ 6]*m_cmat[0] + m[ 7]*m_cmat[3] + m[ 8]*m_cmat[6];
		m_mat_vector[i][7]  = m[ 6]*m_cmat[1] + m[ 7]*m_cmat[4] + m[ 8]*m_cmat[7];
		m_mat_vector[i][8]  = m[ 6]*m_cmat[2] + m[ 7]*m_cmat[5] + m[ 8]*m_cmat[8];
		m_mat_vector[i][9]  = m[ 9]*m_cmat[0] + m[10]*m_cmat[3] + m[11]*m_cmat[6] + m_cmat[9];
		m_mat_vector[i][10] = m[ 9]*m_cmat[1] + m[10]*m_cmat[4] + m[11]*m_cmat[7] + m_cmat[10];
		m_mat_vector[i][11] = m[ 9]*m_cmat[2] + m[10]*m_cmat[5] + m[11]*m_cmat[8] + m_cmat[11];
	}
	next_fn();
}

TGP_FUNCTION( model1_state::vmat_load1 )
{
	UINT32 a = fifoin_pop();
	logerror("TGP vmat_load1 0x%x (%x)\n", a, m_pushpc);
	memcpy(m_cmat, m_ram_data+a, sizeof(m_cmat));
	next_fn();
}

TGP_FUNCTION( model1_state::ram_trans )
{
	float a = ram_get_f();
	float b = ram_get_f();
	float c = ram_get_f();
	logerror("TGP ram_trans (%x)\n", m_pushpc);
	m_cmat[ 9] += m_cmat[0]*a+m_cmat[3]*b+m_cmat[6]*c;
	m_cmat[10] += m_cmat[1]*a+m_cmat[4]*b+m_cmat[7]*c;
	m_cmat[11] += m_cmat[2]*a+m_cmat[5]*b+m_cmat[8]*c;
	next_fn();
}

TGP_FUNCTION( model1_state::f98_load )
{
	int i;
	for(i=0; i<m_list_length; i++) {
		float f = fifoin_pop_f();
		(void)f;
		logerror("TGP load list (%2d/%2d) %f (%x)\n", i, m_list_length, f, m_pushpc);
	}
	next_fn();
}

TGP_FUNCTION( model1_state::f98 )
{
	UINT32 a = fifoin_pop();
	(void)a;
	logerror("TGP load list start %d (%x)\n", a, m_pushpc);
	m_fifoin_cbcount = m_list_length;
	m_fifoin_cb = &model1_state::f98_load;
}

TGP_FUNCTION( model1_state::f99 )
{
	logerror("TGP f99 (%x)\n", m_pushpc);
	next_fn();
}

TGP_FUNCTION( model1_state::f100 )
{
	int i;
	logerror("TGP f100 get list (%x)\n", m_pushpc);
	for(i=0; i<m_list_length; i++)
		fifoout_push_f((machine().rand() % 1000)/100.0);
	next_fn();
}

TGP_FUNCTION( model1_state::groundbox_set )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	float g = fifoin_pop_f();
	logerror("TGP groundbox_set %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, m_pushpc);
	m_tgp_vf_xmin = e;
	m_tgp_vf_xmax = d;
	m_tgp_vf_zmin = g;
	m_tgp_vf_zmax = f;
	m_tgp_vf_ygnd = b;
	m_tgp_vf_yflr = a;
	m_tgp_vf_yjmp = c;

	next_fn();
}

TGP_FUNCTION( model1_state::f102 )
{
	float px, py, pz;
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	UINT32 f = fifoin_pop();
	UINT32 g = fifoin_pop();
	UINT32 h = fifoin_pop();

	m_ccount++;

	logerror("TGP f0 mve_calc %f, %f, %f, %f, %f, %d, %d, %d (%d) (%x)\n", a, b, c, d, e, f, g, h, m_ccount, m_pushpc);

	px = u2f(m_ram_data[m_ram_scanadr+0x16]);
	py = u2f(m_ram_data[m_ram_scanadr+0x17]);
	pz = u2f(m_ram_data[m_ram_scanadr+0x18]);

	//  memset(m_cmat, 0, sizeof(m_cmat));
	//  m_cmat[0] = 1.0;
	//  m_cmat[4] = 1.0;
	//  m_cmat[8] = 1.0;

	px = c;
	py = d;
	pz = e;

#if 1
	m_cmat[ 9] += m_cmat[0]*a+m_cmat[3]*b+m_cmat[6]*c;
	m_cmat[10] += m_cmat[1]*a+m_cmat[4]*b+m_cmat[7]*c;
	m_cmat[11] += m_cmat[2]*a+m_cmat[5]*b+m_cmat[8]*c;
#else
	m_cmat[ 9] += px;
	m_cmat[10] += py;
	m_cmat[11] += pz;
#endif

	logerror("    f0 mve_calc %f, %f, %f\n", px, py, pz);

	fifoout_push_f(c);
	fifoout_push_f(d);
	fifoout_push_f(e);
	fifoout_push(f);
	fifoout_push(g);
	fifoout_push(h);


	next_fn();
}

TGP_FUNCTION( model1_state::f103 )
{
	m_ram_scanadr = fifoin_pop() - 0x8000;
	logerror("TGP f0 mve_setadr 0x%x (%x)\n", m_ram_scanadr, m_pushpc);
	ram_get_i();
	next_fn();
}

const struct model1_state::function model1_state::ftab_vf[] = {
	{ &model1_state::fadd,            2 }, /* 0x00 */
	{ &model1_state::fsub,            2 },
	{ &model1_state::fmul,            2 },
	{ &model1_state::fdiv,            2 },
	{ NULL,                           0 },
	{ &model1_state::matrix_push,     0 },
	{ &model1_state::matrix_pop,      0 },
	{ &model1_state::matrix_write,   12 },
	{ &model1_state::clear_stack,     0 },
	{ NULL,                           0 },
	{ &model1_state::anglev,          2 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ &model1_state::track_select,    1 },
	{ &model1_state::f14,             4 },
	{ &model1_state::anglep,          4 },

	{ &model1_state::matrix_ident,    0 },  /* 0x10 */
	{ &model1_state::matrix_read,     0 },
	{ &model1_state::matrix_trans,    3 },
	{ &model1_state::matrix_scale,    3 },
	{ &model1_state::matrix_rotx,     1 },
	{ &model1_state::matrix_roty,     1 },
	{ &model1_state::matrix_rotz,     1 },
	{ NULL,                           0 },
	{ &model1_state::track_read_quad, 1 },
	{ NULL,                           0 },
	{ &model1_state::transform_point, 3 },
	{ &model1_state::fsin_m1,         1 },
	{ &model1_state::fcos_m1,         1 },
	{ &model1_state::fsinm_m1,        2 },
	{ &model1_state::fcosm_m1,        2 },
	{ &model1_state::distance3,       6 },

	{ NULL,                           0 },  /* 0x20 */
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ &model1_state::acc_set,         1 },
	{ &model1_state::acc_get,         0 },
	{ &model1_state::acc_add,         1 },
	{ &model1_state::acc_sub,         1 },
	{ &model1_state::acc_mul,         1 },
	{ &model1_state::acc_div,         1 }, // not used ?
	{ &model1_state::f42,             3 },
	{ &model1_state::f43,             6 },
	{ &model1_state::f44,             1 },
	{ &model1_state::f45,             1 },
	{ &model1_state::vlength,         3 },
	{ NULL,                           0 },

	{ &model1_state::track_read_info, 1 },  /* 0x30 */
	{ &model1_state::colbox_set,     12 },
	{ &model1_state::colbox_test,     3 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ &model1_state::track_lookup,    4 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },

	{ &model1_state::col_setcirc,     3 },  /* 0x40 */
	{ &model1_state::col_testpt,      2 },
	{ NULL,                           0 },
	{ &model1_state::distance,        4 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ &model1_state::car_move,        4 },
	{ &model1_state::cpa,            12 },
	{ NULL,                           0 },
	{ &model1_state::vmat_store,      1 },
	{ &model1_state::vmat_restore,    1 },
	{ NULL,                           0 },
	{ &model1_state::vmat_mul,        2 },
	{ &model1_state::vmat_read,       1 },
	{ &model1_state::matrix_unrot,    0 },

	{ &model1_state::f80,             0 },  /* 0x50 */
	{ NULL,                           0 },
	{ &model1_state::matrix_rtrans,   0 },
	{ NULL,                           0 },
	{ &model1_state::vmat_save,       1 },
	{ &model1_state::vmat_load,       1 },
	{ &model1_state::ram_setadr,      1 },
	{ &model1_state::groundbox_test,  3 },
	{ NULL,                           0 },
	{ &model1_state::f89,             4 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ &model1_state::f92,             4 },
	{ &model1_state::f93,             1 },
	{ &model1_state::f94,             1 },
	{ &model1_state::vmat_flatten,    0 },

	{ &model1_state::vmat_load1,      1 },  /* 0x60 */
	{ &model1_state::ram_trans,       0 },
	{ &model1_state::f98,             1 },
	{ &model1_state::f99,             0 },
	{ &model1_state::f100,            0 },
	{ &model1_state::groundbox_set,   7 },
	{ &model1_state::f102,            8 },
	{ &model1_state::f103,            1 }
};

// Used in swa scene 1 and unemulated:
//   f14
//   f49_swa
//   f15_swa

const struct model1_state::function model1_state::ftab_swa[] = {
	{ &model1_state::fadd,            2 },  /* 0x00 */
	{ &model1_state::fsub,            2 },
	{ &model1_state::fmul,            2 },
	{ &model1_state::fdiv,            2 },
	{ NULL,                           0 },
	{ &model1_state::matrix_push,     0 },
	{ &model1_state::matrix_pop,      0 },
	{ &model1_state::matrix_write,   12 },
	{ &model1_state::clear_stack,     0 },
	{ &model1_state::matrix_mul,     12 },
	{ &model1_state::anglev,          2 },
	{ &model1_state::f11,             9 },
	{ &model1_state::normalize,       3 },
	{ &model1_state::acc_seti,        1 },
	{ &model1_state::f14,             4 },
	{ &model1_state::f15_swa,         0 },

	{ &model1_state::matrix_ident,    0 }, /* 0x10 */
	{ &model1_state::matrix_read,     0 },
	{ &model1_state::matrix_trans,    3 },
	{ &model1_state::matrix_scale,    3 },
	{ &model1_state::matrix_rotx,     1 },
	{ &model1_state::matrix_roty,     1 },
	{ &model1_state::matrix_rotz,     1 },
	{ NULL,                           0 },
	{ &model1_state::f24_swa,         7 },
	{ NULL,                           0 },
	{ &model1_state::transform_point, 3 },
	{ &model1_state::fsin_m1,         1 },
	{ &model1_state::fcos_m1,         1 },
	{ &model1_state::fsinm_m1,        2 },
	{ &model1_state::fcosm_m1,        2 },
	{ &model1_state::distance3,       6 },

	{ NULL,                           0 }, /* 0x20 */
	{ NULL,                           0 },
	{ &model1_state::ftoi,            1 },
	{ &model1_state::itof,            1 },
	{ &model1_state::acc_set,         1 },
	{ &model1_state::acc_get,         0 },
	{ &model1_state::acc_add,         1 },
	{ &model1_state::acc_sub,         1 },
	{ &model1_state::acc_mul,         1 },
	{ &model1_state::acc_div,         1 }, // not used ?
	{ &model1_state::xyz2rqf,         3 },
	{ &model1_state::f43_swa,         3 },
	{ &model1_state::matrix_sdir,     3 },
	{ &model1_state::f45,             1 },
	{ &model1_state::vlength,         3 },
	{ &model1_state::f47,             3 },

	{ NULL,                           0 }, /* 0x30 */
	{ &model1_state::f49_swa,         6 },
	{ &model1_state::f50_swa,         4 },
	{ NULL,                           0 },
	{ &model1_state::f52,             0 },
	{ &model1_state::matrix_rdir,     3 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ &model1_state::f56,             7 },
	{ &model1_state::f57,             0 },
	{ &model1_state::matrix_readt,    0 },
	{ &model1_state::acc_geti,        0 },
	{ &model1_state::f60,             0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },
	{ NULL,                           0 },

	{ &model1_state::push_and_ident,  0 }, /* 0x40 */
	{ NULL,                           0 },
	{ &model1_state::catmull_rom,    13 }
};


TGP_FUNCTION( model1_state::dump )
{
	logerror("TGP FIFOIN write %08x (%x)\n", fifoin_pop(), m_pushpc);
	m_fifoin_cbcount = 1;
	m_fifoin_cb = &model1_state::dump;
}

TGP_FUNCTION( model1_state::function_get_vf )
{
	UINT32 f = fifoin_pop() >> 23;

	if(m_fifoout_rpos != m_fifoout_wpos) {
		int count = m_fifoout_wpos - m_fifoout_rpos;
		if(count < 0)
			count += FIFO_SIZE;
		logerror("TGP function called with sizeout = %d\n", count);
	}
	if(ARRAY_LENGTH(ftab_vf) > f && NULL != ftab_vf[f].cb) {
		m_fifoin_cbcount = ftab_vf[f].count;
		m_fifoin_cb = model1_state::ftab_vf[f].cb;
		//      logerror("TGP function %d request, %d parameters\n", f, m_fifoin_cbcount);
		if(!m_fifoin_cbcount)
			(this->*m_fifoin_cb)();
	} else {
		logerror("TGP function %d unimplemented (%x)\n", f, m_pushpc);
		m_fifoin_cbcount = 1;
		m_fifoin_cb = &model1_state::dump;
	}
}

TGP_FUNCTION( model1_state::function_get_swa )
{
	UINT32 f = fifoin_pop();

	if(m_fifoout_rpos != m_fifoout_wpos) {
		int count = m_fifoout_wpos - m_fifoout_rpos;
		if(count < 0)
			count += FIFO_SIZE;
		logerror("TGP function called with sizeout = %d\n", count);
	}
	if(ARRAY_LENGTH(ftab_swa) > f && NULL != ftab_swa[f].cb) {
		m_fifoin_cbcount = ftab_swa[f].count;
		m_fifoin_cb = model1_state::ftab_swa[f].cb;
		//      logerror("TGP function %d request, %d parameters\n", f, m_fifoin_cbcount);
		if(!m_fifoin_cbcount)
			(this->*m_fifoin_cb)();
	} else {
		logerror("TGP function %d unimplemented (%x)\n", f, m_pushpc);
		m_fifoin_cbcount = 1;
		m_fifoin_cb = &model1_state::dump;
	}
}

READ16_MEMBER(model1_state::model1_tgp_copro_r)
{
	if(!offset) {
		m_copro_r = fifoout_pop();
		return m_copro_r;
	} else
		return m_copro_r >> 16;
}

WRITE16_MEMBER(model1_state::model1_tgp_copro_w)
{
	if(offset) {
		m_copro_w = (m_copro_w & 0x0000ffff) | (data << 16);
		m_pushpc = space.device().safe_pc();
		fifoin_push(m_copro_w);
	} else
		m_copro_w = (m_copro_w & 0xffff0000) | data;
}

READ16_MEMBER(model1_state::model1_tgp_copro_adr_r)
{
	return m_ram_adr;
}

WRITE16_MEMBER(model1_state::model1_tgp_copro_adr_w)
{
	COMBINE_DATA(&m_ram_adr);
}

READ16_MEMBER(model1_state::model1_tgp_copro_ram_r)
{
	if(!offset) {
		logerror("TGP f0 ram read %04x, %08x (%f) (%x)\n", m_ram_adr, m_ram_data[m_ram_adr], u2f(m_ram_data[m_ram_adr]), space.device().safe_pc());
		return m_ram_data[m_ram_adr];
	} else
		return m_ram_data[m_ram_adr++] >> 16;
}

WRITE16_MEMBER(model1_state::model1_tgp_copro_ram_w)
{
	COMBINE_DATA(m_ram_latch+offset);
	if(offset) {
		UINT32 v = m_ram_latch[0]|(m_ram_latch[1]<<16);
		logerror("TGP f0 ram write %04x, %08x (%f) (%x)\n", m_ram_adr, v, u2f(v), space.device().safe_pc());
		m_ram_data[m_ram_adr] = v;
		m_ram_adr++;
	}
}

MACHINE_START_MEMBER(model1_state,model1)
{
	m_ram_data = auto_alloc_array(machine(), UINT32, 0x10000);

	save_pointer(NAME(m_ram_data), 0x10000);
	save_item(NAME(m_ram_adr));
	save_item(NAME(m_ram_scanadr));
	save_item(NAME(m_ram_latch));
	save_item(NAME(m_fifoout_rpos));
	save_item(NAME(m_fifoout_wpos));
	save_item(NAME(m_fifoout_data));
	save_item(NAME(m_fifoin_rpos));
	save_item(NAME(m_fifoin_wpos));
	save_item(NAME(m_fifoin_data));
	save_item(NAME(m_cmat));
	save_item(NAME(m_mat_stack));
	save_item(NAME(m_mat_vector));
	save_item(NAME(m_mat_stack_pos));
	save_item(NAME(m_acc));
	save_item(NAME(m_list_length));
}

void model1_state::tgp_reset(int swa)
{
	m_ram_adr = 0;
	memset(m_ram_data, 0, 0x10000*4);

	m_fifoout_rpos = 0;
	m_fifoout_wpos = 0;
	m_fifoin_rpos = 0;
	m_fifoin_wpos = 0;

	m_acc = 0;
	m_mat_stack_pos = 0;
	memset(m_cmat, 0, sizeof(m_cmat));
	m_cmat[0] = 1.0;
	m_cmat[4] = 1.0;
	m_cmat[8] = 1.0;

	m_dump = 0;
	m_swa = swa;
	next_fn();
}

/*********************************** Virtua Racing ***********************************/


void model1_state::vr_tgp_reset()
{
	m_ram_adr = 0;
	memset(m_ram_data, 0, 0x8000*4);

	m_copro_fifoout_rpos = 0;
	m_copro_fifoout_wpos = 0;
	m_copro_fifoout_num = 0;
	m_copro_fifoin_rpos = 0;
	m_copro_fifoin_wpos = 0;
	m_copro_fifoin_num = 0;
}

/* FIFO */
READ_LINE_MEMBER(model1_state::copro_fifoin_pop_ok)
{
	if (m_copro_fifoin_num == 0)
	{
		return CLEAR_LINE;
	}

	return ASSERT_LINE;
}

READ32_MEMBER(model1_state::copro_fifoin_pop)
{
	UINT32 r = m_copro_fifoin_data[m_copro_fifoin_rpos++];

	if (m_copro_fifoin_rpos == FIFO_SIZE)
	{
		m_copro_fifoin_rpos = 0;
	}

	m_copro_fifoin_num--;

	return r;
}


void model1_state::copro_fifoin_push(UINT32 data)
{
	if (m_copro_fifoin_num == FIFO_SIZE)
	{
		fatalerror("Copro FIFOIN overflow (at %08X)\n", safe_pc());
	}

	m_copro_fifoin_data[m_copro_fifoin_wpos++] = data;

	if (m_copro_fifoin_wpos == FIFO_SIZE)
	{
		m_copro_fifoin_wpos = 0;
	}

	m_copro_fifoin_num++;
}

UINT32 model1_state::copro_fifoout_pop()
{
	if (m_copro_fifoout_num == 0)
	{
		// Reading from empty FIFO causes the v60 to enter wait state
		m_maincpu->stall();

		machine().scheduler().synchronize();

		return 0;
	}

	UINT32 r = m_copro_fifoout_data[m_copro_fifoout_rpos++];

	if (m_copro_fifoout_rpos == FIFO_SIZE)
	{
		m_copro_fifoout_rpos = 0;
	}

	m_copro_fifoout_num--;

	return r;
}

WRITE32_MEMBER(model1_state::copro_fifoout_push)
{
	if (m_copro_fifoout_num == FIFO_SIZE)
	{
		fatalerror("Copro FIFOOUT overflow (at %08X)\n", m_tgp->pc());
	}

	m_copro_fifoout_data[m_copro_fifoout_wpos++] = data;

	if (m_copro_fifoout_wpos == FIFO_SIZE)
	{
		m_copro_fifoout_wpos = 0;
	}

	m_copro_fifoout_num++;
}

READ32_MEMBER(model1_state::copro_ram_r)
{
	return m_ram_data[offset & 0x7fff];
}

WRITE32_MEMBER(model1_state::copro_ram_w)
{
	m_ram_data[offset&0x7fff] = data;
}

READ16_MEMBER(model1_state::model1_tgp_vr_adr_r)
{
	if ( m_ram_adr == 0 && m_copro_fifoin_num != 0 )
	{
		/* spin the main cpu and let the TGP catch up */
		space.device().execute().spin_until_time(attotime::from_usec(100));
	}

	return m_ram_adr;
}

WRITE16_MEMBER(model1_state::model1_tgp_vr_adr_w)
{
	COMBINE_DATA(&m_ram_adr);
}

READ16_MEMBER(model1_state::model1_vr_tgp_ram_r)
{
	UINT16  r;

	if (!offset)
	{
		r = m_ram_data[m_ram_adr&0x7fff];
	}
	else
	{
		r = m_ram_data[m_ram_adr&0x7fff] >> 16;

		if ( m_ram_adr == 0 && r == 0xffff )
		{
			/* if the TGP is busy, spin some more */
			space.device().execute().spin_until_time(attotime::from_usec(100));
		}

		if ( m_ram_adr & 0x8000 )
			m_ram_adr++;
	}

	return r;
}

WRITE16_MEMBER(model1_state::model1_vr_tgp_ram_w)
{
	COMBINE_DATA(m_ram_latch+offset);

	if (offset)
	{
		UINT32 v = m_ram_latch[0]|(m_ram_latch[1]<<16);
		m_ram_data[m_ram_adr&0x7fff] = v;
		if ( m_ram_adr & 0x8000 )
			m_ram_adr++;
	}
}

READ16_MEMBER(model1_state::model1_vr_tgp_r)
{
	if (!offset)
	{
		m_vr_r = copro_fifoout_pop();
		return m_vr_r;
	}
	else
		return m_vr_r >> 16;
}

WRITE16_MEMBER(model1_state::model1_vr_tgp_w)
{
	if (offset)
	{
		m_vr_w = (m_vr_w & 0x0000ffff) | (data << 16);
		copro_fifoin_push(m_vr_w);
	}
	else
		m_vr_w = (m_vr_w & 0xffff0000) | data;
}

/* TGP memory map */
ADDRESS_MAP_START( model1_vr_tgp_map, AS_PROGRAM, 32, model1_state )
	AM_RANGE(0x00000000, 0x000007ff) AM_RAM AM_REGION("tgp", 0)
	AM_RANGE(0x00400000, 0x00407fff) AM_READWRITE(copro_ram_r, copro_ram_w)
	AM_RANGE(0xff800000, 0xff87ffff) AM_ROM AM_REGION("user2", 0)
ADDRESS_MAP_END
