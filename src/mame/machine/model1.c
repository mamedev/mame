/*
** Model 1 coprocessor TGP simulation
*/

#include "emu.h"
#include "debugger.h"
#include "cpu/mb86233/mb86233.h"
#include "cpu/v60/v60.h"
#include "includes/model1.h"

#define TGP_FUNCTION(name) void name(running_machine *machine)
typedef void (*tgp_func)(running_machine *machine);


enum {FIFO_SIZE = 256};
enum {MAT_STACK_SIZE = 32};

int model1_dump;

static offs_t pushpc;

static int fifoin_rpos, fifoin_wpos;
static UINT32 fifoin_data[FIFO_SIZE];
static int model1_swa;

static int fifoin_cbcount;
static tgp_func fifoin_cb;

static INT32 fifoout_rpos, fifoout_wpos;
static UINT32 fifoout_data[FIFO_SIZE];

static UINT32 list_length;

static float cmat[12], mat_stack[MAT_STACK_SIZE][12], mat_vector[21][12];
static INT32 mat_stack_pos;
static float acc;

static float tgp_vf_xmin, tgp_vf_xmax, tgp_vf_zmin, tgp_vf_zmax, tgp_vf_ygnd, tgp_vf_yflr, tgp_vf_yjmp;
static float tgp_vr_circx, tgp_vr_circy, tgp_vr_circrad, tgp_vr_cbox[12];
static int tgp_vr_select;
static UINT16 ram_adr, ram_latch[2], ram_scanadr;
static UINT32 *ram_data;
static float tgp_vr_base[4];

static UINT32 fifoout_pop(address_space *space)
{
	UINT32 v;
	if(fifoout_wpos == fifoout_rpos) {
		fatalerror("TGP FIFOOUT underflow (%x)", cpu_get_pc(space->cpu));
	}
	v = fifoout_data[fifoout_rpos++];
	if(fifoout_rpos == FIFO_SIZE)
		fifoout_rpos = 0;
	return v;
}

static int puuu = 0;

static void fifoout_push(UINT32 data)
{
	if(!puuu)
		logerror("TGP: Push %d\n", data);
	else
		puuu = 0;
	fifoout_data[fifoout_wpos++] = data;
	if(fifoout_wpos == FIFO_SIZE)
		fifoout_wpos = 0;
	if(fifoout_wpos == fifoout_rpos)
		logerror("TGP FIFOOUT overflow\n");
}

static void fifoout_push_f(float data)
{
	puuu = 1;

	logerror("TGP: Push %f\n", data);
	fifoout_push(f2u(data));
}

static UINT32 fifoin_pop(void)
{
	UINT32 v;
	if(fifoin_wpos == fifoin_rpos)
		logerror("TGP FIFOIN underflow\n");
	v = fifoin_data[fifoin_rpos++];
	if(fifoin_rpos == FIFO_SIZE)
		fifoin_rpos = 0;
	return v;
}

static void fifoin_push(address_space *space, UINT32 data)
{
	//  logerror("TGP FIFOIN write %08x (%x)\n", data, cpu_get_pc(space->cpu));
	fifoin_data[fifoin_wpos++] = data;
	if(fifoin_wpos == FIFO_SIZE)
		fifoin_wpos = 0;
	if(fifoin_wpos == fifoin_rpos)
		logerror("TGP FIFOIN overflow\n");
	fifoin_cbcount--;
	if(!fifoin_cbcount)
		fifoin_cb(space->machine);
}

static float fifoin_pop_f(void)
{
	return u2f(fifoin_pop());
}

static TGP_FUNCTION( function_get_vf );
static TGP_FUNCTION( function_get_swa );

static void next_fn(void)
{
	fifoin_cbcount = 1;
	fifoin_cb = model1_swa ? function_get_swa : function_get_vf;
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

static UINT16 ram_get_i(void)
{
	return ram_data[ram_scanadr++];
}

static float ram_get_f(void)
{
	return u2f(ram_data[ram_scanadr++]);
}

static TGP_FUNCTION( fadd )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float r = a+b;
	logerror("TGP fadd %f+%f=%f (%x)\n", a, b, r, pushpc);
	fifoout_push_f(r);
	next_fn();
}

static TGP_FUNCTION( fsub )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float r = a-b;
	model1_dump = 1;
	logerror("TGP fsub %f-%f=%f (%x)\n", a, b, r, pushpc);
	fifoout_push_f(r);
	next_fn();
}

static TGP_FUNCTION( fmul )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float r = a*b;
	logerror("TGP fmul %f*%f=%f (%x)\n", a, b, r, pushpc);
	fifoout_push_f(r);
	next_fn();
}

static TGP_FUNCTION( fdiv )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
//  float r = !b ? 1e39 : a/b;
	float r = !b ? 0 : a * (1/b);
	logerror("TGP fdiv %f/%f=%f (%x)\n", a, b, r, pushpc);
	fifoout_push_f(r);
	next_fn();
}

static TGP_FUNCTION( matrix_push )
{
	if(mat_stack_pos != MAT_STACK_SIZE) {
		memcpy(mat_stack[mat_stack_pos], cmat, sizeof(cmat));
		mat_stack_pos++;
	}
	logerror("TGP matrix_push (depth=%d, pc=%x)\n", mat_stack_pos, pushpc);
	next_fn();
}

static TGP_FUNCTION( matrix_pop )
{
	if(mat_stack_pos) {
		mat_stack_pos--;
		memcpy(cmat, mat_stack[mat_stack_pos], sizeof(cmat));
	}
	logerror("TGP matrix_pop (depth=%d, pc=%x)\n", mat_stack_pos, pushpc);
	next_fn();
}

static TGP_FUNCTION( matrix_write )
{
	int i;
	for(i=0; i<12; i++)
		cmat[i] = fifoin_pop_f();
	logerror("TGP matrix_write %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f) (%x)\n",
			 cmat[0], cmat[1], cmat[2], cmat[3], cmat[4], cmat[5], cmat[6], cmat[7], cmat[8], cmat[9], cmat[10], cmat[11],
			 pushpc);
	next_fn();
}

static TGP_FUNCTION( clear_stack )
{
	logerror("TGP clear_stack (%x)\n", pushpc);
	mat_stack_pos = 0;
	next_fn();
}

