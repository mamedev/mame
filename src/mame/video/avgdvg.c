/*************************************************************************

    avgdvg.c: Atari DVG and AVG

    Some parts of this code are based on the original version by Eric
    Smith, Brad Oliver, Bernd Wiebelt, Aaron Giles, Andrew Caldwell

    The schematics and Jed Margolin's article on Vector Generators were
    very helpful in understanding the hardware.


**************************************************************************/

#include "emu.h"
#include "avgdvg.h"
#include "video/vector.h"


/*************************************
 *
 *  Global variables
 *
 *************************************/

static UINT8 *avgdvg_vectorram;
static size_t avgdvg_vectorram_size;

static UINT8 *avgdvg_colorram;



/*************************************
 *
 *  Macros and defines
 *
 *************************************/

#define MASTER_CLOCK (12096000)
#define VGSLICE      (10000)
#define MAXVECT      (10000)

#define VGVECTOR 0
#define VGCLIP 1

#define OP0 (vg->op & 1)
#define OP1 (vg->op & 2)
#define OP2 (vg->op & 4)
#define OP3 (vg->op & 8)

#define ST3 (vg->state_latch & 8)


/*************************************
 *
 *  Typedefs
 *
 *************************************/

struct vgvector
{
	int x; int y;
	rgb_t color;
	int intensity;
	int arg1; int arg2;
	int status;
};

struct vgdata
{
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	void set_machine(running_machine &machine) { m_machine = &machine; }

	UINT16 pc;
	UINT8 sp;
	UINT16 dvx;
	UINT16 dvy;
	UINT8 dvy12;
	UINT16 timer;
	UINT16 stack[4];
	UINT16 data;

	UINT8 state_latch;
	UINT8 int_latch;
	UINT8 scale;
	UINT8 bin_scale;
	UINT8 intensity;
	UINT8 color;
	UINT8 enspkl;
	UINT8 spkl_shift;
	UINT8 map;

	UINT16 hst;
	UINT16 lst;
	UINT16 izblank;

	UINT8 op;
	UINT8 halt;
	UINT8 sync_halt;

	UINT16 xdac_xor;
	UINT16 ydac_xor;

	INT32 xpos;
	INT32 ypos;

	INT32 clipx_min;
	INT32 clipy_min;
	INT32 clipx_max;
	INT32 clipy_max;

private:
	running_machine *m_machine;
};

struct vgconf
{
	int (*handler[8])(vgdata *);
	UINT8 (*state_addr)(vgdata *);
	void (*update_databus)(vgdata *);
	void (*vggo)(vgdata *);
	void (*vgrst)(vgdata *);
};


/*************************************
 *
 *  Static variables
 *
 *************************************/

static int xmin, xmax, ymin, ymax;
static int xcenter, ycenter;
static emu_timer *vg_run_timer, *vg_halt_timer;

static int flip_x, flip_y;

static vgdata vgd, *vg;
static const vgconf *vgc;
static int nvect;
static vgvector vectbuf[MAXVECT];


/*************************************
 *
 *  Flipping
 *
 *************************************/

void avg_set_flip_x(int flip)
{
	flip_x = flip;
}

void avg_set_flip_y(int flip)
{
	flip_y = flip;
}

static void avg_apply_flipping(int *x, int *y)
{
	if (flip_x)
		*x += (xcenter - *x) << 1;
	if (flip_y)
		*y += (ycenter - *y) << 1;
}


/*************************************
 *
 *  Vector buffering
 *
 *************************************/

static void vg_flush (running_machine &machine)
{
	int i = 0;

	while (vectbuf[i].status == VGCLIP)
		i++;
	vector_add_point(machine, vectbuf[i].x, vectbuf[i].y, vectbuf[i].color, 0);

	for (i = 0; i < nvect; i++)
	{
		if (vectbuf[i].status == VGVECTOR)
			vector_add_point(machine, vectbuf[i].x, vectbuf[i].y, vectbuf[i].color, vectbuf[i].intensity);

		if (vectbuf[i].status == VGCLIP)
			vector_add_clip(vectbuf[i].x, vectbuf[i].y, vectbuf[i].arg1, vectbuf[i].arg2);
	}

	nvect=0;
}

static void vg_add_point_buf(int x, int y, rgb_t color, int intensity)
{
	if (nvect < MAXVECT)
	{
		vectbuf[nvect].status = VGVECTOR;
		vectbuf[nvect].x = x;
		vectbuf[nvect].y = y;
		vectbuf[nvect].color = color;
		vectbuf[nvect].intensity = intensity;
		nvect++;
	}
}

static void vg_add_clip (int xmin, int ymin, int xmax, int ymax)
{
	if (nvect < MAXVECT)
	{
		vectbuf[nvect].status = VGCLIP;
		vectbuf[nvect].x = xmin;
		vectbuf[nvect].y = ymin;
		vectbuf[nvect].arg1 = xmax;
		vectbuf[nvect].arg2 = ymax;
		nvect++;
	}
}


/*************************************
 *
 *  DVG handler functions
 *
 *************************************/

static void dvg_data(vgdata *vg)
{
	/*
     * DVG uses low bit of state for address
     */
	vg->data = avgdvg_vectorram[(vg->pc << 1) | (vg->state_latch & 1)];
}

static UINT8 dvg_state_addr(vgdata *vg)
{
	UINT8 addr;

	addr =((((vg->state_latch >> 4) ^ 1) & 1) << 7) | (vg->state_latch & 0xf);

	if (OP3)
		addr |= ((vg->op & 7) << 4);

	return addr;
}

