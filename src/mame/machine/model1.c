/*
** Model 1 coprocessor TGP simulation
*/

#include "emu.h"
#include "debugger.h"
#include "cpu/mb86233/mb86233.h"
#include "cpu/v60/v60.h"
#include "includes/model1.h"

#define TGP_FUNCTION(name) void name(running_machine &machine)


static UINT32 fifoout_pop(address_space *space)
{
	model1_state *state = space->machine().driver_data<model1_state>();
	UINT32 v;
	if(state->m_fifoout_wpos == state->m_fifoout_rpos) {
		fatalerror("TGP FIFOOUT underflow (%x)", cpu_get_pc(&space->device()));
	}
	v = state->m_fifoout_data[state->m_fifoout_rpos++];
	if(state->m_fifoout_rpos == FIFO_SIZE)
		state->m_fifoout_rpos = 0;
	return v;
}


static void fifoout_push(model1_state *state, UINT32 data)
{
	if(!state->m_puuu)
		logerror("TGP: Push %d\n", data);
	else
		state->m_puuu = 0;
	state->m_fifoout_data[state->m_fifoout_wpos++] = data;
	if(state->m_fifoout_wpos == FIFO_SIZE)
		state->m_fifoout_wpos = 0;
	if(state->m_fifoout_wpos == state->m_fifoout_rpos)
		logerror("TGP FIFOOUT overflow\n");
}

static void fifoout_push_f(model1_state *state, float data)
{
	state->m_puuu = 1;

	logerror("TGP: Push %f\n", data);
	fifoout_push(state, f2u(data));
}

static UINT32 fifoin_pop(model1_state *state)
{
	UINT32 v;
	if(state->m_fifoin_wpos == state->m_fifoin_rpos)
		logerror("TGP FIFOIN underflow\n");
	v = state->m_fifoin_data[state->m_fifoin_rpos++];
	if(state->m_fifoin_rpos == FIFO_SIZE)
		state->m_fifoin_rpos = 0;
	return v;
}

static void fifoin_push(address_space *space, UINT32 data)
{
	model1_state *state = space->machine().driver_data<model1_state>();
	//  logerror("TGP FIFOIN write %08x (%x)\n", data, cpu_get_pc(&space->device()));
	state->m_fifoin_data[state->m_fifoin_wpos++] = data;
	if(state->m_fifoin_wpos == FIFO_SIZE)
		state->m_fifoin_wpos = 0;
	if(state->m_fifoin_wpos == state->m_fifoin_rpos)
		logerror("TGP FIFOIN overflow\n");
	state->m_fifoin_cbcount--;
	if(!state->m_fifoin_cbcount)
		state->m_fifoin_cb(space->machine());
}

static float fifoin_pop_f(model1_state *state)
{
	return u2f(fifoin_pop(state));
}

static TGP_FUNCTION( function_get_vf );
static TGP_FUNCTION( function_get_swa );

static void next_fn(model1_state *state)
{
	state->m_fifoin_cbcount = 1;
	state->m_fifoin_cb = state->m_swa ? function_get_swa : function_get_vf;
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

static UINT16 ram_get_i(model1_state *state)
{
	return state->m_ram_data[state->m_ram_scanadr++];
}

static float ram_get_f(model1_state *state)
{
	return u2f(state->m_ram_data[state->m_ram_scanadr++]);
}

static TGP_FUNCTION( fadd )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float r = a+b;
	logerror("TGP fadd %f+%f=%f (%x)\n", a, b, r, state->m_pushpc);
	fifoout_push_f(state, r);
	next_fn(state);
}

static TGP_FUNCTION( fsub )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float r = a-b;
	state->m_dump = 1;
	logerror("TGP fsub %f-%f=%f (%x)\n", a, b, r, state->m_pushpc);
	fifoout_push_f(state, r);
	next_fn(state);
}

static TGP_FUNCTION( fmul )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float r = a*b;
	logerror("TGP fmul %f*%f=%f (%x)\n", a, b, r, state->m_pushpc);
	fifoout_push_f(state, r);
	next_fn(state);
}

static TGP_FUNCTION( fdiv )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
//  float r = !b ? 1e39 : a/b;
	float r = !b ? 0 : a * (1/b);
	logerror("TGP fdiv %f/%f=%f (%x)\n", a, b, r, state->m_pushpc);
	fifoout_push_f(state, r);
	next_fn(state);
}