static TGP_FUNCTION( matrix_mul )
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
	logerror("TGP matrix_mul %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, pushpc);
	m[0]  = a*cmat[0] + b*cmat[3] + c*cmat[6];
	m[1]  = a*cmat[1] + b*cmat[4] + c*cmat[7];
	m[2]  = a*cmat[2] + b*cmat[5] + c*cmat[8];
	m[3]  = d*cmat[0] + e*cmat[3] + f*cmat[6];
	m[4]  = d*cmat[1] + e*cmat[4] + f*cmat[7];
	m[5]  = d*cmat[2] + e*cmat[5] + f*cmat[8];
	m[6]  = g*cmat[0] + h*cmat[3] + i*cmat[6];
	m[7]  = g*cmat[1] + h*cmat[4] + i*cmat[7];
	m[8]  = g*cmat[2] + h*cmat[5] + i*cmat[8];
	m[9]  = j*cmat[0] + k*cmat[3] + l*cmat[6] + cmat[9];
	m[10] = j*cmat[1] + k*cmat[4] + l*cmat[7] + cmat[10];
	m[11] = j*cmat[2] + k*cmat[5] + l*cmat[8] + cmat[11];

	memcpy(cmat, m, sizeof(m));
	next_fn();
}

static TGP_FUNCTION( anglev )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	logerror("TGP anglev %f, %f (%x)\n", a, b, pushpc);
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

static TGP_FUNCTION( f11 )
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
	logerror("TGP f11 %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

static TGP_FUNCTION( normalize )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float n = (a*a+b*b+c*c) / sqrt(a*a+b*b+c*c);
	logerror("TGP normalize %f, %f, %f (%x)\n", a, b, c, pushpc);
	fifoout_push_f(a/n);
	fifoout_push_f(b/n);
	fifoout_push_f(c/n);
	next_fn();
}

static TGP_FUNCTION( acc_seti )
{
	INT32 a = fifoin_pop();
	model1_dump = 1;
	logerror("TGP acc_seti %d (%x)\n", a, pushpc);
	acc = a;
	next_fn();
}

static TGP_FUNCTION( track_select )
{
	INT32 a = fifoin_pop();
	logerror("TGP track_select %d (%x)\n", a, pushpc);
	tgp_vr_select = a;
	next_fn();
}

static TGP_FUNCTION( f14 )
{
	tgp_vr_base[0] = fifoin_pop_f();
	tgp_vr_base[1] = fifoin_pop_f();
	tgp_vr_base[2] = fifoin_pop_f();
	tgp_vr_base[3] = fifoin_pop_f();

	next_fn();
}

static TGP_FUNCTION( f15_swa )
{
	logerror("TGP f15_swa (%x)\n", pushpc);

	next_fn();
}

static TGP_FUNCTION( anglep )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	logerror("TGP anglep %f, %f, %f, %f (%x)\n", a, b, c, d, pushpc);
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

static TGP_FUNCTION( matrix_ident )
{
	logerror("TGP matrix_ident (%x)\n", pushpc);
	memset(cmat, 0, sizeof(cmat));
	cmat[0] = 1.0;
	cmat[4] = 1.0;
	cmat[8] = 1.0;
	next_fn();
}

static TGP_FUNCTION( matrix_read )
{
	int i;
	logerror("TGP matrix_read (%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f) (%x)\n",
			 cmat[0], cmat[1], cmat[2], cmat[3], cmat[4], cmat[5], cmat[6], cmat[7], cmat[8], cmat[9], cmat[10], cmat[11], pushpc);
	for(i=0; i<12; i++)
		fifoout_push_f(cmat[i]);
	next_fn();
}

static TGP_FUNCTION( matrix_trans )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();

	cmat[ 9] += cmat[0]*a+cmat[3]*b+cmat[6]*c;
	cmat[10] += cmat[1]*a+cmat[4]*b+cmat[7]*c;
	cmat[11] += cmat[2]*a+cmat[5]*b+cmat[8]*c;
	next_fn();
}

static TGP_FUNCTION( matrix_scale )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	logerror("TGP matrix_scale %f, %f, %f (%x)\n", a, b, c, pushpc);
	cmat[0] *= a;
	cmat[1] *= a;
	cmat[2] *= a;
	cmat[3] *= b;
	cmat[4] *= b;
	cmat[5] *= b;
	cmat[6] *= c;
	cmat[7] *= c;
	cmat[8] *= c;
	next_fn();
}

static TGP_FUNCTION( matrix_rotx )
{
	INT16 a = fifoin_pop();
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;
	logerror("TGP matrix_rotx %d (%x)\n", a, pushpc);
	t1 = cmat[3];
	t2 = cmat[6];
	cmat[3] = c*t1-s*t2;
	cmat[6] = s*t1+c*t2;
	t1 = cmat[4];
	t2 = cmat[7];
	cmat[4] = c*t1-s*t2;
	cmat[7] = s*t1+c*t2;
	t1 = cmat[5];
	t2 = cmat[8];
	cmat[5] = c*t1-s*t2;
	cmat[8] = s*t1+c*t2;
	next_fn();
}

static TGP_FUNCTION( matrix_roty )
{
	INT16 a = fifoin_pop();
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;

	logerror("TGP matrix_roty %d (%x)\n", a, pushpc);
	t1 = cmat[6];
	t2 = cmat[0];
	cmat[6] = c*t1-s*t2;
	cmat[0] = s*t1+c*t2;
	t1 = cmat[7];
	t2 = cmat[1];
	cmat[7] = c*t1-s*t2;
	cmat[1] = s*t1+c*t2;
	t1 = cmat[8];
	t2 = cmat[2];
	cmat[8] = c*t1-s*t2;
	cmat[2] = s*t1+c*t2;
	next_fn();
}

static TGP_FUNCTION( matrix_rotz )
{
	INT16 a = fifoin_pop();
	float s = tsin(a);
	float c = tcos(a);
	float t1, t2;

	logerror("TGP matrix_rotz %d (%x)\n", a, pushpc);
	t1 = cmat[0];
	t2 = cmat[3];
	cmat[0] = c*t1-s*t2;
	cmat[3] = s*t1+c*t2;
	t1 = cmat[1];
	t2 = cmat[4];
	cmat[1] = c*t1-s*t2;
	cmat[4] = s*t1+c*t2;
	t1 = cmat[2];
	t2 = cmat[5];
	cmat[2] = c*t1-s*t2;
	cmat[5] = s*t1+c*t2;
	next_fn();
}