static int dvg_dmapush(vgdata *vg)
{
	if (OP0 == 0)
	{
		vg->sp = (vg->sp + 1) & 0xf;
		vg->stack[vg->sp & 3] = vg->pc;
	}
	return 0;
}

static int dvg_dmald(vgdata *vg)
{
	if (OP0)
	{
		vg->pc = vg->stack[vg->sp & 3];
		vg->sp = (vg->sp - 1) & 0xf;
	}
	else
	{
		vg->pc = vg->dvy;
	}

	return 0;
}

static void dvg_draw_to(int x, int y, int intensity)
{
	if (((x | y) & 0x400) == 0)
		vg_add_point_buf((xmin + x - 512) << 16,
						 (ymin + 512 - y) << 16,
						 VECTOR_COLOR111(7), intensity << 4);
}

static int dvg_gostrobe(vgdata *vg)
{
	int scale, fin, dx, dy, c, mx, my, countx, county, bit, cycles;

	if (vg->op == 0xf)
	{

		scale = (vg->scale +
				 (((vg->dvy & 0x800) >> 11)
				  | (((vg->dvx & 0x800) ^ 0x800) >> 10)
				  | ((vg->dvx & 0x800)  >> 9))) & 0xf;

		vg->dvy &= 0xf00;
		vg->dvx &= 0xf00;
	}
	else
	{
		scale = (vg->scale + vg->op) & 0xf;
	}

	fin = 0xfff - (((2 << scale) & 0x7ff) ^ 0xfff);

	/* Count up or down */
	dx = (vg->dvx & 0x400)? -1: +1;
	dy = (vg->dvy & 0x400)? -1: +1;

	/* Scale factor for rate multipliers */
	mx = (vg->dvx << 2) & 0xfff;
	my = (vg->dvy << 2) & 0xfff;

	cycles = 8 * fin;
	c=0;

	while (fin--)
	{

		/*
         *  The 7497 Bit Rate Multiplier is a 6 bit counter with
         *  clever decoding of output bits to perform the following
         *  operation:
         *
         *  fout = m/64 * fin
         *
         *  where fin is the input frequency, fout is the output
         *  frequency and m is a factor at the input pins. Output
         *  pulses are more or less evenly spaced so we get straight
         *  lines. The DVG has two cascaded 7497s for each coordinate.
         */

		countx = 0;
		county = 0;

		for (bit = 0; bit < 12; bit++)
		{
			if ((c & ((1 << (bit+1)) - 1)) == ((1 << bit) - 1))
			{
				if (mx & (1 << (11 - bit)))
					countx = 1;

				if (my & (1 << (11 - bit)))
					county = 1;
			}
		}

		c = (c + 1) & 0xfff;

		/*
         *  Since x- and y-counters always hold the correct count
         *  wrt. to each other, we can do clipping exactly like the
         *  hardware does. That is, as soon as any counter's bit 10
         *  changes to high, we finish the vector. If bit 10 changes
         *  from high to low, we start a new vector.
         */

		if (countx)
		{
			/* Is y valid and x entering or leaving the valid range? */
			if (((vg->ypos & 0x400) == 0)
				&& ((vg->xpos ^ (vg->xpos + dx)) & 0x400))
			{
				if ((vg->xpos + dx) & 0x400)
					/* We are leaving the valid range */
					dvg_draw_to(vg->xpos, vg->ypos, vg->intensity);
				else
					/* We are entering the valid range */
					dvg_draw_to((vg->xpos + dx) & 0xfff, vg->ypos, 0);
			}
			vg->xpos = (vg->xpos + dx) & 0xfff;
		}

		if (county)
		{
			if (((vg->xpos & 0x400) == 0)
				&& ((vg->ypos ^ (vg->ypos + dy)) & 0x400))
			{
				if ((vg->xpos & 0x400) == 0)
				{
					if ((vg->ypos + dy) & 0x400)
						dvg_draw_to(vg->xpos, vg->ypos, vg->intensity);
					else
						dvg_draw_to(vg->xpos, (vg->ypos + dy) & 0xfff, 0);
				}
			}
			vg->ypos = (vg->ypos + dy) & 0xfff;
		}

	}

	dvg_draw_to(vg->xpos, vg->ypos, vg->intensity);

	return cycles;
}

static int dvg_haltstrobe(vgdata *vg)
{
	vg->halt = OP0;

	if (OP0 == 0)
	{
		vg->xpos = vg->dvx & 0xfff;
		vg->ypos = vg->dvy & 0xfff;
		dvg_draw_to(vg->xpos, vg->ypos, 0);
	}
	return 0;
}

static int dvg_latch3(vgdata *vg)
{
	vg->dvx = (vg->dvx & 0xff) |  ((vg->data & 0xf) << 8);
	vg->intensity = vg->data >> 4;
	return 0;
}

static int dvg_latch2(vgdata *vg)
{
	vg->dvx &= 0xf00;
	if (vg->op != 0xf)
		vg->dvx = (vg->dvx & 0xf00) | vg->data;

	if ((vg->op & 0xa) == 0xa)
		vg->scale = vg->intensity;

	vg->pc++;
	return 0;
}

static int dvg_latch1(vgdata *vg)
{
	vg->dvy = (vg->dvy & 0xff)
		| ((vg->data & 0xf) << 8);
	vg->op = vg->data >> 4;

	if (vg->op == 0xf)
	{
		vg->dvx &= 0xf00;
		vg->dvy &= 0xf00;
	}

	return 0;
}