static TGP_FUNCTION( matrix_push )
{
	model1_state *state = machine.driver_data<model1_state>();
	if(state->m_mat_stack_pos != MAT_STACK_SIZE) {
		memcpy(state->m_mat_stack[state->m_mat_stack_pos], state->m_cmat, sizeof(state->m_cmat));
		state->m_mat_stack_pos++;
	}
	logerror("TGP matrix_push (depth=%d, pc=%x)\n", state->m_mat_stack_pos, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( matrix_pop )
{
	model1_state *state = machine.driver_data<model1_state>();
	if(state->m_mat_stack_pos) {
		state->m_mat_stack_pos--;
		memcpy(state->m_cmat, state->m_mat_stack[state->m_mat_stack_pos], sizeof(state->m_cmat));
	}
	logerror("TGP matrix_pop (depth=%d, pc=%x)\n", state->m_mat_stack_pos, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( matrix_write )
{
	model1_state *state = machine.driver_data<model1_state>();
	int i;
	for(i=0; i<12; i++)
		state->m_cmat[i] = fifoin_pop_f(state);
	logerror("TGP matrix_write %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f) (%x)\n",
			 state->m_cmat[0], state->m_cmat[1], state->m_cmat[2], state->m_cmat[3], state->m_cmat[4], state->m_cmat[5], state->m_cmat[6], state->m_cmat[7], state->m_cmat[8], state->m_cmat[9], state->m_cmat[10], state->m_cmat[11],
			 state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( clear_stack )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP clear_stack (%x)\n", state->m_pushpc);
	state->m_mat_stack_pos = 0;
	next_fn(state);
}

static TGP_FUNCTION( matrix_mul )
{
	model1_state *state = machine.driver_data<model1_state>();
	float m[12];
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	float g = fifoin_pop_f(state);
	float h = fifoin_pop_f(state);
	float i = fifoin_pop_f(state);
	float j = fifoin_pop_f(state);
	float k = fifoin_pop_f(state);
	float l = fifoin_pop_f(state);
	logerror("TGP matrix_mul %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, state->m_pushpc);
	m[0]  = a*state->m_cmat[0] + b*state->m_cmat[3] + c*state->m_cmat[6];
	m[1]  = a*state->m_cmat[1] + b*state->m_cmat[4] + c*state->m_cmat[7];
	m[2]  = a*state->m_cmat[2] + b*state->m_cmat[5] + c*state->m_cmat[8];
	m[3]  = d*state->m_cmat[0] + e*state->m_cmat[3] + f*state->m_cmat[6];
	m[4]  = d*state->m_cmat[1] + e*state->m_cmat[4] + f*state->m_cmat[7];
	m[5]  = d*state->m_cmat[2] + e*state->m_cmat[5] + f*state->m_cmat[8];
	m[6]  = g*state->m_cmat[0] + h*state->m_cmat[3] + i*state->m_cmat[6];
	m[7]  = g*state->m_cmat[1] + h*state->m_cmat[4] + i*state->m_cmat[7];
	m[8]  = g*state->m_cmat[2] + h*state->m_cmat[5] + i*state->m_cmat[8];
	m[9]  = j*state->m_cmat[0] + k*state->m_cmat[3] + l*state->m_cmat[6] + state->m_cmat[9];
	m[10] = j*state->m_cmat[1] + k*state->m_cmat[4] + l*state->m_cmat[7] + state->m_cmat[10];
	m[11] = j*state->m_cmat[2] + k*state->m_cmat[5] + l*state->m_cmat[8] + state->m_cmat[11];

	memcpy(state->m_cmat, m, sizeof(m));
	next_fn(state);
}

static TGP_FUNCTION( anglev )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	logerror("TGP anglev %f, %f (%x)\n", a, b, state->m_pushpc);
	if(!b) {
		if(a>=0)
			fifoout_push(state, 0);
		else
			fifoout_push(state, (UINT32)-32768);
	} else if(!a) {
		if(b>=0)
			fifoout_push(state, 16384);
		else
			fifoout_push(state, (UINT32)-16384);
	} else
		fifoout_push(state, (INT16)(atan2(b, a)*32768/M_PI));
	next_fn(state);
}

static TGP_FUNCTION( f11 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	float g = fifoin_pop_f(state);
	float h = fifoin_pop_f(state);
	float i = fifoin_pop_f(state);
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	(void)g;
	(void)h;
	(void)i;
	logerror("TGP f11 %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, state->m_pushpc);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( normalize )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float n = (a*a+b*b+c*c) / sqrt(a*a+b*b+c*c);
	logerror("TGP normalize %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);
	fifoout_push_f(state, a/n);
	fifoout_push_f(state, b/n);
	fifoout_push_f(state, c/n);
	next_fn(state);
}

static TGP_FUNCTION( acc_seti )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT32 a = fifoin_pop(state);
	state->m_dump = 1;
	logerror("TGP acc_seti %d (%x)\n", a, state->m_pushpc);
	state->m_acc = a;
	next_fn(state);
}

static TGP_FUNCTION( track_select )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT32 a = fifoin_pop(state);
	logerror("TGP track_select %d (%x)\n", a, state->m_pushpc);
	state->m_tgp_vr_select = a;
	next_fn(state);
}

static TGP_FUNCTION( f14 )
{
	model1_state *state = machine.driver_data<model1_state>();
	state->m_tgp_vr_base[0] = fifoin_pop_f(state);
	state->m_tgp_vr_base[1] = fifoin_pop_f(state);
	state->m_tgp_vr_base[2] = fifoin_pop_f(state);
	state->m_tgp_vr_base[3] = fifoin_pop_f(state);

	next_fn(state);
}

static TGP_FUNCTION( f15_swa )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP f15_swa (%x)\n", state->m_pushpc);

	next_fn(state);
}

static TGP_FUNCTION( anglep )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	logerror("TGP anglep %f, %f, %f, %f (%x)\n", a, b, c, d, state->m_pushpc);
	c = a - c;
	d = b - d;
	if(!d) {
		if(c>=0)
			fifoout_push(state, 0);
		else
			fifoout_push(state, (UINT32)-32768);
	} else if(!c) {
		if(d>=0)
			fifoout_push(state, 16384);
		else
			fifoout_push(state, (UINT32)-16384);
	} else
		fifoout_push(state, (INT16)(atan2(d, c)*32768/M_PI));
	next_fn(state);
}

static TGP_FUNCTION( matrix_ident )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP matrix_ident (%x)\n", state->m_pushpc);
	memset(state->m_cmat, 0, sizeof(state->m_cmat));
	state->m_cmat[0] = 1.0;
	state->m_cmat[4] = 1.0;
	state->m_cmat[8] = 1.0;
	next_fn(state);
}

static TGP_FUNCTION( matrix_read )
{
	model1_state *state = machine.driver_data<model1_state>();
	int i;
	logerror("TGP matrix_read (%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f) (%x)\n",
			 state->m_cmat[0], state->m_cmat[1], state->m_cmat[2], state->m_cmat[3], state->m_cmat[4], state->m_cmat[5], state->m_cmat[6], state->m_cmat[7], state->m_cmat[8], state->m_cmat[9], state->m_cmat[10], state->m_cmat[11], state->m_pushpc);
	for(i=0; i<12; i++)
		fifoout_push_f(state, state->m_cmat[i]);
	next_fn(state);
}

static TGP_FUNCTION( matrix_trans )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);

	state->m_cmat[ 9] += state->m_cmat[0]*a+state->m_cmat[3]*b+state->m_cmat[6]*c;
	state->m_cmat[10] += state->m_cmat[1]*a+state->m_cmat[4]*b+state->m_cmat[7]*c;
	state->m_cmat[11] += state->m_cmat[2]*a+state->m_cmat[5]*b+state->m_cmat[8]*c;
	next_fn(state);
}

static TGP_FUNCTION( matrix_scale )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	logerror("TGP matrix_scale %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);
	state->m_cmat[0] *= a;
	state->m_cmat[1] *= a;
	state->m_cmat[2] *= a;
	state->m_cmat[3] *= b;
	state->m_cmat[4] *= b;
	state->m_cmat[5] *= b;
	state->m_cmat[6] *= c;
	state->m_cmat[7] *= c;
	state->m_cmat[8] *= c;
	next_fn(state);
}

static TGP_FUNCTION( matrix_rotx )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT16 a = fifoin_pop(state);
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;
	logerror("TGP matrix_rotx %d (%x)\n", a, state->m_pushpc);
	t1 = state->m_cmat[3];
	t2 = state->m_cmat[6];
	state->m_cmat[3] = c*t1-s*t2;
	state->m_cmat[6] = s*t1+c*t2;
	t1 = state->m_cmat[4];
	t2 = state->m_cmat[7];
	state->m_cmat[4] = c*t1-s*t2;
	state->m_cmat[7] = s*t1+c*t2;
	t1 = state->m_cmat[5];
	t2 = state->m_cmat[8];
	state->m_cmat[5] = c*t1-s*t2;
	state->m_cmat[8] = s*t1+c*t2;
	next_fn(state);
}

static TGP_FUNCTION( matrix_roty )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT16 a = fifoin_pop(state);
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;

	logerror("TGP matrix_roty %d (%x)\n", a, state->m_pushpc);
	t1 = state->m_cmat[6];
	t2 = state->m_cmat[0];
	state->m_cmat[6] = c*t1-s*t2;
	state->m_cmat[0] = s*t1+c*t2;
	t1 = state->m_cmat[7];
	t2 = state->m_cmat[1];
	state->m_cmat[7] = c*t1-s*t2;
	state->m_cmat[1] = s*t1+c*t2;
	t1 = state->m_cmat[8];
	t2 = state->m_cmat[2];
	state->m_cmat[8] = c*t1-s*t2;
	state->m_cmat[2] = s*t1+c*t2;
	next_fn(state);
}

static TGP_FUNCTION( matrix_rotz )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT16 a = fifoin_pop(state);
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;

	logerror("TGP matrix_rotz %d (%x)\n", a, state->m_pushpc);
	t1 = state->m_cmat[0];
	t2 = state->m_cmat[3];
	state->m_cmat[0] = c*t1-s*t2;
	state->m_cmat[3] = s*t1+c*t2;
	t1 = state->m_cmat[1];
	t2 = state->m_cmat[4];
	state->m_cmat[1] = c*t1-s*t2;
	state->m_cmat[4] = s*t1+c*t2;
	t1 = state->m_cmat[2];
	t2 = state->m_cmat[5];
	state->m_cmat[2] = c*t1-s*t2;
	state->m_cmat[5] = s*t1+c*t2;
	next_fn(state);
}

static TGP_FUNCTION( track_read_quad )
{
	model1_state *state = machine.driver_data<model1_state>();
	const UINT32 *tgp_data = (const UINT32 *)machine.region("user2")->base();
	UINT32 a = fifoin_pop(state);
	int offd;

	logerror("TGP track_read_quad %d (%x)\n", a, state->m_pushpc);

	offd = tgp_data[0x20+state->m_tgp_vr_select] + 16*a;
	fifoout_push(state, tgp_data[offd]);
	fifoout_push(state, tgp_data[offd+1]);
	fifoout_push(state, tgp_data[offd+2]);
	fifoout_push(state, tgp_data[offd+3]);
	fifoout_push(state, tgp_data[offd+4]);
	fifoout_push(state, tgp_data[offd+5]);
	fifoout_push(state, tgp_data[offd+6]);
	fifoout_push(state, tgp_data[offd+7]);
	fifoout_push(state, tgp_data[offd+8]);
	fifoout_push(state, tgp_data[offd+9]);
	fifoout_push(state, tgp_data[offd+10]);
	fifoout_push(state, tgp_data[offd+11]);
	next_fn(state);
}