static TGP_FUNCTION( track_read_quad )
{
	const UINT32 *tgp_data = (const UINT32 *)memory_region(machine, "user2");
	UINT32 a = fifoin_pop();
	int offd;

	logerror("TGP track_read_quad %d (%x)\n", a, pushpc);

	offd = tgp_data[0x20+tgp_vr_select] + 16*a;
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

static TGP_FUNCTION( f24_swa )
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
	logerror("TGP f24_swa %f, %f, %f, %f, %f, %f, %x (%x)\n", a, b, c, d, e, f, g, pushpc);
	fifoout_push_f(0);
	next_fn();
}

static TGP_FUNCTION( transform_point )
{
	float x = fifoin_pop_f();
	float y = fifoin_pop_f();
	float z = fifoin_pop_f();
	logerror("TGP transform_point %f, %f, %f (%x)\n", x, y, z, pushpc);

	fifoout_push_f(cmat[0]*x+cmat[3]*y+cmat[6]*z+cmat[9]);
	fifoout_push_f(cmat[1]*x+cmat[4]*y+cmat[7]*z+cmat[10]);
	fifoout_push_f(cmat[2]*x+cmat[5]*y+cmat[8]*z+cmat[11]);
	next_fn();
}

static TGP_FUNCTION( fcos_m1 )
{
    INT16 a = fifoin_pop();
	logerror("TGP fcos %d (%x)\n", a, pushpc);
	fifoout_push_f(tcos(a));
	next_fn();
}

static TGP_FUNCTION( fsin_m1 )
{
    INT16 a = fifoin_pop();
	logerror("TGP fsin %d (%x)\n", a, pushpc);
	fifoout_push_f(tsin(a));
	next_fn();
}

static TGP_FUNCTION( fcosm_m1 )
{
    INT16 a = fifoin_pop();
	float b = fifoin_pop_f();
	logerror("TGP fcosm %d, %f (%x)\n", a, b, pushpc);
	fifoout_push_f(b*tcos(a));
	next_fn();
}

static TGP_FUNCTION( fsinm_m1 )
{
    INT16 a = fifoin_pop();
	float b = fifoin_pop_f();
	model1_dump = 1;
	logerror("TGP fsinm %d, %f (%x)\n", a, b, pushpc);
	fifoout_push_f(b*tsin(a));
	next_fn();
}

static TGP_FUNCTION( distance3 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	logerror("TGP distance3 (%f, %f, %f), (%f, %f, %f) (%x)\n", a, b, c, d, e, f, pushpc);
	a -= d;
	b -= e;
	c -= f;
	fifoout_push_f((a*a+b*b+c*c)/sqrt(a*a+b*b+c*c));
	next_fn();
}

static TGP_FUNCTION( ftoi )
{
	float a = fifoin_pop_f();
	logerror("TGP ftoi %f (%x)\n", a, pushpc);
	fifoout_push((int)a);
	next_fn();
}

static TGP_FUNCTION( itof )
{
	INT32 a = fifoin_pop();
	logerror("TGP itof %d (%x)\n", a, pushpc);
	fifoout_push_f(a);
	next_fn();
}

static TGP_FUNCTION( acc_set )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_set %f (%x)\n", a, pushpc);
	acc = a;
	next_fn();
}

static TGP_FUNCTION( acc_get )
{
	logerror("TGP acc_get (%x)\n", pushpc);
	fifoout_push_f(acc);
	next_fn();
}

static TGP_FUNCTION( acc_add )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_add %f (%x)\n", a, pushpc);
	acc += a;
	next_fn();
}

static TGP_FUNCTION( acc_sub )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_sub %f (%x)\n", a, pushpc);
	acc -= a;
	next_fn();
}

static TGP_FUNCTION( acc_mul )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_mul %f (%x)\n", a, pushpc);
	acc *= a;
	next_fn();
}

static TGP_FUNCTION( acc_div )
{
	float a = fifoin_pop_f();
	logerror("TGP acc_div %f (%x)\n", a, pushpc);
	acc /= a;
	next_fn();
}