static int dvg_latch0(vgdata *vg)
{
	vg->dvy &= 0xf00;
	if (vg->op == 0xf)
		dvg_latch3(vg);
	else
		vg->dvy = (vg->dvy & 0xf00) | vg->data;

	vg->pc++;
	return 0;
}


/********************************************************************
 *
 *  AVG handler functions
 *
 *  AVG is in many ways different from DVG. The only thing they have
 *  in common is the state machine approach. There are small
 *  differences among the AVGs, mostly related to color and vector
 *  clipping.
 *
 *******************************************************************/

static void avg_data(vgdata *vg)
{
	vg->data = avgdvg_vectorram[vg->pc ^ 1];
}

static void starwars_data(vgdata *vg)
{
	vg->data = avgdvg_vectorram[vg->pc];
}

static void quantum_data(vgdata *vg)
{
	vg->data = ((UINT16 *)avgdvg_vectorram)[vg->pc >> 1];
}

static void mhavoc_data(vgdata *vg)
{
	UINT8 *bank;

	if (vg->pc & 0x2000)
	{
		bank = &vg->machine().root_device().memregion("alpha")->base()[0x18000];
		vg->data = bank[(vg->map << 13) | ((vg->pc ^ 1) & 0x1fff)];
	}
	else
	{
		vg->data = avgdvg_vectorram[vg->pc ^ 1];
	}
}

static UINT8 avg_state_addr(vgdata *vg)
{
	return (((vg->state_latch >> 4) ^ 1) << 7)
		| (vg->op << 4)
		| (vg->state_latch & 0xf);
}

static int avg_latch0(vgdata *vg)
{
	vg->dvy = (vg->dvy & 0x1f00) | vg->data;
	vg->pc++;

	return 0;
}

static int quantum_st2st3(vgdata *vg)
{
	/* Quantum doesn't decode latch0 or latch2 but ST2 and ST3 are fed
     * into the address controller which increments the PC
     */
	vg->pc++;
	return 0;
}

static int avg_latch1(vgdata *vg)
{
	vg->dvy12 = (vg->data >> 4) &1;
	vg->op = vg->data >> 5;

	vg->int_latch = 0;
	vg->dvy = (vg->dvy12 << 12)
		| ((vg->data & 0xf) << 8 );
	vg->dvx = 0;
	vg->pc++;

	return 0;
}

static int quantum_latch1(vgdata *vg)
{
	vg->dvy = vg->data & 0x1fff;
	vg->dvy12 = (vg->data >> 12) & 1;
	vg->op = vg->data >> 13;

	vg->int_latch = 0;
	vg->dvx = 0;
	vg->pc++;

	return 0;
}

static int bzone_latch1(vgdata *vg)
{
	/*
     * Battle Zone has clipping hardware. We need to remember the
     * position of the beam when the analog switches hst or lst get
     * turned off.
     */

	if (vg->hst == 0)
	{
		vg->clipx_max = vg->xpos;
		vg->clipy_min = vg->ypos;
	}

	if (vg->lst == 0)
	{
		vg->clipx_min = vg->xpos;
		vg->clipy_max = vg->ypos;
	}

	if (vg->lst==0 || vg->hst==0)
	{
		vg_add_clip(vg->clipx_min, vg->clipy_min, vg->clipx_max, vg->clipy_max);
	}
	vg->lst = vg->hst = 1;

	return avg_latch1(vg);
}

static int mhavoc_latch1(vgdata *vg)
{
	/*
     * Major Havoc just has ymin clipping
     */

	if (vg->lst == 0)
	{
		vg_add_clip(0, vg->ypos, xmax << 16, ymax << 16);
	}
	vg->lst = 1;

	return avg_latch1(vg);
}

static int avg_latch2(vgdata *vg)
{
	vg->dvx = (vg->dvx & 0x1f00) | vg->data;
	vg->pc++;

	return 0;
}

static int avg_latch3(vgdata *vg)
{
	vg->int_latch = vg->data >> 4;
	vg->dvx = ((vg->int_latch & 1) << 12)
		| ((vg->data & 0xf) << 8 )
		| (vg->dvx & 0xff);
	vg->pc++;

	return 0;
}

static int quantum_latch3(vgdata *vg)
{
	vg->int_latch = vg->data >> 12;
	vg->dvx = vg->data & 0xfff;
	vg->pc++;

	return 0;
}


static int avg_strobe0(vgdata *vg)
{
	int i;

	if (OP0)
	{
		vg->stack[vg->sp & 3] = vg->pc;
	}
	else
	{
		/*
         * Normalization is done to get roughly constant deflection
         * speeds. See Jed's essay why this is important. In addition
         * to the intensity and overall time saving issues it is also
         * needed to avoid accumulation of DAC errors. The X/Y DACs
         * only use bits 3-12. The normalization ensures that the
         * first three bits hold no important information.
         *
         * The circuit doesn't check for dvx=dvy=0. In this case
         * shifting goes on as long as VCTR, SCALE and CNTR are
         * low. We cut off after 16 shifts.
         */
		i = 0;
		while ((((vg->dvy ^ (vg->dvy << 1)) & 0x1000) == 0)
			   && (((vg->dvx ^ (vg->dvx << 1)) & 0x1000) == 0)
			   && (i++ < 16))
		{
			vg->dvy = (vg->dvy & 0x1000) | ((vg->dvy << 1) & 0x1fff);
			vg->dvx = (vg->dvx & 0x1000) | ((vg->dvx << 1) & 0x1fff);
			vg->timer >>= 1;
			vg->timer |= 0x4000 | (OP1 << 6);
		}

		if (OP1)
			vg->timer &= 0xff;
	}

	return 0;
}