static TGP_FUNCTION( f24_swa )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	UINT32 g = fifoin_pop(state);
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	(void)g;
	logerror("TGP f24_swa %f, %f, %f, %f, %f, %f, %x (%x)\n", a, b, c, d, e, f, g, state->m_pushpc);
	fifoout_push_f(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( transform_point )
{
	model1_state *state = machine.driver_data<model1_state>();
	float x = fifoin_pop_f(state);
	float y = fifoin_pop_f(state);
	float z = fifoin_pop_f(state);
	logerror("TGP transform_point %f, %f, %f (%x)\n", x, y, z, state->m_pushpc);

	fifoout_push_f(state, state->m_cmat[0]*x+state->m_cmat[3]*y+state->m_cmat[6]*z+state->m_cmat[9]);
	fifoout_push_f(state, state->m_cmat[1]*x+state->m_cmat[4]*y+state->m_cmat[7]*z+state->m_cmat[10]);
	fifoout_push_f(state, state->m_cmat[2]*x+state->m_cmat[5]*y+state->m_cmat[8]*z+state->m_cmat[11]);
	next_fn(state);
}

static TGP_FUNCTION( fcos_m1 )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT16 a = fifoin_pop(state);
	logerror("TGP fcos %d (%x)\n", a, state->m_pushpc);
	fifoout_push_f(state, tcos(a));
	next_fn(state);
}

static TGP_FUNCTION( fsin_m1 )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT16 a = fifoin_pop(state);
	logerror("TGP fsin %d (%x)\n", a, state->m_pushpc);
	fifoout_push_f(state, tsin(a));
	next_fn(state);
}

static TGP_FUNCTION( fcosm_m1 )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT16 a = fifoin_pop(state);
	float b = fifoin_pop_f(state);
	logerror("TGP fcosm %d, %f (%x)\n", a, b, state->m_pushpc);
	fifoout_push_f(state, b*tcos(a));
	next_fn(state);
}

static TGP_FUNCTION( fsinm_m1 )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT16 a = fifoin_pop(state);
	float b = fifoin_pop_f(state);
	state->m_dump = 1;
	logerror("TGP fsinm %d, %f (%x)\n", a, b, state->m_pushpc);
	fifoout_push_f(state, b*tsin(a));
	next_fn(state);
}

static TGP_FUNCTION( distance3 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	logerror("TGP distance3 (%f, %f, %f), (%f, %f, %f) (%x)\n", a, b, c, d, e, f, state->m_pushpc);
	a -= d;
	b -= e;
	c -= f;
	fifoout_push_f(state, (a*a+b*b+c*c)/sqrt(a*a+b*b+c*c));
	next_fn(state);
}

static TGP_FUNCTION( ftoi )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	logerror("TGP ftoi %f (%x)\n", a, state->m_pushpc);
	fifoout_push(state, (int)a);
	next_fn(state);
}

static TGP_FUNCTION( itof )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT32 a = fifoin_pop(state);
	logerror("TGP itof %d (%x)\n", a, state->m_pushpc);
	fifoout_push_f(state, a);
	next_fn(state);
}

static TGP_FUNCTION( acc_set )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	logerror("TGP acc_set %f (%x)\n", a, state->m_pushpc);
	state->m_acc = a;
	next_fn(state);
}

static TGP_FUNCTION( acc_get )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP acc_get (%x)\n", state->m_pushpc);
	fifoout_push_f(state, state->m_acc);
	next_fn(state);
}

static TGP_FUNCTION( acc_add )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	logerror("TGP acc_add %f (%x)\n", a, state->m_pushpc);
	state->m_acc += a;
	next_fn(state);
}

static TGP_FUNCTION( acc_sub )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	logerror("TGP acc_sub %f (%x)\n", a, state->m_pushpc);
	state->m_acc -= a;
	next_fn(state);
}

static TGP_FUNCTION( acc_mul )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	logerror("TGP acc_mul %f (%x)\n", a, state->m_pushpc);
	state->m_acc *= a;
	next_fn(state);
}

static TGP_FUNCTION( acc_div )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	logerror("TGP acc_div %f (%x)\n", a, state->m_pushpc);
	state->m_acc /= a;
	next_fn(state);
}

static TGP_FUNCTION( f42 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP f42 %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);
	//  fifoout_push_f((machine.rand() % 1000) - 500);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	next_fn(state);
}


// r = (x2 + y2 + z2)1/2,     f = tan-1(y/(x2+z2)1/2),     q = tan-1(z/x)

static TGP_FUNCTION( xyz2rqf )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float norm;
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP xyz2rqf %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);
	fifoout_push_f(state, (a*a+b*b+c*c)/sqrt(a*a+b*b+c*c));
	norm = sqrt(a*a+c*c);
	if(!c) {
		if(a>=0)
			fifoout_push(state, 0);
		else
			fifoout_push(state, (UINT32)-32768);
	} else if(!a) {
		if(c>=0)
			fifoout_push(state, 16384);
		else
			fifoout_push(state, (UINT32)-16384);
	} else
		fifoout_push(state, (INT16)(atan2(c, a)*32768/M_PI));

	if(!b)
		fifoout_push(state, 0);
	else if(!norm) {
		if(b>=0)
			fifoout_push(state, 16384);
		else
			fifoout_push(state, (UINT32)-16384);
	} else
		fifoout_push(state, (INT16)(atan2(b, norm)*32768/M_PI));

	next_fn(state);
}