static TGP_FUNCTION( f42 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP f42 %f, %f, %f (%x)\n", a, b, c, pushpc);
	//  fifoout_push_f((mame_rand(machine) % 1000) - 500);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}


// r = (x2 + y2 + z2)1/2,     f = tan-1(y/(x2+z2)1/2),     q = tan-1(z/x)

static TGP_FUNCTION( xyz2rqf )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float norm;
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP xyz2rqf %f, %f, %f (%x)\n", a, b, c, pushpc);
	fifoout_push_f((a*a+b*b+c*c)/sqrt(a*a+b*b+c*c));
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

static TGP_FUNCTION( f43 )
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
	logerror("TGP f43 %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

static TGP_FUNCTION( f43_swa )
{
	float a = fifoin_pop_f();
	int b = fifoin_pop();
	int c = fifoin_pop();
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP f43_swa %f, %d, %d (%x)\n", a, b, c, pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

static TGP_FUNCTION( f44 )
{
	float a = fifoin_pop_f();
	(void)a;
	logerror("TGP f44 %f (%x)\n", a, pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

static TGP_FUNCTION( matrix_sdir )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float norm = sqrt(a*a+b*b+c*c);
	float t[9], m[9];
	logerror("TGP matrix_sdir %f, %f, %f (%x)\n", a, b, c, pushpc);

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

	m[0]  = t[0]*cmat[0] + t[1]*cmat[3] + t[2]*cmat[6];
	m[1]  = t[0]*cmat[1] + t[1]*cmat[4] + t[2]*cmat[7];
	m[2]  = t[0]*cmat[2] + t[1]*cmat[5] + t[2]*cmat[8];
	m[3]  = t[3]*cmat[0] + t[4]*cmat[3] + t[5]*cmat[6];
	m[4]  = t[3]*cmat[1] + t[4]*cmat[4] + t[5]*cmat[7];
	m[5]  = t[3]*cmat[2] + t[4]*cmat[5] + t[5]*cmat[8];
	m[6]  = t[6]*cmat[0] + t[7]*cmat[3] + t[8]*cmat[6];
	m[7]  = t[6]*cmat[1] + t[7]*cmat[4] + t[8]*cmat[7];
	m[8]  = t[6]*cmat[2] + t[7]*cmat[5] + t[8]*cmat[8];

	memcpy(cmat, m, sizeof(m));

	next_fn();
}

static TGP_FUNCTION( f45 )
{
	float a = fifoin_pop_f();
	(void)a;
	logerror("TGP f45 %f (%x)\n", a, pushpc);
	fifoout_push_f(0);
	next_fn();
}

static TGP_FUNCTION( vlength )
{
	float a = fifoin_pop_f() - tgp_vr_base[0];
	float b = fifoin_pop_f() - tgp_vr_base[1];
	float c = fifoin_pop_f() - tgp_vr_base[2];
	logerror("TGP vlength %f, %f, %f (%x)\n", a, b, c, pushpc);

	a = (a*a+b*b+c*c);
	b = 1/sqrt(a);
	c = a * b;
	c -= tgp_vr_base[3];
	fifoout_push_f(c);
	next_fn();
}

static TGP_FUNCTION( f47 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	logerror("TGP f47 %f, %f, %f (%x)\n", a, b, c, pushpc);
	fifoout_push_f(a+c);
	fifoout_push_f(b+c);
	next_fn();
}

static TGP_FUNCTION( track_read_info )
{
	const UINT32 *tgp_data = (const UINT32 *)memory_region(machine, "user2");
    UINT16 a = fifoin_pop();
	int offd;

	logerror("TGP track_read_info %d (%x)\n", a, pushpc);

	offd = tgp_data[0x20+tgp_vr_select] + 16*a;
	fifoout_push(tgp_data[offd+15]);
	next_fn();
}

static TGP_FUNCTION( colbox_set )
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
	logerror("TGP colbox_set %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, pushpc);
	tgp_vr_cbox[ 0] = a;
	tgp_vr_cbox[ 1] = b;
	tgp_vr_cbox[ 2] = c;
	tgp_vr_cbox[ 3] = d;
	tgp_vr_cbox[ 4] = e;
	tgp_vr_cbox[ 5] = f;
	tgp_vr_cbox[ 6] = g;
	tgp_vr_cbox[ 7] = h;
	tgp_vr_cbox[ 8] = i;
	tgp_vr_cbox[ 9] = j;
	tgp_vr_cbox[10] = k;
	tgp_vr_cbox[11] = l;
	next_fn();
}

static TGP_FUNCTION( colbox_test )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP colbox_test %f, %f, %f (%x)\n", a, b, c, pushpc);

	// #### Wrong, need to check with the tgp_vr_cbox coordinates
	// Game only test sign, negative = collision
	fifoout_push_f(-1);
	next_fn();
}

static TGP_FUNCTION( f49_swa )
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
	logerror("TGP f49_swa %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, pushpc);
	next_fn();
}

static TGP_FUNCTION( f50_swa )
{
    float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	logerror("TGP f50_swa %f, %f, %f, %f (%x)\n", a, b, c, d, pushpc);
	fifoout_push_f(d);
	next_fn();
}

static TGP_FUNCTION( f52 )
{
	logerror("TGP f52 (%x)\n", pushpc);
	next_fn();
}

static TGP_FUNCTION( matrix_rdir )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float norm = sqrt(a*a+c*c);
	float t1, t2;
	(void)b;

	logerror("TGP matrix_rdir %f, %f, %f (%x)\n", a, b, c, pushpc);

	if(!norm) {
		c = 1;
		a = 0;
	} else {
		c /= norm;
		a /= norm;
	}

	t1 = cmat[6];
	t2 = cmat[0];
	cmat[6] = c*t1-a*t2;
	cmat[0] = a*t1+c*t2;
	t1 = cmat[7];
	t2 = cmat[1];
	cmat[7] = c*t1-a*t2;
	cmat[1] = a*t1+c*t2;
	t1 = cmat[8];
	t2 = cmat[2];
	cmat[8] = c*t1-a*t2;
	cmat[2] = a*t1+c*t2;
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

static TGP_FUNCTION( track_lookup )
{
	const UINT32 *tgp_data = (const UINT32 *)memory_region(machine, "user2");
	float a = fifoin_pop_f();
	UINT32 b = fifoin_pop();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	int offi, offd, len;
	float dist;
	int i;
	UINT32 behaviour, entry;
	float height;

	logerror("TGP track_lookup %f, 0x%x, %f, %f (%x)\n", a, b, c, d, pushpc);

	offi = tgp_data[0x10+tgp_vr_select] + b;
	offd = tgp_data[0x20+tgp_vr_select];

	len = tgp_data[offi++];

	dist = -1;

	behaviour = 0;
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
					behaviour = tgp_data[posd+15];
					height = z;
					entry = bpos+i;
				}
			}
		}
	}

	ram_data[0x0000] = 0; // non zero = still computing
	ram_data[0x8001] = f2u(height);
	ram_data[0x8002] = entry;

	next_fn();
}

static TGP_FUNCTION( f56 )
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

	logerror("TGP f56 %f, %f, %f, %f, %f, %f, %d (%x)\n", a, b, c, d, e, f, g, pushpc);
	fifoout_push(0);
	next_fn();
}

static TGP_FUNCTION( f57 )
{
	logerror("TGP f57 (%x)\n", pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

static TGP_FUNCTION( matrix_readt )
{
	logerror("TGP matrix_readt (%x)\n", pushpc);
	fifoout_push_f(cmat[9]);
	fifoout_push_f(cmat[10]);
	fifoout_push_f(cmat[11]);
	next_fn();
}

static TGP_FUNCTION( acc_geti )
{
	logerror("TGP acc_geti (%x)\n", pushpc);
	fifoout_push((int)acc);
	next_fn();
}

static TGP_FUNCTION( f60 )
{
	logerror("TGP f60 (%x)\n", pushpc);
	fifoout_push_f(0);
	fifoout_push_f(0);
	fifoout_push_f(0);
	next_fn();
}

static TGP_FUNCTION( col_setcirc )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	logerror("TGP col_setcirc %f, %f, %f (%x)\n", a, b, c, pushpc);
	tgp_vr_circx = a;
	tgp_vr_circy = b;
	tgp_vr_circrad = c;
	next_fn();
}

static TGP_FUNCTION( col_testpt )
{
	float x, y;
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	logerror("TGP col_testpt %f, %f (%x)\n", a, b, pushpc);
	x = a - tgp_vr_circx;
	y = b - tgp_vr_circy;
	fifoout_push_f(((x*x+y*y)/sqrt(x*x+y*y)) - tgp_vr_circrad);
	next_fn();
}

static TGP_FUNCTION( push_and_ident )
{
	if(mat_stack_pos != MAT_STACK_SIZE) {
		memcpy(mat_stack[mat_stack_pos], cmat, sizeof(cmat));
		mat_stack_pos++;
	}
	logerror("TGP push_and_ident (depth=%d, pc=%x)\n", mat_stack_pos, pushpc);
	memset(cmat, 0, sizeof(cmat));
	cmat[0] = 1.0;
	cmat[4] = 1.0;
	cmat[8] = 1.0;
	next_fn();
}

static TGP_FUNCTION( catmull_rom )
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

	logerror("TGP catmull_rom %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, m, pushpc);

	m2 = m*m;
	m3 = m*m*m;

	w1 = 0.5*(-m3+2*m2-m);
	w2 = 0.5*(3*m3-5*m2+2);
	w3 = 0.5*(-3*m3+4*m2+m);
	w4 = 0.5*(m3-m2);

	fifoout_push_f(a*w1+d*w2+g*w3+j*w4);
	fifoout_push_f(b*w1+e*w2+h*w3+k*w4);
	fifoout_push_f(c*w1+f*w2+i*w3+l*w4);
	next_fn();
}

static TGP_FUNCTION( distance )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	logerror("TGP distance (%f, %f), (%f, %f) (%x)\n", a, b, c, d, pushpc);
	c -= a;
	d -= b;
	fifoout_push_f((c*c+d*d)/sqrt(c*c+d*d));
	next_fn();
}