static int quantum_strobe0(vgdata *vg)
{
	int i;

	if (OP0)
	{
		vg->stack[vg->sp & 3] = vg->pc;
	}
	else
	{
		/*
         * Quantum normalizes to 12 bit
         */
		i = 0;
		while ((((vg->dvy ^ (vg->dvy << 1)) & 0x800) == 0)
			   && (((vg->dvx ^ (vg->dvx << 1)) & 0x800) == 0)
			   && (i++ < 16))
		{
			vg->dvy = (vg->dvy << 1) & 0xfff;
			vg->dvx = (vg->dvx << 1) & 0xfff;
			vg->timer >>= 1;
			vg->timer |= 0x2000 ;
		}
	}

	return 0;
}

static int avg_common_strobe1(vgdata *vg)
{
	if (OP2)
	{
		if (OP1)
			vg->sp = (vg->sp - 1) & 0xf;
		else
			vg->sp = (vg->sp + 1) & 0xf;
	}
	return 0;
}

static int avg_strobe1(vgdata *vg)
{
	int i;

	if (OP2 == 0)
	{
		for (i = vg->bin_scale; i > 0; i--)
		{
			vg->timer >>= 1;
			vg->timer |= 0x4000 | (OP1 << 6);
		}
		if (OP1)
			vg->timer &= 0xff;
	}

	return avg_common_strobe1(vg);
}

static int quantum_strobe1(vgdata *vg)
{
	int i;

	if (OP2 == 0)
	{
		for (i = vg->bin_scale; i > 0; i--)
		{
			vg->timer >>= 1;
			vg->timer |= 0x2000;
		}
	}

	return avg_common_strobe1(vg);
}

static int avg_common_strobe2(vgdata *vg)
{
	if (OP2)
	{
		if (OP0)
		{
			vg->pc = vg->dvy << 1;

			if (vg->dvy == 0)
			{
				/*
                 * Tempest and Quantum keep the AVG in an endless
                 * loop. I.e. at one point the AVG jumps to address 0
                 * and starts over again. The main CPU updates vector
                 * RAM while AVG is running. The hardware takes care
                 * that the AVG dosen't read vector RAM while the CPU
                 * writes to it. Usually we wait until the AVG stops
                 * (halt flag) and then draw all vectors at once. This
                 * doesn't work for Tempest and Quantum so we wait for
                 * the jump to zero and draw vectors then.
                 *
                 * Note that this has nothing to do with the real hardware
                 * because for a vector monitor it is perfectly okay to
                 * have the AVG drawing all the time. In the emulation we
                 * somehow have to divide the stream of vectors into
                 * 'frames'.
                 */

				vector_clear_list();
				vg_flush(vg->machine());
			}
		}
		else
		{
			vg->pc = vg->stack[vg->sp & 3];
		}
	}
	else
	{
		if (vg->dvy12)
		{
			vg->scale = vg->dvy & 0xff;
			vg->bin_scale = (vg->dvy >> 8) & 7;
		}
	}

	return 0;
}

static int avg_strobe2(vgdata *vg)
{
	if ((OP2 == 0) && (vg->dvy12 == 0))
	{
		vg->color = vg->dvy & 0x7;
		vg->intensity = (vg->dvy >> 4) & 0xf;
	}

	return  avg_common_strobe2(vg);
}

static int mhavoc_strobe2(vgdata *vg)
{
	if (OP2 == 0)
	{
		if (vg->dvy12)
		{
			if (vg->dvy & 0x800)
				vg->lst = 0;
		}
		else
		{
			vg->color = vg->dvy & 0xf;

			vg->intensity = (vg->dvy >> 4) & 0xf;
			vg->map = (vg->dvy >> 8) & 0x3;
			if (vg->dvy & 0x800)
			{
				vg->enspkl = 1;
				vg->spkl_shift = ((vg->dvy >> 3) & 1)
					| ((vg->dvy >> 1) & 2)
					| ((vg->dvy << 1) & 4)
					| ((vg->dvy << 2) & 8)
					| ((vg->machine().rand() & 0x7) << 4);
			}
			else
			{
				vg->enspkl = 0;
			}

			/* Major Havoc can do X-flipping by inverting the DAC input */
			if (vg->dvy & 0x400)
				vg->xdac_xor = 0x1ff;
			else
				vg->xdac_xor = 0x200;
		}
	}

	return  avg_common_strobe2(vg);
}

static int tempest_strobe2(vgdata *vg)
{
	if ((OP2 == 0) && (vg->dvy12 == 0))
	{
		/* Contrary to previous documentation in MAME,
        Tempest does not have the vg->enspkl bit. */
		if (vg->dvy & 0x800)
			vg->color = vg->dvy & 0xf;
		else
			vg->intensity = (vg->dvy >> 4) & 0xf;
	}

	return  avg_common_strobe2(vg);
}

static int quantum_strobe2(vgdata *vg)
{
	if ((OP2 == 0) && (vg->dvy12 == 0) && (vg->dvy & 0x800))
	{
		vg->color = vg->dvy & 0xf;
		vg->intensity = (vg->dvy >> 4) & 0xf;
	}

	return  avg_common_strobe2(vg);
}