static TGP_FUNCTION( f43 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	logerror("TGP f43 %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, state->m_pushpc);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( f43_swa )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	int b = fifoin_pop(state);
	int c = fifoin_pop(state);
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP f43_swa %f, %d, %d (%x)\n", a, b, c, state->m_pushpc);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( f44 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	(void)a;
	logerror("TGP f44 %f (%x)\n", a, state->m_pushpc);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( matrix_sdir )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float norm = sqrt(a*a+b*b+c*c);
	float t[9], m[9];
	logerror("TGP matrix_sdir %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);

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

	m[0]  = t[0]*state->m_cmat[0] + t[1]*state->m_cmat[3] + t[2]*state->m_cmat[6];
	m[1]  = t[0]*state->m_cmat[1] + t[1]*state->m_cmat[4] + t[2]*state->m_cmat[7];
	m[2]  = t[0]*state->m_cmat[2] + t[1]*state->m_cmat[5] + t[2]*state->m_cmat[8];
	m[3]  = t[3]*state->m_cmat[0] + t[4]*state->m_cmat[3] + t[5]*state->m_cmat[6];
	m[4]  = t[3]*state->m_cmat[1] + t[4]*state->m_cmat[4] + t[5]*state->m_cmat[7];
	m[5]  = t[3]*state->m_cmat[2] + t[4]*state->m_cmat[5] + t[5]*state->m_cmat[8];
	m[6]  = t[6]*state->m_cmat[0] + t[7]*state->m_cmat[3] + t[8]*state->m_cmat[6];
	m[7]  = t[6]*state->m_cmat[1] + t[7]*state->m_cmat[4] + t[8]*state->m_cmat[7];
	m[8]  = t[6]*state->m_cmat[2] + t[7]*state->m_cmat[5] + t[8]*state->m_cmat[8];

	memcpy(state->m_cmat, m, sizeof(m));

	next_fn(state);
}

static TGP_FUNCTION( f45 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	(void)a;
	logerror("TGP f45 %f (%x)\n", a, state->m_pushpc);
	fifoout_push_f(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( vlength )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state) - state->m_tgp_vr_base[0];
	float b = fifoin_pop_f(state) - state->m_tgp_vr_base[1];
	float c = fifoin_pop_f(state) - state->m_tgp_vr_base[2];
	logerror("TGP vlength %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);

	a = (a*a+b*b+c*c);
	b = 1/sqrt(a);
	c = a * b;
	c -= state->m_tgp_vr_base[3];
	fifoout_push_f(state, c);
	next_fn(state);
}

static TGP_FUNCTION( f47 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	logerror("TGP f47 %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);
	fifoout_push_f(state, a+c);
	fifoout_push_f(state, b+c);
	next_fn(state);
}

static TGP_FUNCTION( track_read_info )
{
	model1_state *state = machine.driver_data<model1_state>();
	const UINT32 *tgp_data = (const UINT32 *)machine.region("user2")->base();
	UINT16 a = fifoin_pop(state);
	int offd;

	logerror("TGP track_read_info %d (%x)\n", a, state->m_pushpc);

	offd = tgp_data[0x20+state->m_tgp_vr_select] + 16*a;
	fifoout_push(state, tgp_data[offd+15]);
	next_fn(state);
}

static TGP_FUNCTION( colbox_set )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	float g = fifoin_pop_f(state);
	float h = fifoin_pop_f(state);
	float i = fifoin_pop_f(state);
	float j = fifoin_pop_f(state);
	float k = fifoin_pop_f(state);
	float l = fifoin_pop_f(state);
	logerror("TGP colbox_set %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, state->m_pushpc);
	state->m_tgp_vr_cbox[ 0] = a;
	state->m_tgp_vr_cbox[ 1] = b;
	state->m_tgp_vr_cbox[ 2] = c;
	state->m_tgp_vr_cbox[ 3] = d;
	state->m_tgp_vr_cbox[ 4] = e;
	state->m_tgp_vr_cbox[ 5] = f;
	state->m_tgp_vr_cbox[ 6] = g;
	state->m_tgp_vr_cbox[ 7] = h;
	state->m_tgp_vr_cbox[ 8] = i;
	state->m_tgp_vr_cbox[ 9] = j;
	state->m_tgp_vr_cbox[10] = k;
	state->m_tgp_vr_cbox[11] = l;
	next_fn(state);
}

static TGP_FUNCTION( colbox_test )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP colbox_test %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);

	// #### Wrong, need to check with the tgp_vr_cbox coordinates
	// Game only test sign, negative = collision
	fifoout_push_f(state, -1);
	next_fn(state);
}

static TGP_FUNCTION( f49_swa )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	logerror("TGP f49_swa %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( f50_swa )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	logerror("TGP f50_swa %f, %f, %f, %f (%x)\n", a, b, c, d, state->m_pushpc);
	fifoout_push_f(state, d);
	next_fn(state);
}

static TGP_FUNCTION( f52 )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP f52 (%x)\n", state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( matrix_rdir )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float norm = sqrt(a*a+c*c);
	float t1, t2;
	(void)b;

	logerror("TGP matrix_rdir %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);

	if(!norm) {
		c = 1;
		a = 0;
	} else {
		c /= norm;
		a /= norm;
	}

	t1 = state->m_cmat[6];
	t2 = state->m_cmat[0];
	state->m_cmat[6] = c*t1-a*t2;
	state->m_cmat[0] = a*t1+c*t2;
	t1 = state->m_cmat[7];
	t2 = state->m_cmat[1];
	state->m_cmat[7] = c*t1-a*t2;
	state->m_cmat[1] = a*t1+c*t2;
	t1 = state->m_cmat[8];
	t2 = state->m_cmat[2];
	state->m_cmat[8] = c*t1-a*t2;
	state->m_cmat[2] = a*t1+c*t2;
	next_fn(state);
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

static TGP_FUNCTION( track_lookup )
{
	model1_state *state = machine.driver_data<model1_state>();
	const UINT32 *tgp_data = (const UINT32 *)machine.region("user2")->base();
	float a = fifoin_pop_f(state);
	UINT32 b = fifoin_pop(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	int offi, offd, len;
	float dist;
	int i;
	UINT32 entry;
	float height;

	logerror("TGP track_lookup %f, 0x%x, %f, %f (%x)\n", a, b, c, d, state->m_pushpc);

	offi = tgp_data[0x10+state->m_tgp_vr_select] + b;
	offd = tgp_data[0x20+state->m_tgp_vr_select];

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

	state->m_ram_data[0x0000] = 0; // non zero = still computing
	state->m_ram_data[0x8001] = f2u(height);
	state->m_ram_data[0x8002] = entry;

	next_fn(state);
}

static TGP_FUNCTION( f56 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	UINT32 g = fifoin_pop(state);
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)f;
	(void)g;

	logerror("TGP f56 %f, %f, %f, %f, %f, %f, %d (%x)\n", a, b, c, d, e, f, g, state->m_pushpc);
	fifoout_push(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( f57 )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP f57 (%x)\n", state->m_pushpc);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( matrix_readt )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP matrix_readt (%x)\n", state->m_pushpc);
	fifoout_push_f(state, state->m_cmat[9]);
	fifoout_push_f(state, state->m_cmat[10]);
	fifoout_push_f(state, state->m_cmat[11]);
	next_fn(state);
}

static TGP_FUNCTION( acc_geti )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP acc_geti (%x)\n", state->m_pushpc);
	fifoout_push(state, (int)state->m_acc);
	next_fn(state);
}

static TGP_FUNCTION( f60 )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP f60 (%x)\n", state->m_pushpc);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	fifoout_push_f(state, 0);
	next_fn(state);
}

static TGP_FUNCTION( col_setcirc )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	logerror("TGP col_setcirc %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);
	state->m_tgp_vr_circx = a;
	state->m_tgp_vr_circy = b;
	state->m_tgp_vr_circrad = c;
	next_fn(state);
}

static TGP_FUNCTION( col_testpt )
{
	model1_state *state = machine.driver_data<model1_state>();
	float x, y;
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	logerror("TGP col_testpt %f, %f (%x)\n", a, b, state->m_pushpc);
	x = a - state->m_tgp_vr_circx;
	y = b - state->m_tgp_vr_circy;
	fifoout_push_f(state, ((x*x+y*y)/sqrt(x*x+y*y)) - state->m_tgp_vr_circrad);
	next_fn(state);
}

static TGP_FUNCTION( push_and_ident )
{
	model1_state *state = machine.driver_data<model1_state>();
	if(state->m_mat_stack_pos != MAT_STACK_SIZE) {
		memcpy(state->m_mat_stack[state->m_mat_stack_pos], state->m_cmat, sizeof(state->m_cmat));
		state->m_mat_stack_pos++;
	}
	logerror("TGP push_and_ident (depth=%d, pc=%x)\n", state->m_mat_stack_pos, state->m_pushpc);
	memset(state->m_cmat, 0, sizeof(state->m_cmat));
	state->m_cmat[0] = 1.0;
	state->m_cmat[4] = 1.0;
	state->m_cmat[8] = 1.0;
	next_fn(state);
}

static TGP_FUNCTION( catmull_rom )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	float g = fifoin_pop_f(state);
	float h = fifoin_pop_f(state);
	float i = fifoin_pop_f(state);
	float j = fifoin_pop_f(state);
	float k = fifoin_pop_f(state);
	float l = fifoin_pop_f(state);
	float m = fifoin_pop_f(state);
	float m2, m3;
	float w1, w2, w3, w4;

	logerror("TGP catmull_rom %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, m, state->m_pushpc);

	m2 = m*m;
	m3 = m*m*m;

	w1 = 0.5*(-m3+2*m2-m);
	w2 = 0.5*(3*m3-5*m2+2);
	w3 = 0.5*(-3*m3+4*m2+m);
	w4 = 0.5*(m3-m2);

	fifoout_push_f(state, a*w1+d*w2+g*w3+j*w4);
	fifoout_push_f(state, b*w1+e*w2+h*w3+k*w4);
	fifoout_push_f(state, c*w1+f*w2+i*w3+l*w4);
	next_fn(state);
}

static TGP_FUNCTION( distance )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	logerror("TGP distance (%f, %f), (%f, %f) (%x)\n", a, b, c, d, state->m_pushpc);
	c -= a;
	d -= b;
	fifoout_push_f(state, (c*c+d*d)/sqrt(c*c+d*d));
	next_fn(state);
}