static TGP_FUNCTION( car_move )
{
	INT16 a = fifoin_pop();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float dx, dy;
	logerror("TGP car_move (%d, %f), (%f, %f) (%x)\n", a, b, c, d, pushpc);

	dx = b*tsin(a);
	dy = b*tcos(a);

	fifoout_push_f(dx);
	fifoout_push_f(dy);
	fifoout_push_f(c+dx);
	fifoout_push_f(d+dy);
	next_fn();
}

static TGP_FUNCTION( cpa )
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
	logerror("TGP cpa %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, h, i, j, k, l, pushpc);

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

	fifoout_push_f(sqrt(dv2));
	next_fn();
}

static TGP_FUNCTION( vmat_store )
{
	UINT32 a = fifoin_pop();
	if(a<21)
		memcpy(mat_vector[a], cmat, sizeof(cmat));
	else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_store %d (%x)\n", a, pushpc);
	next_fn();
}

static TGP_FUNCTION( vmat_restore )
{
	UINT32 a = fifoin_pop();
	if(a<21)
		memcpy(cmat, mat_vector[a], sizeof(cmat));
	else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_restore %d (%x)\n", a, pushpc);
	next_fn();
}

static TGP_FUNCTION( vmat_mul )
{
	UINT32 a = fifoin_pop();
	UINT32 b = fifoin_pop();
	if(a<21 && b<21) {
		mat_vector[b][0]  = mat_vector[a][ 0]*cmat[0] + mat_vector[a][ 1]*cmat[3] + mat_vector[a][ 2]*cmat[6];
		mat_vector[b][1]  = mat_vector[a][ 0]*cmat[1] + mat_vector[a][ 1]*cmat[4] + mat_vector[a][ 2]*cmat[7];
		mat_vector[b][2]  = mat_vector[a][ 0]*cmat[2] + mat_vector[a][ 1]*cmat[5] + mat_vector[a][ 2]*cmat[8];
		mat_vector[b][3]  = mat_vector[a][ 3]*cmat[0] + mat_vector[a][ 4]*cmat[3] + mat_vector[a][ 5]*cmat[6];
		mat_vector[b][4]  = mat_vector[a][ 3]*cmat[1] + mat_vector[a][ 4]*cmat[4] + mat_vector[a][ 5]*cmat[7];
		mat_vector[b][5]  = mat_vector[a][ 3]*cmat[2] + mat_vector[a][ 4]*cmat[5] + mat_vector[a][ 5]*cmat[8];
		mat_vector[b][6]  = mat_vector[a][ 6]*cmat[0] + mat_vector[a][ 7]*cmat[3] + mat_vector[a][ 8]*cmat[6];
		mat_vector[b][7]  = mat_vector[a][ 6]*cmat[1] + mat_vector[a][ 7]*cmat[4] + mat_vector[a][ 8]*cmat[7];
		mat_vector[b][8]  = mat_vector[a][ 6]*cmat[2] + mat_vector[a][ 7]*cmat[5] + mat_vector[a][ 8]*cmat[8];
		mat_vector[b][9]  = mat_vector[a][ 9]*cmat[0] + mat_vector[a][10]*cmat[3] + mat_vector[a][11]*cmat[6] + cmat[9];
		mat_vector[b][10] = mat_vector[a][ 9]*cmat[1] + mat_vector[a][10]*cmat[4] + mat_vector[a][11]*cmat[7] + cmat[10];
		mat_vector[b][11] = mat_vector[a][ 9]*cmat[2] + mat_vector[a][10]*cmat[5] + mat_vector[a][11]*cmat[8] + cmat[11];
	} else
		logerror("TGP ERROR bad vector index\n");
	logerror("TGP vmat_mul %d, %d (%x)\n", a, b, pushpc);
	next_fn();
}

static TGP_FUNCTION( vmat_read )
{
	UINT32 a = fifoin_pop();
	logerror("TGP vmat_read %d (%x)\n", a, pushpc);
	if(a<21) {
		int i;
		for(i=0; i<12; i++)
			fifoout_push_f(mat_vector[a][i]);
	} else {
		int i;
		logerror("TGP ERROR bad vector index\n");
		for(i=0; i<12; i++)
			fifoout_push_f(0);
	}
	next_fn();
}

static TGP_FUNCTION( matrix_rtrans )
{
	logerror("TGP matrix_rtrans (%x)\n", pushpc);
	fifoout_push_f(cmat[ 9]);
	fifoout_push_f(cmat[10]);
	fifoout_push_f(cmat[11]);
	next_fn();
}

static TGP_FUNCTION( matrix_unrot )
{
	logerror("TGP matrix_unrot (%x)\n", pushpc);
	memset(cmat, 0, 9*sizeof(cmat[0]));
	cmat[0] = 1.0;
	cmat[4] = 1.0;
	cmat[8] = 1.0;
	next_fn();
}

static TGP_FUNCTION( f80 )
{
	logerror("TGP f80 (%x)\n", pushpc);
	//  cmat[9] = cmat[10] = cmat[11] = 0;
	next_fn();
}