static int starwars_strobe2(vgdata *vg)
{
	if ((OP2 == 0) && (vg->dvy12 == 0))
	{
		vg->intensity = vg->dvy & 0xff;
		vg->color = (vg->dvy >> 8) & 0xf;
	}

	return  avg_common_strobe2(vg);
}

static int bzone_strobe2(vgdata *vg)
{
	if ((OP2 == 0) && (vg->dvy12 == 0))
	{
		vg->intensity = (vg->dvy >> 4) & 0xf;

		if (!(vg->dvy & 0x400))
		{
			vg->lst = vg->dvy & 0x200;
			vg->hst = vg->lst ^ 0x200;
			/*
             * If izblank is true the zblank signal gets
             * inverted. This behaviour can't be handled with the
             * clipping we have right now. Battle Zone doesn't seem to
             * invert zblank so it's no issue.
             */
			vg->izblank = vg->dvy & 0x100;
		}
	}
	return avg_common_strobe2(vg);
}

static int avg_common_strobe3(vgdata *vg)
{
	int cycles=0;

	vg->halt = OP0;

	if ((vg->op & 5) == 0)
	{
		if (OP1)
		{
			cycles = 0x100 - (vg->timer & 0xff);
		}
		else
		{
			cycles = 0x8000 - vg->timer;
		}
		vg->timer = 0;

		vg->xpos += ((((vg->dvx >> 3) ^ vg->xdac_xor) - 0x200) * cycles * (vg->scale ^ 0xff)) >> 4;
		vg->ypos -= ((((vg->dvy >> 3) ^ vg->ydac_xor) - 0x200) * cycles * (vg->scale ^ 0xff)) >> 4;
	}
	if (OP2)
	{
		cycles = 0x8000 - vg->timer;
		vg->timer = 0;
		vg->xpos = xcenter;
		vg->ypos = ycenter;
		vg_add_point_buf(vg->xpos, vg->ypos, 0, 0);
	}

	return cycles;
}

static int avg_strobe3(vgdata *vg)
{
	int cycles;

	cycles = avg_common_strobe3(vg);

	if ((vg->op & 5) == 0)
	{
		vg_add_point_buf(vg->xpos, vg->ypos, VECTOR_COLOR111(vg->color),
						 (((vg->int_latch >> 1) == 1)? vg->intensity: vg->int_latch & 0xe) << 4);
	}

	return cycles;
}

static int bzone_strobe3(vgdata *vg)
{
	/* Battle Zone is B/W */
	int cycles;

	cycles = avg_common_strobe3(vg);

	if ((vg->op & 5) == 0)
	{
		vg_add_point_buf(vg->xpos, vg->ypos, VECTOR_COLOR111(7),
						 (((vg->int_latch >> 1) == 1)? vg->intensity: vg->int_latch & 0xe) << 4);
	}

	return cycles;
}

static int tempest_strobe3(vgdata *vg)
{
	int cycles, r, g, b, bit0, bit1, bit2, bit3, x, y;
	UINT8 data;

	cycles = avg_common_strobe3(vg);

	if ((vg->op & 5) == 0)
	{
		data = avgdvg_colorram[vg->color];
		bit3 = (~data >> 3) & 1;
		bit2 = (~data >> 2) & 1;
		bit1 = (~data >> 1) & 1;
		bit0 = (~data >> 0) & 1;

		r = bit1 * 0xf3 + bit0 * 0x0c;
		g = bit3 * 0xf3;
		b = bit2 * 0xf3;

		x = vg->xpos;
		y = vg->ypos;

		avg_apply_flipping(&x, &y);

		vg_add_point_buf(y - ycenter + xcenter,
						 x - xcenter + ycenter, MAKE_RGB(r, g, b),
						 (((vg->int_latch >> 1) == 1)? vg->intensity: vg->int_latch & 0xe) << 4);
	}

	return cycles;
}

static int mhavoc_strobe3(vgdata *vg)
{
	int cycles, r, g, b, bit0, bit1, bit2, bit3, dx, dy, i;

	UINT8 data;

	vg->halt = OP0;
	cycles = 0;

	if ((vg->op & 5) == 0)
	{
		if (OP1)
		{
			cycles = 0x100 - (vg->timer & 0xff);
		}
		else
		{
			cycles = 0x8000 - vg->timer;
		}
		vg->timer = 0;
		dx = ((((vg->dvx >> 3) ^ vg->xdac_xor) - 0x200) * (vg->scale ^ 0xff));
		dy = ((((vg->dvy >> 3) ^ vg->ydac_xor) - 0x200) * (vg->scale ^ 0xff));


		if (vg->enspkl)
		{
			for (i=0; i<cycles/8; i++)
			{
				vg->xpos += dx/2;
				vg->ypos -= dy/2;
				data = avgdvg_colorram[0xf +
									   (((vg->spkl_shift & 1) << 3)
										| (vg->spkl_shift & 4)
										| ((vg->spkl_shift & 0x10) >> 3)
										| ((vg->spkl_shift & 0x40) >> 6))];
				bit3 = (~data >> 3) & 1;
				bit2 = (~data >> 2) & 1;
				bit1 = (~data >> 1) & 1;
				bit0 = (~data >> 0) & 1;
				r = bit3 * 0xcb + bit2 * 0x34;
				g = bit1 * 0xcb;
				b = bit0 * 0xcb;

				vg_add_point_buf(vg->xpos, vg->ypos, MAKE_RGB(r, g, b),
								 (((vg->int_latch >> 1) == 1)? vg->intensity: vg->int_latch & 0xe) << 4);
				vg->spkl_shift = (((vg->spkl_shift & 0x40) >> 6)
								  ^ ((vg->spkl_shift & 0x20) >> 5)
								  ^ 1 )
					| (vg->spkl_shift << 1);

				if ((vg->spkl_shift & 0x7f) == 0x7f)
					vg->spkl_shift = 0;
			}
		}
		else
		{
			vg->xpos += (dx * cycles) >> 4;
			vg->ypos -= (dy * cycles) >> 4;
			data = avgdvg_colorram[vg->color];

			bit3 = (~data >> 3) & 1;
			bit2 = (~data >> 2) & 1;
			bit1 = (~data >> 1) & 1;
			bit0 = (~data >> 0) & 1;
			r = bit3 * 0xcb + bit2 * 0x34;
			g = bit1 * 0xcb;
			b = bit0 * 0xcb;

			vg_add_point_buf(vg->xpos, vg->ypos, MAKE_RGB(r, g, b),
							 (((vg->int_latch >> 1) == 1)? vg->intensity: vg->int_latch & 0xe) << 4);
		}
	}
	if (OP2)
	{
		cycles = 0x8000 - vg->timer;
		vg->timer = 0;
		vg->xpos = xcenter;
		vg->ypos = ycenter;
		vg_add_point_buf(vg->xpos, vg->ypos, 0, 0);
	}

	return cycles;
}