static TGP_FUNCTION( car_move )
{
	model1_state *state = machine.driver_data<model1_state>();
	INT16 a = fifoin_pop(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float dx, dy;
	logerror("TGP car_move (%d, %f), (%f, %f) (%x)\n", a, b, c, d, state->m_pushpc);

	dx = b*tsin(a);
	dy = b*tcos(a);

	fifoout_push_f(state, dx);
	fifoout_push_f(state, dy);
	fifoout_push_f(state, c+dx);
	fifoout_push_f(state, d+dy);
	next_fn(state);
}

static TGP_FUNCTION( cpa )
{
	model1_state *state = machine.driver_data<model1_state>();
	float dv_x, dv_y, dv_z, dv2, dw_x, dw_y, dw_z, dt;

	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	float g = fifoin_pop_f(state);
	float h = fifoin_pop_f(state);
	float i = fifoin_pop_f(state);
	float j = fifoin_pop_f(state);
	float k = fifoin_pop_f(state);
	float l = fifoin_pop_f(state);
	logerror("TGP cpa %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, state->m_pushpc);

	dv_x = (b-a) - (d-c);
	dv_y = (f-e) - (h-g);
	dv_z = (j-i) - (l-k);
	dv2 = dv_x*dv_x + dv_y*dv_y + dv_z*dv_z;
	if(dv2 < 0.001)
		dt = 0;
	else {
		dw_x = a-c;
		dw_y = e-g;
		dw_z = i-k;
		dt = -(dw_x*dv_x + dw_y*dv_y + dw_z*dv_z)/dv2;
	}
	if(dt < 0)
		dt = 0;
	else if(dt > 1.0)
		dt = 1.0;

	dv_x = (a-c)*(1-dt) + (b-d)*dt;
	dv_y = (e-g)*(1-dt) + (f-h)*dt;
	dv_z = (i-k)*(1-dt) + (j-l)*dt;
	dv2 = dv_x*dv_x + dv_y*dv_y + dv_z*dv_z;

	fifoout_push_f(state, sqrt(dv2));
	next_fn(state);
}

static TGP_FUNCTION( vmat_store )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	if(a<21)
		memcpy(state->m_mat_vector[a], state->m_cmat, sizeof(state->m_cmat));
	else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_store %d (%x)\n", a, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( vmat_restore )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	if(a<21)
		memcpy(state->m_cmat, state->m_mat_vector[a], sizeof(state->m_cmat));
	else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_restore %d (%x)\n", a, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( vmat_mul )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	UINT32 b = fifoin_pop(state);
	if(a<21 && b<21) {
		state->m_mat_vector[b][0]  = state->m_mat_vector[a][ 0]*state->m_cmat[0] + state->m_mat_vector[a][ 1]*state->m_cmat[3] + state->m_mat_vector[a][ 2]*state->m_cmat[6];
		state->m_mat_vector[b][1]  = state->m_mat_vector[a][ 0]*state->m_cmat[1] + state->m_mat_vector[a][ 1]*state->m_cmat[4] + state->m_mat_vector[a][ 2]*state->m_cmat[7];
		state->m_mat_vector[b][2]  = state->m_mat_vector[a][ 0]*state->m_cmat[2] + state->m_mat_vector[a][ 1]*state->m_cmat[5] + state->m_mat_vector[a][ 2]*state->m_cmat[8];
		state->m_mat_vector[b][3]  = state->m_mat_vector[a][ 3]*state->m_cmat[0] + state->m_mat_vector[a][ 4]*state->m_cmat[3] + state->m_mat_vector[a][ 5]*state->m_cmat[6];
		state->m_mat_vector[b][4]  = state->m_mat_vector[a][ 3]*state->m_cmat[1] + state->m_mat_vector[a][ 4]*state->m_cmat[4] + state->m_mat_vector[a][ 5]*state->m_cmat[7];
		state->m_mat_vector[b][5]  = state->m_mat_vector[a][ 3]*state->m_cmat[2] + state->m_mat_vector[a][ 4]*state->m_cmat[5] + state->m_mat_vector[a][ 5]*state->m_cmat[8];
		state->m_mat_vector[b][6]  = state->m_mat_vector[a][ 6]*state->m_cmat[0] + state->m_mat_vector[a][ 7]*state->m_cmat[3] + state->m_mat_vector[a][ 8]*state->m_cmat[6];
		state->m_mat_vector[b][7]  = state->m_mat_vector[a][ 6]*state->m_cmat[1] + state->m_mat_vector[a][ 7]*state->m_cmat[4] + state->m_mat_vector[a][ 8]*state->m_cmat[7];
		state->m_mat_vector[b][8]  = state->m_mat_vector[a][ 6]*state->m_cmat[2] + state->m_mat_vector[a][ 7]*state->m_cmat[5] + state->m_mat_vector[a][ 8]*state->m_cmat[8];
		state->m_mat_vector[b][9]  = state->m_mat_vector[a][ 9]*state->m_cmat[0] + state->m_mat_vector[a][10]*state->m_cmat[3] + state->m_mat_vector[a][11]*state->m_cmat[6] + state->m_cmat[9];
		state->m_mat_vector[b][10] = state->m_mat_vector[a][ 9]*state->m_cmat[1] + state->m_mat_vector[a][10]*state->m_cmat[4] + state->m_mat_vector[a][11]*state->m_cmat[7] + state->m_cmat[10];
		state->m_mat_vector[b][11] = state->m_mat_vector[a][ 9]*state->m_cmat[2] + state->m_mat_vector[a][10]*state->m_cmat[5] + state->m_mat_vector[a][11]*state->m_cmat[8] + state->m_cmat[11];
	} else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_mul %d, %d (%x)\n", a, b, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( vmat_read )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	logerror("TGP vmat_read %d (%x)\n", a, state->m_pushpc);
	if(a<21) {
		int i;
		for(i=0; i<12; i++)
			fifoout_push_f(state, state->m_mat_vector[a][i]);
	} else {
		int i;
		logerror("TGP ERROR bad vector index\n");
		for(i=0; i<12; i++)
			fifoout_push_f(state, 0);
	}
	next_fn(state);
}

static TGP_FUNCTION( matrix_rtrans )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP matrix_rtrans (%x)\n", state->m_pushpc);
	fifoout_push_f(state, state->m_cmat[ 9]);
	fifoout_push_f(state, state->m_cmat[10]);
	fifoout_push_f(state, state->m_cmat[11]);
	next_fn(state);
}

static TGP_FUNCTION( matrix_unrot )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP matrix_unrot (%x)\n", state->m_pushpc);
	memset(state->m_cmat, 0, 9*sizeof(state->m_cmat[0]));
	state->m_cmat[0] = 1.0;
	state->m_cmat[4] = 1.0;
	state->m_cmat[8] = 1.0;
	next_fn(state);
}

static TGP_FUNCTION( f80 )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP f80 (%x)\n", state->m_pushpc);
	//  state->m_cmat[9] = state->m_cmat[10] = state->m_cmat[11] = 0;
	next_fn(state);
}

static TGP_FUNCTION( vmat_save )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	int i;
	logerror("TGP vmat_save 0x%x (%x)\n", a, state->m_pushpc);
	for(i=0; i<16; i++)
		memcpy(state->m_ram_data+a+0x10*i, state->m_mat_vector[i], sizeof(state->m_cmat));
	next_fn(state);
}

static TGP_FUNCTION( vmat_load )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	int i;
	logerror("TGP vmat_load 0x%x (%x)\n", a, state->m_pushpc);
	for(i=0; i<16; i++)
		memcpy(state->m_mat_vector[i], state->m_ram_data+a+0x10*i, sizeof(state->m_cmat));
	next_fn(state);
}