static TGP_FUNCTION( vmat_save )
{
	UINT32 a = fifoin_pop();
	int i;
	logerror("TGP vmat_save 0x%x (%x)\n", a, pushpc);
	for(i=0; i<16; i++)
		memcpy(ram_data+a+0x10*i, mat_vector[i], sizeof(cmat));
	next_fn();
}

static TGP_FUNCTION( vmat_load )
{
	UINT32 a = fifoin_pop();
	int i;
	logerror("TGP vmat_load 0x%x (%x)\n", a, pushpc);
	for(i=0; i<16; i++)
		memcpy(mat_vector[i], ram_data+a+0x10*i, sizeof(cmat));
	next_fn();
}

static TGP_FUNCTION( ram_setadr )
{
    ram_scanadr = fifoin_pop() - 0x8000;
	logerror("TGP f0 ram_setadr 0x%x (%x)\n", ram_scanadr+0x8000, pushpc);
	next_fn();
}

static TGP_FUNCTION( groundbox_test )
{
	int out_x, out_y, out_z;
	float x, /*y,*/ z;
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();

	logerror("TGP groundbox_test %f, %f, %f (%x)\n", a, b, c, pushpc);
	x = cmat[0]*a+cmat[3]*b+cmat[6]*c+cmat[9];
	//y = cmat[1]*a+cmat[4]*b+cmat[7]*c+cmat[10];
	z = cmat[2]*a+cmat[5]*b+cmat[8]*c+cmat[11];

	out_x = x < tgp_vf_xmin || x > tgp_vf_xmax;
	out_z = z < tgp_vf_zmin || z > tgp_vf_zmax;
	out_y = 1; // Wrong, but untestable it seems.

	fifoout_push(out_x);
	fifoout_push(out_y);
	fifoout_push(out_z);
	next_fn();
}

static TGP_FUNCTION( f89 )
{
    UINT32 a = fifoin_pop();
	UINT32 b = fifoin_pop();
    UINT32 c = fifoin_pop();
	UINT32 d = fifoin_pop();
	(void)a;
	(void)b;
	(void)c;
	logerror("TGP list set base 0x%x, 0x%x, %d, length=%d (%x)\n", a, b, c, d, pushpc);
	list_length = d;
	next_fn();
}

static TGP_FUNCTION( f92 )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	logerror("TGP f92 %f, %f, %f, %f (%x)\n", a, b, c, d, pushpc);
	next_fn();
}

static TGP_FUNCTION( f93 )
{
	float a = fifoin_pop_f();
	(void)a;
	logerror("TGP f93 %f (%x)\n", a, pushpc);
	next_fn();
}

static TGP_FUNCTION( f94 )
{
	UINT32 a = fifoin_pop();
	(void)a;
	logerror("TGP f94 %d (%x)\n", a, pushpc);
	next_fn();
}

static TGP_FUNCTION( vmat_flatten )
{
	int i;
	float m[12];
	logerror("TGP vmat_flatten (%x)\n", pushpc);

	for(i=0; i<16; i++) {
		memcpy(m, mat_vector[i], sizeof(cmat));
		m[1] = m[4] = m[7] = m[10] = 0;

		mat_vector[i][0]  = m[ 0]*cmat[0] + m[ 1]*cmat[3] + m[ 2]*cmat[6];
		mat_vector[i][1]  = m[ 0]*cmat[1] + m[ 1]*cmat[4] + m[ 2]*cmat[7];
		mat_vector[i][2]  = m[ 0]*cmat[2] + m[ 1]*cmat[5] + m[ 2]*cmat[8];
		mat_vector[i][3]  = m[ 3]*cmat[0] + m[ 4]*cmat[3] + m[ 5]*cmat[6];
		mat_vector[i][4]  = m[ 3]*cmat[1] + m[ 4]*cmat[4] + m[ 5]*cmat[7];
		mat_vector[i][5]  = m[ 3]*cmat[2] + m[ 4]*cmat[5] + m[ 5]*cmat[8];
		mat_vector[i][6]  = m[ 6]*cmat[0] + m[ 7]*cmat[3] + m[ 8]*cmat[6];
		mat_vector[i][7]  = m[ 6]*cmat[1] + m[ 7]*cmat[4] + m[ 8]*cmat[7];
		mat_vector[i][8]  = m[ 6]*cmat[2] + m[ 7]*cmat[5] + m[ 8]*cmat[8];
		mat_vector[i][9]  = m[ 9]*cmat[0] + m[10]*cmat[3] + m[11]*cmat[6] + cmat[9];
		mat_vector[i][10] = m[ 9]*cmat[1] + m[10]*cmat[4] + m[11]*cmat[7] + cmat[10];
		mat_vector[i][11] = m[ 9]*cmat[2] + m[10]*cmat[5] + m[11]*cmat[8] + cmat[11];
	}
	next_fn();
}

static TGP_FUNCTION( vmat_load1 )
{
	UINT32 a = fifoin_pop();
	logerror("TGP vmat_load1 0x%x (%x)\n", a, pushpc);
	memcpy(cmat, ram_data+a, sizeof(cmat));
	next_fn();
}

static TGP_FUNCTION( ram_trans )
{
	float a = ram_get_f();
	float b = ram_get_f();
	float c = ram_get_f();
	logerror("TGP ram_trans (%x)\n", pushpc);
	cmat[ 9] += cmat[0]*a+cmat[3]*b+cmat[6]*c;
	cmat[10] += cmat[1]*a+cmat[4]*b+cmat[7]*c;
	cmat[11] += cmat[2]*a+cmat[5]*b+cmat[8]*c;
	next_fn();
}

static TGP_FUNCTION( f98_load )
{
	int i;
	for(i=0; i<list_length; i++) {
		float f = fifoin_pop_f();
		(void)f;
		logerror("TGP load list (%2d/%2d) %f (%x)\n", i, list_length, f, pushpc);
	}
	next_fn();
}

static TGP_FUNCTION( f98 )
{
    UINT32 a = fifoin_pop();
	(void)a;
	logerror("TGP load list start %d (%x)\n", a, pushpc);
	fifoin_cbcount = list_length;
	fifoin_cb = f98_load;
}

static TGP_FUNCTION( f99 )
{
	logerror("TGP f99 (%x)\n", pushpc);
	next_fn();
}

static TGP_FUNCTION( f100 )
{
	int i;
	logerror("TGP f100 get list (%x)\n", pushpc);
	for(i=0; i<list_length; i++)
		fifoout_push_f((mame_rand(machine) % 1000)/100.0);
	next_fn();
}