static int starwars_strobe3(vgdata *vg)
{
	int cycles;

	cycles = avg_common_strobe3(vg);

	if ((vg->op & 5) == 0)
	{
		vg_add_point_buf(vg->xpos, vg->ypos, VECTOR_COLOR111(vg->color),
						 ((vg->int_latch >> 1) * vg->intensity) >> 3);
	}

	return cycles;
}

static int quantum_strobe3(vgdata *vg)
{
	int cycles=0, r, g, b, bit0, bit1, bit2, bit3, x, y;

	UINT16 data;

	vg->halt = OP0;

	if ((vg->op & 5) == 0)
	{
		data = ((UINT16 *)avgdvg_colorram)[vg->color];
		bit3 = (~data >> 3) & 1;
		bit2 = (~data >> 2) & 1;
		bit1 = (~data >> 1) & 1;
		bit0 = (~data >> 0) & 1;

		g = bit1 * 0xaa + bit0 * 0x54;
		b = bit2 * 0xce;
		r = bit3 * 0xce;

		cycles = 0x4000 - vg->timer;
		vg->timer = 0;

		vg->xpos += (((((vg->dvx & 0xfff) >> 2) ^ vg->xdac_xor) - 0x200) * cycles * (vg->scale ^ 0xff)) >> 4;
		vg->ypos -= (((((vg->dvy & 0xfff) >> 2) ^ vg->ydac_xor) - 0x200) * cycles * (vg->scale ^ 0xff)) >> 4;

		x = vg->xpos;
		y = vg->ypos;

		avg_apply_flipping(&x, &y);

		vg_add_point_buf(y - ycenter + xcenter,
						 x - xcenter + ycenter, MAKE_RGB(r, g, b),
						 ((vg->int_latch == 2)? vg->intensity: vg->int_latch) << 4);
	}
	if (OP2)
	{
		cycles = 0x4000 - vg->timer;
		vg->timer = 0;
		vg->xpos = xcenter;
		vg->ypos = ycenter;
		vg_add_point_buf(vg->xpos, vg->ypos, 0, 0);
	}

	return cycles;
}

static void avg_vggo(vgdata *vg)
{
	vg->pc = 0;
	vg->sp = 0;
}

static void tempest_vggo(vgdata *vg)
{
	vg->pc = 0;
	vg->sp = 0;
	/*
     * Tempest and Quantum trigger VGGO from time to time even though
     * the VG runs in an endless loop for these games (see
     * avg_common_strobe2). If we don't discard all vectors in the
     * current buffer at this point, the screen starts flickering.
     */
	nvect = 0;
}

static void dvg_vggo(vgdata *vg)
{
	vg->dvy = 0;
	vg->op = 0;
}

static void avg_vgrst(vgdata *vg)
{
	vg->state_latch = 0;
	vg->bin_scale = 0;
	vg->scale = 0;
	vg->color = 0;
}

static void mhavoc_vgrst(vgdata *vg)
{
	avg_vgrst(vg);
	vg->enspkl = 0;
}

static void dvg_vgrst(vgdata *vg)
{
	vg->state_latch = 0;
	vg->dvy = 0;
	vg->op = 0;
}

static void vg_set_halt(int dummy)
{
	vg->halt = dummy;
	vg->sync_halt = dummy;
}

static TIMER_CALLBACK( vg_set_halt_callback )
{
	vg_set_halt(param);
}


/********************************************************************
 *
 * State Machine
 *
 * The state machine is a 256x4 bit PROM connected to a latch. The
 * address of the next state is generated from the latched previous
 * state, an op code and the halt flag. Op codes come from vector
 * RAM/ROM. The state machine is clocked with 1.5 MHz. Three bits of
 * the state are decoded and used to trigger various parts of the
 * hardware.
 *
 *******************************************************************/