static TGP_FUNCTION( ram_setadr )
{
	model1_state *state = machine.driver_data<model1_state>();
	state->m_ram_scanadr = fifoin_pop(state) - 0x8000;
	logerror("TGP f0 ram_setadr 0x%x (%x)\n", state->m_ram_scanadr+0x8000, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( groundbox_test )
{
	model1_state *state = machine.driver_data<model1_state>();
	int out_x, out_y, out_z;
	float x, /*y,*/ z;
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);

	logerror("TGP groundbox_test %f, %f, %f (%x)\n", a, b, c, state->m_pushpc);
	x = state->m_cmat[0]*a+state->m_cmat[3]*b+state->m_cmat[6]*c+state->m_cmat[9];
	//y = state->m_cmat[1]*a+state->m_cmat[4]*b+state->m_cmat[7]*c+state->m_cmat[10];
	z = state->m_cmat[2]*a+state->m_cmat[5]*b+state->m_cmat[8]*c+state->m_cmat[11];

	out_x = x < state->m_tgp_vf_xmin || x > state->m_tgp_vf_xmax;
	out_z = z < state->m_tgp_vf_zmin || z > state->m_tgp_vf_zmax;
	out_y = 1; // Wrong, but untestable it seems.

	fifoout_push(state, out_x);
	fifoout_push(state, out_y);
	fifoout_push(state, out_z);
	next_fn(state);
}

static TGP_FUNCTION( f89 )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	UINT32 b = fifoin_pop(state);
	UINT32 c = fifoin_pop(state);
	UINT32 d = fifoin_pop(state);
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP list set base 0x%x, 0x%x, %d, length=%d (%x)\n", a, b, c, d, state->m_pushpc);
	state->m_list_length = d;
	next_fn(state);
}