static TGP_FUNCTION( groundbox_set )
{
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	float f = fifoin_pop_f();
	float g = fifoin_pop_f();
	logerror("TGP groundbox_set %f, %f, %f, %f, %f, %f, %f (%x)\n", a, b, c, d, e, f, g, pushpc);
	tgp_vf_xmin = e;
	tgp_vf_xmax = d;
	tgp_vf_zmin = g;
	tgp_vf_zmax = f;
	tgp_vf_ygnd = b;
	tgp_vf_yflr = a;
	tgp_vf_yjmp = c;

	next_fn();
}

static TGP_FUNCTION( f102 )
{
	static int ccount = 0;
	float px, py, pz;
	float a = fifoin_pop_f();
	float b = fifoin_pop_f();
	float c = fifoin_pop_f();
	float d = fifoin_pop_f();
	float e = fifoin_pop_f();
	UINT32 f = fifoin_pop();
	UINT32 g = fifoin_pop();
	UINT32 h = fifoin_pop();

	ccount++;

	logerror("TGP f0 mve_calc %f, %f, %f, %f, %f, %d, %d, %d (%d) (%x)\n", a, b, c, d, e, f, g, h, ccount, pushpc);

	px = u2f(ram_data[ram_scanadr+0x16]);
	py = u2f(ram_data[ram_scanadr+0x17]);
	pz = u2f(ram_data[ram_scanadr+0x18]);

	//  memset(cmat, 0, sizeof(cmat));
	//  cmat[0] = 1.0;
	//  cmat[4] = 1.0;
	//  cmat[8] = 1.0;

	px = c;
	py = d;
	pz = e;

#if 1
	cmat[ 9] += cmat[0]*a+cmat[3]*b+cmat[6]*c;
	cmat[10] += cmat[1]*a+cmat[4]*b+cmat[7]*c;
	cmat[11] += cmat[2]*a+cmat[5]*b+cmat[8]*c;
#else
	cmat[ 9] += px;
	cmat[10] += py;
	cmat[11] += pz;
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

static TGP_FUNCTION( f103 )
{
    ram_scanadr = fifoin_pop() - 0x8000;
	logerror("TGP f0 mve_setadr 0x%x (%x)\n", ram_scanadr, pushpc);
	ram_get_i();
	next_fn();
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
	logerror("TGP FIFOIN write %08x (%x)\n", fifoin_pop(), pushpc);
	fifoin_cbcount = 1;
	fifoin_cb = dump;
}

static TGP_FUNCTION( function_get_vf )
{
	UINT32 f = fifoin_pop() >> 23;

	if(fifoout_rpos != fifoout_wpos) {
		int count = fifoout_wpos - fifoout_rpos;
		if(count < 0)
			count += FIFO_SIZE;
		logerror("TGP function called with sizeout = %d\n", count);
	}
	if(ARRAY_LENGTH(ftab_vf) > f && NULL != ftab_vf[f].cb) {
		fifoin_cbcount = ftab_vf[f].count;
		fifoin_cb = ftab_vf[f].cb;
		//      logerror("TGP function %d request, %d parameters\n", f, fifoin_cbcount);
		if(!fifoin_cbcount)
			fifoin_cb(machine);
	} else {
		logerror("TGP function %d unimplemented (%x)\n", f, pushpc);
		fifoin_cbcount = 1;
		fifoin_cb = dump;
	}
}

static TGP_FUNCTION( function_get_swa )
{
	UINT32 f = fifoin_pop();

	if(fifoout_rpos != fifoout_wpos) {
		int count = fifoout_wpos - fifoout_rpos;
		if(count < 0)
			count += FIFO_SIZE;
		logerror("TGP function called with sizeout = %d\n", count);
	}
	if(ARRAY_LENGTH(ftab_swa) > f && NULL != ftab_swa[f].cb) {
		fifoin_cbcount = ftab_swa[f].count;
		fifoin_cb = ftab_swa[f].cb;
		//      logerror("TGP function %d request, %d parameters\n", f, fifoin_cbcount);
		if(!fifoin_cbcount)
			fifoin_cb(machine);
	} else {
		logerror("TGP function %d unimplemented (%x)\n", f, pushpc);
		fifoin_cbcount = 1;
		fifoin_cb = dump;
	}
}

READ16_HANDLER( model1_tgp_copro_r )
{
	static UINT32 cur;
	if(!offset) {
		cur = fifoout_pop(space);
		return cur;
	} else
		return cur >> 16;
}

WRITE16_HANDLER( model1_tgp_copro_w )
{
	static UINT32 cur;
	if(offset) {
		cur = (cur & 0x0000ffff) | (data << 16);
		pushpc = cpu_get_pc(space->cpu);
		fifoin_push(space, cur);
	} else
		cur = (cur & 0xffff0000) | data;
}

READ16_HANDLER( model1_tgp_copro_adr_r )
{
	return ram_adr;
}

WRITE16_HANDLER( model1_tgp_copro_adr_w )
{
	COMBINE_DATA(&ram_adr);
}

READ16_HANDLER( model1_tgp_copro_ram_r )
{
	if(!offset) {
		logerror("TGP f0 ram read %04x, %08x (%f) (%x)\n", ram_adr, ram_data[ram_adr], u2f(ram_data[ram_adr]), cpu_get_pc(space->cpu));
		return ram_data[ram_adr];
	} else
		return ram_data[ram_adr++] >> 16;
}

WRITE16_HANDLER( model1_tgp_copro_ram_w )
{
	COMBINE_DATA(ram_latch+offset);
	if(offset) {
		UINT32 v = ram_latch[0]|(ram_latch[1]<<16);
		logerror("TGP f0 ram write %04x, %08x (%f) (%x)\n", ram_adr, v, u2f(v), cpu_get_pc(space->cpu));
		ram_data[ram_adr] = v;
		ram_adr++;
	}
}

MACHINE_START( model1 )
{
	ram_data = auto_alloc_array(machine, UINT32, 0x10000);

	state_save_register_global_pointer(machine, ram_data, 0x10000);
	state_save_register_global(machine, ram_adr);
	state_save_register_global(machine, ram_scanadr);
	state_save_register_global_array(machine, ram_latch);
	state_save_register_global(machine, fifoout_rpos);
	state_save_register_global(machine, fifoout_wpos);
	state_save_register_global_array(machine, fifoout_data);
	state_save_register_global(machine, fifoin_rpos);
	state_save_register_global(machine, fifoin_wpos);
	state_save_register_global_array(machine, fifoin_data);
	state_save_register_global_array(machine, cmat);
	state_save_register_global_2d_array(machine, mat_stack);
	state_save_register_global_2d_array(machine, mat_vector);
	state_save_register_global(machine, mat_stack_pos);
	state_save_register_global(machine, acc);
	state_save_register_global(machine, list_length);
}

void model1_tgp_reset(running_machine *machine, int swa)
{
	ram_adr = 0;
	memset(ram_data, 0, 0x10000*4);

	fifoout_rpos = 0;
	fifoout_wpos = 0;
	fifoin_rpos = 0;
	fifoin_wpos = 0;

	acc = 0;
	mat_stack_pos = 0;
	memset(cmat, 0, sizeof(cmat));
	cmat[0] = 1.0;
	cmat[4] = 1.0;
	cmat[8] = 1.0;

	model1_dump = 0;
	model1_swa = swa;
	next_fn();
}

/*********************************** Virtua Racing ***********************************/

static int copro_fifoout_rpos, copro_fifoout_wpos;
static UINT32 copro_fifoout_data[FIFO_SIZE];
static int copro_fifoout_num;
static int copro_fifoin_rpos, copro_fifoin_wpos;
static UINT32 copro_fifoin_data[FIFO_SIZE];
static int copro_fifoin_num;

void model1_vr_tgp_reset( running_machine *machine )
{
	ram_adr = 0;
	memset(ram_data, 0, 0x8000*4);

	copro_fifoout_rpos = 0;
	copro_fifoout_wpos = 0;
	copro_fifoout_num = 0;
	copro_fifoin_rpos = 0;
	copro_fifoin_wpos = 0;
	copro_fifoin_num = 0;
}

/* FIFO */
static int copro_fifoin_pop(running_device *device, UINT32 *result)
{
	UINT32 r;

	if (copro_fifoin_num == 0)
	{
		return 0;
	}

	r = copro_fifoin_data[copro_fifoin_rpos++];

	if (copro_fifoin_rpos == FIFO_SIZE)
	{
		copro_fifoin_rpos = 0;
	}

	copro_fifoin_num--;

	*result = r;

	return 1;
}

static void copro_fifoin_push(address_space *space, UINT32 data)
{
	if (copro_fifoin_num == FIFO_SIZE)
	{
		fatalerror("Copro FIFOIN overflow (at %08X)", cpu_get_pc(space->cpu));
		return;
	}

	copro_fifoin_data[copro_fifoin_wpos++] = data;

	if (copro_fifoin_wpos == FIFO_SIZE)
	{
		copro_fifoin_wpos = 0;
	}

	copro_fifoin_num++;
}

static UINT32 copro_fifoout_pop(address_space *space)
{
	UINT32 r;

	if (copro_fifoout_num == 0)
	{
		// Reading from empty FIFO causes the v60 to enter wait state
		v60_stall(space->machine->device("maincpu"));

		timer_call_after_resynch(space->machine, NULL, 0, NULL);

		return 0;
	}

	r = copro_fifoout_data[copro_fifoout_rpos++];

	if (copro_fifoout_rpos == FIFO_SIZE)
	{
		copro_fifoout_rpos = 0;
	}

	copro_fifoout_num--;

	return r;
}

static void copro_fifoout_push(running_device *device, UINT32 data)
{
	if (copro_fifoout_num == FIFO_SIZE)
	{
		fatalerror("Copro FIFOOUT overflow (at %08X)", cpu_get_pc(device));
		return;
	}

	copro_fifoout_data[copro_fifoout_wpos++] = data;

	if (copro_fifoout_wpos == FIFO_SIZE)
	{
		copro_fifoout_wpos = 0;
	}

	copro_fifoout_num++;
}

static READ32_HANDLER(copro_ram_r)
{
	return ram_data[offset & 0x7fff];
}

static WRITE32_HANDLER(copro_ram_w)
{
	ram_data[offset&0x7fff] = data;
}

READ16_HANDLER( model1_tgp_vr_adr_r )
{
	if ( ram_adr == 0 && copro_fifoin_num != 0 )
	{
		/* spin the main cpu and let the TGP catch up */
		cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(100));
	}

	return ram_adr;
}