static TIMER_CALLBACK( run_state_machine )
{
	int cycles = 0;
	UINT8 *state_prom = machine.root_device().memregion("user1")->base();

	while (cycles < VGSLICE)
	{
		/* Get next state */
		vg->state_latch = (vg->state_latch & 0x10)
			| (state_prom[vgc->state_addr(vg)] & 0xf);

		if (ST3)
		{
			/* Read vector RAM/ROM */
			vgc->update_databus(vg);

			/* Decode state and call the corresponding handler */
			cycles += (vgc->handler[vg->state_latch & 7])(vg);
		}

		/* If halt flag was set, let CPU catch up before we make halt visible */
		if (vg->halt && !(vg->state_latch & 0x10))
			vg_halt_timer->adjust(attotime::from_hz(MASTER_CLOCK) * cycles, 1);

		vg->state_latch = (vg->halt << 4) | (vg->state_latch & 0xf);
		cycles += 8;
	}

	vg_run_timer->adjust(attotime::from_hz(MASTER_CLOCK) * cycles);
}


/*************************************
 *
 *  VG halt/vggo
 *
 ************************************/

CUSTOM_INPUT( avgdvg_done_r )
{
	return vg->sync_halt ? 0x01 : 0x00;
}

WRITE8_HANDLER( avgdvg_go_w )
{
	vgc->vggo(vg);

	if (vg->sync_halt && (nvect > 10))
	{
		/*
         * This is a good time to start a new frame. Major Havoc
         * sometimes sets VGGO after a very short vector list. That's
         * why we ignore frames with less than 10 vectors.
         */
		vector_clear_list();
	}
	vg_flush(space.machine());

	vg_set_halt(0);
	vg_run_timer->adjust(attotime::zero);
}

WRITE16_HANDLER( avgdvg_go_word_w )
{
	avgdvg_go_w(space, offset, data);
}


/*************************************
 *
 *  Reset
 *
 ************************************/

WRITE8_HANDLER( avgdvg_reset_w )
{
	vgc->vgrst(vg);
	vg_set_halt(1);
}

WRITE16_HANDLER( avgdvg_reset_word_w )
{
	avgdvg_reset_w (space,0,0);
}

MACHINE_RESET( avgdvg )
{
	avgdvg_reset_w (machine.device("maincpu")->memory().space(AS_PROGRAM),0,0);
}


/*************************************
 *
 *  Configuration of VG variants
 *
 *************************************/

static const vgconf dvg_default =
{
	{
		dvg_dmapush,
		dvg_dmald,
		dvg_gostrobe,
		dvg_haltstrobe,
		dvg_latch0,
		dvg_latch1,
		dvg_latch2,
		dvg_latch3
	},
	dvg_state_addr,
	dvg_data,
	dvg_vggo,
	dvg_vgrst
};

static const vgconf avg_default =
{
	{
		avg_latch0,
		avg_latch1,
		avg_latch2,
		avg_latch3,
		avg_strobe0,
		avg_strobe1,
		avg_strobe2,
		avg_strobe3
	},
	avg_state_addr,
	avg_data,
	avg_vggo,
	avg_vgrst
};

static const vgconf avg_mhavoc =
{
	{
		avg_latch0,
		mhavoc_latch1,
		avg_latch2,
		avg_latch3,
		avg_strobe0,
		avg_strobe1,
		mhavoc_strobe2,
		mhavoc_strobe3
	},
	avg_state_addr,
	mhavoc_data,
	avg_vggo,
	mhavoc_vgrst
};

static const vgconf avg_starwars =
{
	{
		avg_latch0,
		avg_latch1,
		avg_latch2,
		avg_latch3,
		avg_strobe0,
		avg_strobe1,
		starwars_strobe2,
		starwars_strobe3
	},
	avg_state_addr,
	starwars_data,
	avg_vggo,
	avg_vgrst
};

static const vgconf avg_tempest =
{
	{
		avg_latch0,
		avg_latch1,
		avg_latch2,
		avg_latch3,
		avg_strobe0,
		avg_strobe1,
		tempest_strobe2,
		tempest_strobe3
	},
	avg_state_addr,
	avg_data,
	tempest_vggo,
	avg_vgrst
};

static const vgconf avg_bzone =
{
	{
		avg_latch0,
		bzone_latch1,
		avg_latch2,
		avg_latch3,
		avg_strobe0,
		avg_strobe1,
		bzone_strobe2,
		bzone_strobe3
	},
	avg_state_addr,
	avg_data,
	avg_vggo,
	avg_vgrst
};

static const vgconf avg_quantum =
{
	{
		quantum_st2st3,
		quantum_latch1,
		quantum_st2st3,
		quantum_latch3,
		quantum_strobe0,
		quantum_strobe1,
		quantum_strobe2,
		quantum_strobe3
	},
	avg_state_addr,
	quantum_data,
	tempest_vggo,
	avg_vgrst
};

static const vgconf avg_tomcat =
{
	{
		avg_latch0,
		avg_latch1,
		avg_latch2,
		avg_latch3,
		avg_strobe0,
		avg_strobe1,
		starwars_strobe2,
		starwars_strobe3
	},
	avg_state_addr,
	avg_data,
	avg_vggo,
	avg_vgrst
};

/*************************************
 *
 *  Vector generator init
 *
 ************************************/