static TGP_FUNCTION( f92 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	logerror("TGP f92 %f, %f, %f, %f (%x)\n", a, b, c, d, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( f93 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	(void)a;
	logerror("TGP f93 %f (%x)\n", a, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( f94 )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	(void)a;
	logerror("TGP f94 %d (%x)\n", a, state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( vmat_flatten )
{
	model1_state *state = machine.driver_data<model1_state>();
	int i;
	float m[12];
	logerror("TGP vmat_flatten (%x)\n", state->m_pushpc);

	for(i=0; i<16; i++) {
		memcpy(m, state->m_mat_vector[i], sizeof(state->m_cmat));
		m[1] = m[4] = m[7] = m[10] = 0;

		state->m_mat_vector[i][0]  = m[ 0]*state->m_cmat[0] + m[ 1]*state->m_cmat[3] + m[ 2]*state->m_cmat[6];
		state->m_mat_vector[i][1]  = m[ 0]*state->m_cmat[1] + m[ 1]*state->m_cmat[4] + m[ 2]*state->m_cmat[7];
		state->m_mat_vector[i][2]  = m[ 0]*state->m_cmat[2] + m[ 1]*state->m_cmat[5] + m[ 2]*state->m_cmat[8];
		state->m_mat_vector[i][3]  = m[ 3]*state->m_cmat[0] + m[ 4]*state->m_cmat[3] + m[ 5]*state->m_cmat[6];
		state->m_mat_vector[i][4]  = m[ 3]*state->m_cmat[1] + m[ 4]*state->m_cmat[4] + m[ 5]*state->m_cmat[7];
		state->m_mat_vector[i][5]  = m[ 3]*state->m_cmat[2] + m[ 4]*state->m_cmat[5] + m[ 5]*state->m_cmat[8];
		state->m_mat_vector[i][6]  = m[ 6]*state->m_cmat[0] + m[ 7]*state->m_cmat[3] + m[ 8]*state->m_cmat[6];
		state->m_mat_vector[i][7]  = m[ 6]*state->m_cmat[1] + m[ 7]*state->m_cmat[4] + m[ 8]*state->m_cmat[7];
		state->m_mat_vector[i][8]  = m[ 6]*state->m_cmat[2] + m[ 7]*state->m_cmat[5] + m[ 8]*state->m_cmat[8];
		state->m_mat_vector[i][9]  = m[ 9]*state->m_cmat[0] + m[10]*state->m_cmat[3] + m[11]*state->m_cmat[6] + state->m_cmat[9];
		state->m_mat_vector[i][10] = m[ 9]*state->m_cmat[1] + m[10]*state->m_cmat[4] + m[11]*state->m_cmat[7] + state->m_cmat[10];
		state->m_mat_vector[i][11] = m[ 9]*state->m_cmat[2] + m[10]*state->m_cmat[5] + m[11]*state->m_cmat[8] + state->m_cmat[11];
	}
	next_fn(state);
}

static TGP_FUNCTION( vmat_load1 )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	logerror("TGP vmat_load1 0x%x (%x)\n", a, state->m_pushpc);
	memcpy(state->m_cmat, state->m_ram_data+a, sizeof(state->m_cmat));
	next_fn(state);
}

static TGP_FUNCTION( ram_trans )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = ram_get_f(state);
	float b = ram_get_f(state);
	float c = ram_get_f(state);
	logerror("TGP ram_trans (%x)\n", state->m_pushpc);
	state->m_cmat[ 9] += state->m_cmat[0]*a+state->m_cmat[3]*b+state->m_cmat[6]*c;
	state->m_cmat[10] += state->m_cmat[1]*a+state->m_cmat[4]*b+state->m_cmat[7]*c;
	state->m_cmat[11] += state->m_cmat[2]*a+state->m_cmat[5]*b+state->m_cmat[8]*c;
	next_fn(state);
}

static TGP_FUNCTION( f98_load )
{
	model1_state *state = machine.driver_data<model1_state>();
	int i;
	for(i=0; i<state->m_list_length; i++) {
		float f = fifoin_pop_f(state);
		(void)f;
		logerror("TGP load list (%2d/%2d) %f (%x)\n", i, state->m_list_length, f, state->m_pushpc);
	}
	next_fn(state);
}

static TGP_FUNCTION( f98 )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 a = fifoin_pop(state);
	(void)a;
	logerror("TGP load list start %d (%x)\n", a, state->m_pushpc);
	state->m_fifoin_cbcount = state->m_list_length;
	state->m_fifoin_cb = f98_load;
}

static TGP_FUNCTION( f99 )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP f99 (%x)\n", state->m_pushpc);
	next_fn(state);
}

static TGP_FUNCTION( f100 )
{
	model1_state *state = machine.driver_data<model1_state>();
	int i;
	logerror("TGP f100 get list (%x)\n", state->m_pushpc);
	for(i=0; i<state->m_list_length; i++)
		fifoout_push_f(state, (machine.rand() % 1000)/100.0);
	next_fn(state);
}

static TGP_FUNCTION( groundbox_set )
{
	model1_state *state = machine.driver_data<model1_state>();
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	float f = fifoin_pop_f(state);
	float g = fifoin_pop_f(state);
	logerror("TGP groundbox_set %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, state->m_pushpc);
	state->m_tgp_vf_xmin = e;
	state->m_tgp_vf_xmax = d;
	state->m_tgp_vf_zmin = g;
	state->m_tgp_vf_zmax = f;
	state->m_tgp_vf_ygnd = b;
	state->m_tgp_vf_yflr = a;
	state->m_tgp_vf_yjmp = c;

	next_fn(state);
}

static TGP_FUNCTION( f102 )
{
	model1_state *state = machine.driver_data<model1_state>();
	float px, py, pz;
	float a = fifoin_pop_f(state);
	float b = fifoin_pop_f(state);
	float c = fifoin_pop_f(state);
	float d = fifoin_pop_f(state);
	float e = fifoin_pop_f(state);
	UINT32 f = fifoin_pop(state);
	UINT32 g = fifoin_pop(state);
	UINT32 h = fifoin_pop(state);

	state->m_ccount++;

	logerror("TGP f0 mve_calc %f, %f, %f, %f, %f, %d, %d, %d (%d) (%x)\n", a, b, c, d, e, f, g, h, state->m_ccount, state->m_pushpc);

	px = u2f(state->m_ram_data[state->m_ram_scanadr+0x16]);
	py = u2f(state->m_ram_data[state->m_ram_scanadr+0x17]);
	pz = u2f(state->m_ram_data[state->m_ram_scanadr+0x18]);

	//  memset(state->m_cmat, 0, sizeof(state->m_cmat));
	//  state->m_cmat[0] = 1.0;
	//  state->m_cmat[4] = 1.0;
	//  state->m_cmat[8] = 1.0;

	px = c;
	py = d;
	pz = e;

#if 1
	state->m_cmat[ 9] += state->m_cmat[0]*a+state->m_cmat[3]*b+state->m_cmat[6]*c;
	state->m_cmat[10] += state->m_cmat[1]*a+state->m_cmat[4]*b+state->m_cmat[7]*c;
	state->m_cmat[11] += state->m_cmat[2]*a+state->m_cmat[5]*b+state->m_cmat[8]*c;
#else
	state->m_cmat[ 9] += px;
	state->m_cmat[10] += py;
	state->m_cmat[11] += pz;
#endif

	logerror("    f0 mve_calc %f, %f, %f\n", px, py, pz);

	fifoout_push_f(state, c);
	fifoout_push_f(state, d);
	fifoout_push_f(state, e);
	fifoout_push(state, f);
	fifoout_push(state, g);
	fifoout_push(state, h);


	next_fn(state);
}

static TGP_FUNCTION( f103 )
{
	model1_state *state = machine.driver_data<model1_state>();
	state->m_ram_scanadr = fifoin_pop(state) - 0x8000;
	logerror("TGP f0 mve_setadr 0x%x (%x)\n", state->m_ram_scanadr, state->m_pushpc);
	ram_get_i(state);
	next_fn(state);
}

struct function {
	tgp_func cb;
	int count;
};

static const struct function ftab_vf[] = {
	{ fadd,            2 }, /* 0x00 */
	{ fsub,            2 },
	{ fmul,            2 },
	{ fdiv,            2 },
	{ NULL,            0 },
	{ matrix_push,     0 },
	{ matrix_pop,      0 },
	{ matrix_write,   12 },
	{ clear_stack,     0 },
	{ NULL,            0 },
	{ anglev,          2 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ track_select,    1 },
	{ f14,             4 },
	{ anglep,          4 },

	{ matrix_ident,    0 },  /* 0x10 */
	{ matrix_read,     0 },
	{ matrix_trans,    3 },
	{ matrix_scale,    3 },
	{ matrix_rotx,     1 },
	{ matrix_roty,     1 },
	{ matrix_rotz,     1 },
	{ NULL,            0 },
	{ track_read_quad, 1 },
	{ NULL,            0 },
	{ transform_point, 3 },
	{ fsin_m1,         1 },
	{ fcos_m1,         1 },
	{ fsinm_m1,        2 },
	{ fcosm_m1,        2 },
	{ distance3,       6 },

	{ NULL,            0 },  /* 0x20 */
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ acc_set,         1 },
	{ acc_get,         0 },
	{ acc_add,         1 },
	{ acc_sub,         1 },
	{ acc_mul,         1 },
	{ acc_div,         1 }, // not used ?
	{ f42,             3 },
	{ f43,             6 },
	{ f44,             1 },
	{ f45,             1 },
	{ vlength,         3 },
	{ NULL, 0 },

	{ track_read_info, 1 },  /* 0x30 */
	{ colbox_set,     12 },
	{ colbox_test,     3 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ track_lookup,    4 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },

	{ col_setcirc,     3 },  /* 0x40 */
	{ col_testpt,      2 },
	{ NULL,            0 },
	{ distance,        4 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ car_move,        4 },
	{ cpa,            12 },
	{ NULL,            0 },
	{ vmat_store,      1 },
	{ vmat_restore,    1 },
	{ NULL,            0 },
	{ vmat_mul,        2 },
	{ vmat_read,       1 },
	{ matrix_unrot,    0 },

	{ f80,             0 },  /* 0x50 */
	{ NULL,            0 },
	{ matrix_rtrans,   0 },
	{ NULL,            0 },
	{ vmat_save,       1 },
	{ vmat_load,       1 },
	{ ram_setadr,      1 },
	{ groundbox_test,  3 },
	{ NULL,            0 },
	{ f89,             4 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ f92,             4 },
	{ f93,             1 },
	{ f94,             1 },
	{ vmat_flatten,    0 },

	{ vmat_load1,      1 },  /* 0x60 */
	{ ram_trans,       0 },
	{ f98,             1 },
	{ f99,             0 },
	{ f100,            0 },
	{ groundbox_set,   7 },
	{ f102,            8 },
	{ f103,            1 }
};

// Used in swa scene 1 and unemulated:
//   f14
//   f49_swa
//   f15_swa

static const struct function ftab_swa[] = {
	{ fadd,            2 },  /* 0x00 */
	{ fsub,            2 },
	{ fmul,            2 },
	{ fdiv,            2 },
	{ NULL,            0 },
	{ matrix_push,     0 },
	{ matrix_pop,      0 },
	{ matrix_write,   12 },
	{ clear_stack,     0 },
	{ matrix_mul,     12 },
	{ anglev,          2 },
	{ f11,             9 },
	{ normalize,       3 },
	{ acc_seti,        1 },
	{ f14,             4 },
	{ f15_swa,         0 },

	{ matrix_ident,    0 }, /* 0x10 */
	{ matrix_read,     0 },
	{ matrix_trans,    3 },
	{ matrix_scale,    3 },
	{ matrix_rotx,     1 },
	{ matrix_roty,     1 },
	{ matrix_rotz,     1 },
	{ NULL,            0 },
	{ f24_swa,         7 },
	{ NULL,            0 },
	{ transform_point, 3 },
	{ fsin_m1,         1 },
	{ fcos_m1,         1 },
	{ fsinm_m1,        2 },
	{ fcosm_m1,        2 },
	{ distance3,       6 },

	{ NULL,            0 }, /* 0x20 */
	{ NULL,            0 },
	{ ftoi,            1 },
	{ itof,            1 },
	{ acc_set,         1 },
	{ acc_get,         0 },
	{ acc_add,         1 },
	{ acc_sub,         1 },
	{ acc_mul,         1 },
	{ acc_div,         1 }, // not used ?
	{ xyz2rqf,         3 },
	{ f43_swa,         3 },
	{ matrix_sdir,     3 },
	{ f45,             1 },
	{ vlength,         3 },
	{ f47,             3 },

	{ NULL,            0 }, /* 0x30 */
	{ f49_swa,         6 },
	{ f50_swa,         4 },
	{ NULL,            0 },
	{ f52,             0 },
	{ matrix_rdir,     3 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ f56,             7 },
	{ f57,             0 },
	{ matrix_readt,    0 },
	{ acc_geti,        0 },
	{ f60,             0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },
	{ NULL,            0 },

	{ push_and_ident,  0 }, /* 0x40 */
	{ NULL,            0 },
	{ catmull_rom,    13 }
};


static TGP_FUNCTION( dump )
{
	model1_state *state = machine.driver_data<model1_state>();
	logerror("TGP FIFOIN write %08x (%x)\n", fifoin_pop(state), state->m_pushpc);
	state->m_fifoin_cbcount = 1;
	state->m_fifoin_cb = dump;
}

static TGP_FUNCTION( function_get_vf )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 f = fifoin_pop(state) >> 23;

	if(state->m_fifoout_rpos != state->m_fifoout_wpos) {
		int count = state->m_fifoout_wpos - state->m_fifoout_rpos;
		if(count < 0)
			count += FIFO_SIZE;
		logerror("TGP function called with sizeout = %d\n", count);
	}
	if(ARRAY_LENGTH(ftab_vf) > f && NULL != ftab_vf[f].cb) {
		state->m_fifoin_cbcount = ftab_vf[f].count;
		state->m_fifoin_cb = ftab_vf[f].cb;
		//      logerror("TGP function %d request, %d parameters\n", f, state->m_fifoin_cbcount);
		if(!state->m_fifoin_cbcount)
			state->m_fifoin_cb(machine);
	} else {
		logerror("TGP function %d unimplemented (%x)\n", f, state->m_pushpc);
		state->m_fifoin_cbcount = 1;
		state->m_fifoin_cb = dump;
	}
}

static TGP_FUNCTION( function_get_swa )
{
	model1_state *state = machine.driver_data<model1_state>();
	UINT32 f = fifoin_pop(state);

	if(state->m_fifoout_rpos != state->m_fifoout_wpos) {
		int count = state->m_fifoout_wpos - state->m_fifoout_rpos;
		if(count < 0)
			count += FIFO_SIZE;
		logerror("TGP function called with sizeout = %d\n", count);
	}
	if(ARRAY_LENGTH(ftab_swa) > f && NULL != ftab_swa[f].cb) {
		state->m_fifoin_cbcount = ftab_swa[f].count;
		state->m_fifoin_cb = ftab_swa[f].cb;
		//      logerror("TGP function %d request, %d parameters\n", f, state->m_fifoin_cbcount);
		if(!state->m_fifoin_cbcount)
			state->m_fifoin_cb(machine);
	} else {
		logerror("TGP function %d unimplemented (%x)\n", f, state->m_pushpc);
		state->m_fifoin_cbcount = 1;
		state->m_fifoin_cb = dump;
	}
}