WRITE16_HANDLER( model1_tgp_vr_adr_w )
{
	COMBINE_DATA(&ram_adr);
}

READ16_HANDLER( model1_vr_tgp_ram_r )
{
	UINT16	r;

	if (!offset)
	{
		r = ram_data[ram_adr&0x7fff];
	}
	else
	{
		r = ram_data[ram_adr&0x7fff] >> 16;

		if ( ram_adr == 0 && r == 0xffff )
		{
			/* if the TGP is busy, spin some more */
			cpu_spinuntil_time(space->cpu, ATTOTIME_IN_USEC(100));
		}

		if ( ram_adr & 0x8000 )
			ram_adr++;
	}

	return r;
}

WRITE16_HANDLER( model1_vr_tgp_ram_w )
{
	COMBINE_DATA(ram_latch+offset);

	if (offset)
	{
		UINT32 v = ram_latch[0]|(ram_latch[1]<<16);
		ram_data[ram_adr&0x7fff] = v;
		if ( ram_adr & 0x8000 )
			ram_adr++;
	}
}

READ16_HANDLER( model1_vr_tgp_r )
{
	static UINT32 cur;

	if (!offset)
	{
		cur = copro_fifoout_pop(space);
		return cur;
	}
	else
		return cur >> 16;
}

WRITE16_HANDLER( model1_vr_tgp_w )
{
	static UINT32 cur;

	if (offset)
	{
		cur = (cur & 0x0000ffff) | (data << 16);
		copro_fifoin_push(space, cur);
	}
	else
		cur = (cur & 0xffff0000) | data;
}

/* TGP config */
const mb86233_cpu_core model1_vr_tgp_config =
{
	copro_fifoin_pop,
	copro_fifoout_push,
	"user5"
};

/* TGP memory map */
ADDRESS_MAP_START( model1_vr_tgp_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x000007ff) AM_RAM AM_REGION("tgp", 0)
	AM_RANGE(0x00400000, 0x00407fff) AM_READWRITE(copro_ram_r, copro_ram_w)
	AM_RANGE(0xff800000, 0xff87ffff) AM_ROM AM_REGION("user2", 0)
ADDRESS_MAP_END