static void register_state (running_machine &machine)
{
	state_save_register_item(machine, "AVG", NULL, 0, vg->pc);
	state_save_register_item(machine, "AVG", NULL, 0, vg->sp);
	state_save_register_item(machine, "AVG", NULL, 0, vg->dvx);
	state_save_register_item(machine, "AVG", NULL, 0, vg->dvy);
	state_save_register_item(machine, "AVG", NULL, 0, vg->dvy12);
	state_save_register_item(machine, "AVG", NULL, 0, vg->timer);
	state_save_register_item_array(machine, "AVG", NULL, 0, vg->stack);
	state_save_register_item(machine, "AVG", NULL, 0, vg->data);
	state_save_register_item(machine, "AVG", NULL, 0, vg->state_latch);
	state_save_register_item(machine, "AVG", NULL, 0, vg->int_latch);
	state_save_register_item(machine, "AVG", NULL, 0, vg->scale);
	state_save_register_item(machine, "AVG", NULL, 0, vg->bin_scale);
	state_save_register_item(machine, "AVG", NULL, 0, vg->intensity);
	state_save_register_item(machine, "AVG", NULL, 0, vg->color);
	state_save_register_item(machine, "AVG", NULL, 0, vg->enspkl);
	state_save_register_item(machine, "AVG", NULL, 0, vg->spkl_shift);
	state_save_register_item(machine, "AVG", NULL, 0, vg->map);
	state_save_register_item(machine, "AVG", NULL, 0, vg->hst);
	state_save_register_item(machine, "AVG", NULL, 0, vg->lst);
	state_save_register_item(machine, "AVG", NULL, 0, vg->izblank);
	state_save_register_item(machine, "AVG", NULL, 0, vg->op);
	state_save_register_item(machine, "AVG", NULL, 0, vg->halt);
	state_save_register_item(machine, "AVG", NULL, 0, vg->sync_halt);
	state_save_register_item(machine, "AVG", NULL, 0, vg->xdac_xor);
	state_save_register_item(machine, "AVG", NULL, 0, vg->ydac_xor);
	state_save_register_item(machine, "AVG", NULL, 0, vg->xpos);
	state_save_register_item(machine, "AVG", NULL, 0, vg->ypos);
	state_save_register_item(machine, "AVG", NULL, 0, vg->clipx_min);
	state_save_register_item(machine, "AVG", NULL, 0, vg->clipy_min);
	state_save_register_item(machine, "AVG", NULL, 0, vg->clipx_max);
	state_save_register_item(machine, "AVG", NULL, 0, vg->clipy_max);

	state_save_register_item(machine, "AVG", NULL, 0, flip_x);
	state_save_register_item(machine, "AVG", NULL, 0, flip_y);
	state_save_register_item_pointer(machine, "AVG", NULL, 0, avgdvg_vectorram, avgdvg_vectorram_size);
}

static VIDEO_START( avg_common )
{
	const rectangle &visarea = machine.primary_screen->visible_area();

	avgdvg_vectorram = reinterpret_cast<UINT8 *>(machine.root_device().memshare("vectorram")->ptr());
	avgdvg_vectorram_size = machine.root_device().memshare("vectorram")->bytes();

	avgdvg_colorram = reinterpret_cast<UINT8 *>(machine.root_device().memshare("colorram")->ptr());

	vg = &vgd;
	vg->set_machine(machine);

	xmin = visarea.min_x;
	ymin = visarea.min_y;
	xmax = visarea.max_x;
	ymax = visarea.max_y;

	xcenter = ((xmax - xmin) / 2) << 16;
	ycenter = ((ymax - ymin) / 2) << 16;

	flip_x = flip_y = 0;

	vg_halt_timer = machine.scheduler().timer_alloc(FUNC(vg_set_halt_callback));
	vg_run_timer = machine.scheduler().timer_alloc(FUNC(run_state_machine));

	/*
     * The x and y DACs use 10 bit of the counter values which are in
     * two's complement representation. The DAC input is xored with
     * 0x200 to convert the value to unsigned.
     */
	vg->xdac_xor = 0x200;
	vg->ydac_xor = 0x200;

	register_state (machine);
	VIDEO_START_CALL(vector);
}

VIDEO_START( dvg )
{
	const rectangle &visarea = machine.primary_screen->visible_area();

	avgdvg_vectorram = reinterpret_cast<UINT8 *>(machine.root_device().memshare("vectorram")->ptr());
	avgdvg_vectorram_size = machine.root_device().memshare("vectorram")->bytes();

	avgdvg_colorram = reinterpret_cast<UINT8 *>(machine.root_device().memshare("colorram")->ptr());

	vgc = &dvg_default;
	vg = &vgd;
	vg->set_machine(machine);

	xmin = visarea.min_x;
	ymin = visarea.min_y;

	vg_halt_timer = machine.scheduler().timer_alloc(FUNC(vg_set_halt_callback));
	vg_run_timer = machine.scheduler().timer_alloc(FUNC(run_state_machine));

	register_state (machine);

	VIDEO_START_CALL(vector);
}

VIDEO_START( avg )
{
	vgc = &avg_default;
	VIDEO_START_CALL(avg_common);
}

VIDEO_START( avg_starwars )
{
	vgc = &avg_starwars;
	VIDEO_START_CALL(avg_common);
}

VIDEO_START( avg_tempest )
{
	vgc = &avg_tempest;
	VIDEO_START_CALL(avg_common);
}

VIDEO_START( avg_mhavoc )
{
	vgc = &avg_mhavoc;
	VIDEO_START_CALL(avg_common);
}

VIDEO_START( avg_bzone )
{
	vgc = &avg_bzone;
	VIDEO_START_CALL(avg_common);
}

VIDEO_START( avg_quantum )
{
	vgc = &avg_quantum;
	VIDEO_START_CALL(avg_common);
}

VIDEO_START( avg_tomcat )
{
	vgc = &avg_tomcat;
	VIDEO_START_CALL(avg_common);
}