READ16_MEMBER(model1_state::model1_tgp_copro_r)
{
	if(!offset) {
		m_copro_r = fifoout_pop(&space);
		return m_copro_r;
	} else
		return m_copro_r >> 16;
}

WRITE16_MEMBER(model1_state::model1_tgp_copro_w)
{
	if(offset) {
		m_copro_w = (m_copro_w & 0x0000ffff) | (data << 16);
		m_pushpc = cpu_get_pc(&space.device());
		fifoin_push(&space, m_copro_w);
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
		logerror("TGP f0 ram read %04x, %08x (%f) (%x)\n", m_ram_adr, m_ram_data[m_ram_adr], u2f(m_ram_data[m_ram_adr]), cpu_get_pc(&space.device()));
		return m_ram_data[m_ram_adr];
	} else
		return m_ram_data[m_ram_adr++] >> 16;
}

WRITE16_MEMBER(model1_state::model1_tgp_copro_ram_w)
{
	COMBINE_DATA(m_ram_latch+offset);
	if(offset) {
		UINT32 v = m_ram_latch[0]|(m_ram_latch[1]<<16);
		logerror("TGP f0 ram write %04x, %08x (%f) (%x)\n", m_ram_adr, v, u2f(v), cpu_get_pc(&space.device()));
		m_ram_data[m_ram_adr] = v;
		m_ram_adr++;
	}
}

MACHINE_START( model1 )
{
	model1_state *state = machine.driver_data<model1_state>();
	state->m_ram_data = auto_alloc_array(machine, UINT32, 0x10000);

	state_save_register_global_pointer(machine, state->m_ram_data, 0x10000);
	state_save_register_global(machine, state->m_ram_adr);
	state_save_register_global(machine, state->m_ram_scanadr);
	state_save_register_global_array(machine, state->m_ram_latch);
	state_save_register_global(machine, state->m_fifoout_rpos);
	state_save_register_global(machine, state->m_fifoout_wpos);
	state_save_register_global_array(machine, state->m_fifoout_data);
	state_save_register_global(machine, state->m_fifoin_rpos);
	state_save_register_global(machine, state->m_fifoin_wpos);
	state_save_register_global_array(machine, state->m_fifoin_data);
	state_save_register_global_array(machine, state->m_cmat);
	state_save_register_global_2d_array(machine, state->m_mat_stack);
	state_save_register_global_2d_array(machine, state->m_mat_vector);
	state_save_register_global(machine, state->m_mat_stack_pos);
	state_save_register_global(machine, state->m_acc);
	state_save_register_global(machine, state->m_list_length);
}

void model1_tgp_reset(running_machine &machine, int swa)
{
	model1_state *state = machine.driver_data<model1_state>();
	state->m_ram_adr = 0;
	memset(state->m_ram_data, 0, 0x10000*4);

	state->m_fifoout_rpos = 0;
	state->m_fifoout_wpos = 0;
	state->m_fifoin_rpos = 0;
	state->m_fifoin_wpos = 0;

	state->m_acc = 0;
	state->m_mat_stack_pos = 0;
	memset(state->m_cmat, 0, sizeof(state->m_cmat));
	state->m_cmat[0] = 1.0;
	state->m_cmat[4] = 1.0;
	state->m_cmat[8] = 1.0;

	state->m_dump = 0;
	state->m_swa = swa;
	next_fn(state);
}

/*********************************** Virtua Racing ***********************************/


void model1_vr_tgp_reset( running_machine &machine )
{
	model1_state *state = machine.driver_data<model1_state>();
	state->m_ram_adr = 0;
	memset(state->m_ram_data, 0, 0x8000*4);

	state->m_copro_fifoout_rpos = 0;
	state->m_copro_fifoout_wpos = 0;
	state->m_copro_fifoout_num = 0;
	state->m_copro_fifoin_rpos = 0;
	state->m_copro_fifoin_wpos = 0;
	state->m_copro_fifoin_num = 0;
}

/* FIFO */
static int copro_fifoin_pop(device_t *device, UINT32 *result)
{
	model1_state *state = device->machine().driver_data<model1_state>();
	UINT32 r;

	if (state->m_copro_fifoin_num == 0)
	{
		return 0;
	}

	r = state->m_copro_fifoin_data[state->m_copro_fifoin_rpos++];

	if (state->m_copro_fifoin_rpos == FIFO_SIZE)
	{
		state->m_copro_fifoin_rpos = 0;
	}

	state->m_copro_fifoin_num--;

	*result = r;

	return 1;
}

static void copro_fifoin_push(address_space *space, UINT32 data)
{
	model1_state *state = space->machine().driver_data<model1_state>();
	if (state->m_copro_fifoin_num == FIFO_SIZE)
	{
		fatalerror("Copro FIFOIN overflow (at %08X)", cpu_get_pc(&space->device()));
		return;
	}

	state->m_copro_fifoin_data[state->m_copro_fifoin_wpos++] = data;

	if (state->m_copro_fifoin_wpos == FIFO_SIZE)
	{
		state->m_copro_fifoin_wpos = 0;
	}

	state->m_copro_fifoin_num++;
}

static UINT32 copro_fifoout_pop(address_space *space)
{
	model1_state *state = space->machine().driver_data<model1_state>();
	UINT32 r;

	if (state->m_copro_fifoout_num == 0)
	{
		// Reading from empty FIFO causes the v60 to enter wait state
		v60_stall(space->machine().device("maincpu"));

		space->machine().scheduler().synchronize();

		return 0;
	}

	r = state->m_copro_fifoout_data[state->m_copro_fifoout_rpos++];

	if (state->m_copro_fifoout_rpos == FIFO_SIZE)
	{
		state->m_copro_fifoout_rpos = 0;
	}

	state->m_copro_fifoout_num--;

	return r;
}

static void copro_fifoout_push(device_t *device, UINT32 data)
{
	model1_state *state = device->machine().driver_data<model1_state>();
	if (state->m_copro_fifoout_num == FIFO_SIZE)
	{
		fatalerror("Copro FIFOOUT overflow (at %08X)", cpu_get_pc(device));
		return;
	}

	state->m_copro_fifoout_data[state->m_copro_fifoout_wpos++] = data;

	if (state->m_copro_fifoout_wpos == FIFO_SIZE)
	{
		state->m_copro_fifoout_wpos = 0;
	}

	state->m_copro_fifoout_num++;
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
		device_spin_until_time(&space.device(), attotime::from_usec(100));
	}

	return m_ram_adr;
}

WRITE16_MEMBER(model1_state::model1_tgp_vr_adr_w)
{
	COMBINE_DATA(&m_ram_adr);
}

READ16_MEMBER(model1_state::model1_vr_tgp_ram_r)
{
	UINT16	r;

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
			device_spin_until_time(&space.device(), attotime::from_usec(100));
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
		m_vr_r = copro_fifoout_pop(&space);
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
		copro_fifoin_push(&space, m_vr_w);
	}
	else
		m_vr_w = (m_vr_w & 0xffff0000) | data;
}

/* TGP config */
const mb86233_cpu_core model1_vr_tgp_config =
{
	copro_fifoin_pop,
	copro_fifoout_push,
	"user5"
};

/* TGP memory map */
ADDRESS_MAP_START( model1_vr_tgp_map, AS_PROGRAM, 32, model1_state )
	AM_RANGE(0x00000000, 0x000007ff) AM_RAM AM_REGION("tgp", 0)
	AM_RANGE(0x00400000, 0x00407fff) AM_READWRITE(copro_ram_r, copro_ram_w)
	AM_RANGE(0xff800000, 0xff87ffff) AM_ROM AM_REGION("user2", 0)
ADDRESS_MAP_END
