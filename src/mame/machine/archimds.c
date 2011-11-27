/******************************************************************************
 *
 *  Acorn Archimedes custom chips (IOC, MEMC, VIDC)
 *
 *  Memory map (from http://b-em.bbcmicro.com/arculator/archdocs.txt)
 *
 *  0000000 - 1FFFFFF - logical RAM (32 meg)
 *  2000000 - 2FFFFFF - physical RAM (supervisor only - max 16MB - requires quad MEMCs)
 *  3000000 - 33FFFFF - IOC (IO controllers - supervisor only)
 *  3310000 - FDC - WD1772
 *  33A0000 - Econet - 6854
 *  33B0000 - Serial - 6551
 *  3240000 - 33FFFFF - internal expansion cards
 *  32D0000 - hard disc controller (not IDE) - HD63463
 *  3350010 - printer
 *  3350018 - latch A
 *  3350040 - latch B
 *  3270000 - external expansion cards
 *
 *  3400000 - 3FFFFFF - ROM (read - 12 meg - Arthur and RiscOS 2 512k, RiscOS 3 2MB)
 *  3400000 - 37FFFFF - Low ROM  (4 meg, I think this is expansion ROMs)
 *  3800000 - 3FFFFFF - High ROM (main OS ROM)
 *
 *  3400000 - 35FFFFF - VICD10 (write - supervisor only)
 *  3600000 - 3FFFFFF - MEMC (write - supervisor only)
 *
 *****************************************************************************/

#include "emu.h"
#include "cpu/arm/arm.h"
#include "sound/dac.h"
#include "includes/archimds.h"
#include "machine/i2cmem.h"
#include "debugger.h"
#include "machine/wd17xx.h"

static const int page_sizes[4] = { 4096, 8192, 16384, 32768 };

#define IOC_LOG 0

UINT32 *archimedes_memc_physmem;
static UINT32 memc_pagesize;
static int memc_latchrom;
static UINT32 ioc_timercnt[4], ioc_timerout[4];
static UINT32 vidc_vidstart, vidc_vidend, vidc_vidinit,vidc_vidcur;
static UINT32 vidc_sndstart, vidc_sndend, vidc_sndcur;
static UINT8 video_dma_on,audio_dma_on;
UINT8 i2c_clk;
INT16 memc_pages[0x2000];	// the logical RAM area is 32 megs, and the smallest page size is 4k
UINT32 vidc_regs[256];
UINT8 ioc_regs[0x80/4];
UINT8 vidc_bpp_mode;
UINT8 vidc_interlace;
static UINT8 vidc_pixel_clk;
static const UINT32 pixel_rate[4] = { 8000000, 12000000, 16000000, 24000000};
static UINT8 vidc_stereo_reg[8];

static emu_timer *timer[4], *snd_timer, *vid_timer;
static emu_timer *vbl_timer;

void archimedes_request_irq_a(running_machine &machine, int mask)
{
	ioc_regs[IRQ_STATUS_A] |= mask;

	if (ioc_regs[IRQ_MASK_A] & mask)
	{
		cputag_set_input_line(machine, "maincpu", ARM_IRQ_LINE, ASSERT_LINE);
	}
}

void archimedes_request_irq_b(running_machine &machine, int mask)
{
	ioc_regs[IRQ_STATUS_B] |= mask;

	if (ioc_regs[IRQ_MASK_B] & mask)
	{
		generic_pulse_irq_line(machine.device("maincpu"), ARM_IRQ_LINE);
	}
}

void archimedes_request_fiq(running_machine &machine, int mask)
{
	ioc_regs[FIQ_STATUS] |= mask;

	if (ioc_regs[FIQ_MASK] & mask)
	{
		generic_pulse_irq_line(machine.device("maincpu"), ARM_FIRQ_LINE);
	}
}

void archimedes_clear_irq_a(running_machine &machine, int mask)
{
	ioc_regs[IRQ_STATUS_A] &= ~mask;
}

void archimedes_clear_irq_b(running_machine &machine, int mask)
{
	ioc_regs[IRQ_STATUS_B] &= ~mask;
}

void archimedes_clear_fiq(running_machine &machine, int mask)
{
	ioc_regs[FIQ_STATUS] &= ~mask;
}

static TIMER_CALLBACK( vidc_vblank )
{
	archimedes_request_irq_a(machine, ARCHIMEDES_IRQA_VBL);

	// set up for next vbl
	vbl_timer->adjust(machine.primary_screen->time_until_pos(vidc_regs[0xb4]));
}

/* video DMA */
/* TODO: what type of DMA this is, burst or cycle steal? Docs doesn't explain it (4 usec is the DRAM refresh). */
static TIMER_CALLBACK( vidc_video_tick )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	static UINT8 *vram = machine.region("vram")->base();
	UINT32 size;

	size = vidc_vidend-vidc_vidstart+0x10;

	for(vidc_vidcur = 0;vidc_vidcur < size;vidc_vidcur++)
		vram[vidc_vidcur] = (space->read_byte(vidc_vidstart+vidc_vidcur));

	if(video_dma_on)
		vid_timer->adjust(space->machine().primary_screen->time_until_pos(vidc_regs[0xb4]));
	else
		vid_timer->adjust(attotime::never);
}

/* audio DMA */
static TIMER_CALLBACK( vidc_audio_tick )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 ulaw_comp;
	INT16 res;
	UINT8 ch;
	static const char *const dac_port[8] = { "dac0", "dac1", "dac2", "dac3", "dac4", "dac5", "dac6", "dac7" };
	static const INT16 mulawTable[256] =
	{
		-32124,-31100,-30076,-29052,-28028,-27004,-25980,-24956,
		-23932,-22908,-21884,-20860,-19836,-18812,-17788,-16764,
		-15996,-15484,-14972,-14460,-13948,-13436,-12924,-12412,
		-11900,-11388,-10876,-10364, -9852, -9340, -8828, -8316,
		-7932, -7676, -7420, -7164, -6908, -6652, -6396, -6140,
		-5884, -5628, -5372, -5116, -4860, -4604, -4348, -4092,
		-3900, -3772, -3644, -3516, -3388, -3260, -3132, -3004,
		-2876, -2748, -2620, -2492, -2364, -2236, -2108, -1980,
		-1884, -1820, -1756, -1692, -1628, -1564, -1500, -1436,
		-1372, -1308, -1244, -1180, -1116, -1052,  -988,  -924,
		-876,  -844,  -812,  -780,  -748,  -716,  -684,  -652,
		-620,  -588,  -556,  -524,  -492,  -460,  -428,  -396,
		-372,  -356,  -340,  -324,  -308,  -292,  -276,  -260,
		-244,  -228,  -212,  -196,  -180,  -164,  -148,  -132,
		-120,  -112,  -104,   -96,   -88,   -80,   -72,   -64,
		-56,   -48,   -40,   -32,   -24,   -16,    -8,     -1,
		32124, 31100, 30076, 29052, 28028, 27004, 25980, 24956,
		23932, 22908, 21884, 20860, 19836, 18812, 17788, 16764,
		15996, 15484, 14972, 14460, 13948, 13436, 12924, 12412,
		11900, 11388, 10876, 10364,  9852,  9340,  8828,  8316,
		7932,  7676,  7420,  7164,  6908,  6652,  6396,  6140,
		5884,  5628,  5372,  5116,  4860,  4604,  4348,  4092,
		3900,  3772,  3644,  3516,  3388,  3260,  3132,  3004,
		2876,  2748,  2620,  2492,  2364,  2236,  2108,  1980,
		1884,  1820,  1756,  1692,  1628,  1564,  1500,  1436,
		1372,  1308,  1244,  1180,  1116,  1052,   988,   924,
		876,   844,   812,   780,   748,   716,   684,   652,
		620,   588,   556,   524,   492,   460,   428,   396,
		372,   356,   340,   324,   308,   292,   276,   260,
		244,   228,   212,   196,   180,   164,   148,   132,
		120,   112,   104,    96,    88,    80,    72,    64,
		 56,    48,    40,    32,    24,    16,     8,     0
	};

	for(ch=0;ch<8;ch++)
	{
		UINT8 ulaw_temp = (space->read_byte(vidc_sndstart+vidc_sndcur + ch)) ^ 0xff;

		ulaw_comp = (ulaw_temp>>1) | ((ulaw_temp&1)<<7);

		res = mulawTable[ulaw_comp];

		dac_signed_data_16_w(space->machine().device(dac_port[ch & 7]), res^0x8000);
	}

	vidc_sndcur+=8;

	if (vidc_sndcur >= (vidc_sndend-vidc_sndstart)+0x10)
	{
		vidc_sndcur = 0;
		archimedes_request_irq_b(machine, ARCHIMEDES_IRQB_SOUND_EMPTY);

		if(!audio_dma_on)
		{
			snd_timer->adjust(attotime::never);
			for(ch=0;ch<8;ch++)
				dac_signed_data_16_w(space->machine().device(dac_port[ch & 7]), 0x8000);
		}
	}
}

static void a310_set_timer(int tmr)
{
	double freq;

	switch(tmr)
	{
		case 0:
		case 1:
            timer[tmr]->adjust(attotime::from_usec(ioc_timercnt[tmr]/8), tmr); // TODO: ARM timings are quite off there, it should be latch and not latch/8
			break;
		case 2:
			freq = 1000000.0 / (double)(ioc_timercnt[tmr]+1);
			timer[tmr]->adjust(attotime::from_hz(freq), tmr);
			break;
		case 3:
			freq = 1000000.0 / (double)((ioc_timercnt[tmr]+1)*16);
			timer[tmr]->adjust(attotime::from_hz(freq), tmr);
			break;
	}
}

// param
static TIMER_CALLBACK( ioc_timer )
{
	// all timers always run
	a310_set_timer(param);

	// but only timers 0 and 1 generate IRQs
	switch (param)
	{
		case 0:
			archimedes_request_irq_a(machine, ARCHIMEDES_IRQA_TIMER0);
			break;

		case 1:
			archimedes_request_irq_a(machine, ARCHIMEDES_IRQA_TIMER1);
			break;
	}
}

void archimedes_reset(running_machine &machine)
{
	int i;

	memc_latchrom = 1;			// map in the boot ROM

	// kill all memc mappings
	for (i = 0; i < (32*1024*1024)/(4096); i++)
	{
		memc_pages[i] = -1;		// indicate unmapped
	}

	ioc_regs[IRQ_STATUS_A] = 0x10 | 0x80; //set up POR (Power On Reset) and Force IRQ at start-up
	ioc_regs[IRQ_STATUS_B] = 0x02; //set up IL[1] On
	ioc_regs[FIQ_STATUS] = 0x80;   //set up Force FIQ
	ioc_regs[CONTROL] = 0xff;
}

void archimedes_init(running_machine &machine)
{
	memc_pagesize = 0;

	vbl_timer = machine.scheduler().timer_alloc(FUNC(vidc_vblank));
	vbl_timer->adjust(attotime::never);

	timer[0] = machine.scheduler().timer_alloc(FUNC(ioc_timer));
	timer[1] = machine.scheduler().timer_alloc(FUNC(ioc_timer));
	timer[2] = machine.scheduler().timer_alloc(FUNC(ioc_timer));
	timer[3] = machine.scheduler().timer_alloc(FUNC(ioc_timer));
	timer[0]->adjust(attotime::never);
	timer[1]->adjust(attotime::never);
	timer[2]->adjust(attotime::never);
	timer[3]->adjust(attotime::never);

	vid_timer = machine.scheduler().timer_alloc(FUNC(vidc_video_tick));
	snd_timer = machine.scheduler().timer_alloc(FUNC(vidc_audio_tick));
	snd_timer->adjust(attotime::never);
}

READ32_HANDLER(archimedes_memc_logical_r)
{
	UINT32 page, poffs;

	// are we mapping in the boot ROM?
	if (memc_latchrom)
	{
		UINT32 *rom;

		rom = (UINT32 *)space->machine().region("maincpu")->base();

		return rom[offset & 0x1fffff];
	}
	else
	{
		// figure out the page number and offset in the page
		page = (offset<<2) / page_sizes[memc_pagesize];
		poffs = (offset<<2) % page_sizes[memc_pagesize];

//      printf("Reading offset %x (addr %x): page %x (size %d %d) offset %x ==> %x %x\n", offset, offset<<2, page, memc_pagesize, page_sizes[memc_pagesize], poffs, memc_pages[page], memc_pages[page]*page_sizes[memc_pagesize]);

		if (memc_pages[page] != -1)
		{
			return archimedes_memc_physmem[((memc_pages[page] * page_sizes[memc_pagesize]) + poffs)>>2];
		}
		else
		{
			//printf("ARCHIMEDES_MEMC: Reading unmapped page %02x\n",page);
			return 0xdeadbeef;
		}
	}

	return 0;
}



WRITE32_HANDLER(archimedes_memc_logical_w)
{
	UINT32 page, poffs;

	// if the boot ROM is mapped, ignore writes
	if (memc_latchrom)
	{
		return;
	}
	else
	{
		// figure out the page number and offset in the page
		page = (offset<<2) / page_sizes[memc_pagesize];
		poffs = (offset<<2) % page_sizes[memc_pagesize];

//      printf("Writing offset %x (addr %x): page %x (size %d %d) offset %x ==> %x %x\n", offset, offset<<2, page, memc_pagesize, page_sizes[memc_pagesize], poffs, memc_pages[page], memc_pages[page]*page_sizes[memc_pagesize]);

		if (memc_pages[page] != -1)
		{
			COMBINE_DATA(&archimedes_memc_physmem[((memc_pages[page] * page_sizes[memc_pagesize]) + poffs)>>2]);
		}
		else
		{
			//printf("ARCHIMEDES_MEMC: Writing unmapped page %02x, what do we do?\n",page);
		}
	}
}

/* Aristocrat Mark 5 - same as normal AA except with Dram emulator */
READ32_HANDLER(aristmk5_drame_memc_logical_r)
{
	UINT32 page, poffs;

	// are we mapping in the boot ROM?
	if (memc_latchrom)
	{
		UINT32 *rom;

		rom = (UINT32 *)space->machine().region("maincpu")->base();

		return rom[offset & 0x1fffff];
	}
	else
	{
		// figure out the page number and offset in the page
		page = (offset<<2) / page_sizes[memc_pagesize];
		poffs = (offset<<2) % page_sizes[memc_pagesize];



		if (memc_pages[page] != -1)
		{
            /******************* DRAM Emulator - gal20v - Aristocrat Mark 5 ************************
            A Dynamic RAM emulator is provided which avoids the need to execute code
            in DRAM in those regulatory environments where it is not needed.

            When pin 5 of U36 ( gal20v ) is low, the pin 25 output is high and enables the
            logic buffer inputs and provides a fixed jmp address to a plurality
            of rom addresses ( 0xEAD0000A  shown on logic buffer arrangement in schematics )

            In this state, DRAM memory space is disabled.

            ****************************************************************************************/
            if(!(memc_pages[page] & 0x10)  && (offset <= 0x3ff))
                return 0xEAD0000A;
            return archimedes_memc_physmem[((memc_pages[page] * page_sizes[memc_pagesize]) + poffs)>>2];
		}
		else
		{
			//printf("ARCHIMEDES_MEMC: Reading unmapped page %02x\n",page);
			return 0xdeadbeef;
		}
	}

	return 0;
}

#if 0
DIRECT_UPDATE_HANDLER( a310_setopbase )
{
	// if we're not in logical memory, MAME can do the right thing
	if (address > 0x1ffffff)
	{
		return address;
	}

	// if the boot ROM is mapped in, do some trickery to make it show up
	if (memc_latchrom)
	{
		direct.explicit_configure(0x000000, 0x1fffff, 0x1fffff, *direct.space().machine().region("maincpu"));
	}
	else	// executing from logical memory
	{
		offs_t pagesize = page_sizes[memc_pagesize];
		UINT32 page = address / pagesize;

		direct.explicit_configure(page * pagesize, page * pagesize - 1, pagesize - 1, &archimedes_memc_physmem[(memc_pages[page] * pagesize)>>2]);
	}

	return ~0;
}
#endif

void archimedes_driver_init(running_machine &machine)
{
//  address_space *space = machine.device<arm_device>("maincpu")->space(AS_PROGRAM);
//  space->set_direct_update_handler(direct_update_delegate(FUNC(a310_setopbase), &machine));
}

static const char *const ioc_regnames[] =
{
	"(rw) Control",					// 0
	"(read) Keyboard receive (write) keyboard send",	// 1
	"?",
	"?",
	"(read) IRQ status A",				// 4
	"(read) IRQ request A (write) IRQ clear",	// 5
	"(rw) IRQ mask A",				// 6
	"?",
	"(read) IRQ status B",		// 8
	"(read) IRQ request B",		// 9
	"(rw) IRQ mask B",		// 10
	"?",
	"(read) FIQ status",		// 12
	"(read) FIQ request",		// 13
	"(rw) FIQ mask",		// 14
	"?",
	"(read) Timer 0 count low (write) Timer 0 latch low",		// 16
	"(read) Timer 0 count high (write) Timer 0 latch high",		// 17
	"(write) Timer 0 go command",					// 18
	"(write) Timer 0 latch command",				// 19
	"(read) Timer 1 count low (write) Timer 1 latch low",		// 20
	"(read) Timer 1 count high (write) Timer 1 latch high",		// 21
	"(write) Timer 1 go command",					// 22
	"(write) Timer 1 latch command",				// 23
	"(read) Timer 2 count low (write) Timer 2 latch low",		// 24
	"(read) Timer 2 count high (write) Timer 2 latch high",		// 25
	"(write) Timer 2 go command",					// 26
	"(write) Timer 2 latch command",				// 27
	"(read) Timer 3 count low (write) Timer 3 latch low",		// 28
	"(read) Timer 3 count high (write) Timer 3 latch high",		// 29
	"(write) Timer 3 go command",					// 30
	"(write) Timer 3 latch command"					// 31
};

static void latch_timer_cnt(int tmr)
{
	double time = timer[tmr]->elapsed().as_double();
	time *= 2000000.0;	// find out how many 2 MHz ticks have gone by
	ioc_timerout[tmr] = ioc_timercnt[tmr] - (UINT32)time;
}

/* TODO: should be a 8-bit handler */
static READ32_HANDLER( ioc_ctrl_r )
{
	if(IOC_LOG)
	logerror("IOC: R %s = %02x (PC=%x) %02x\n", ioc_regnames[offset&0x1f], ioc_regs[offset&0x1f], cpu_get_pc( &space->device() ),offset & 0x1f);

	switch (offset & 0x1f)
	{
		case CONTROL:
		{
			UINT8 i2c_data;
			static UINT8 flyback; //internal name for vblank here
			int vert_pos;

			vert_pos = space->machine().primary_screen->vpos();
			flyback = (vert_pos <= vidc_regs[VIDC_VDSR] || vert_pos >= vidc_regs[VIDC_VDER]) ? 0x80 : 0x00;

			i2c_data = (i2cmem_sda_read(space->machine().device("i2cmem")) & 1);

			return (flyback) | (ioc_regs[CONTROL] & 0x7c) | (i2c_clk<<1) | i2c_data;
		}

		case KART:	// keyboard read
			archimedes_request_irq_b(space->machine(), ARCHIMEDES_IRQB_KBD_XMIT_EMPTY);
			break;

		case IRQ_STATUS_A:
			return (ioc_regs[IRQ_STATUS_A] & 0x7f) | 0x80; // Force IRQ is always '1'

		case IRQ_REQUEST_A:
			return (ioc_regs[IRQ_STATUS_A] & ioc_regs[IRQ_MASK_A]);

		case IRQ_MASK_A:
			return (ioc_regs[IRQ_MASK_A]);

		case IRQ_STATUS_B:
			return (ioc_regs[IRQ_STATUS_B]);

		case IRQ_REQUEST_B:
			return (ioc_regs[IRQ_STATUS_B] & ioc_regs[IRQ_MASK_B]);

		case IRQ_MASK_B:
			return (ioc_regs[IRQ_MASK_B]);

		case FIQ_STATUS:
			return (ioc_regs[FIQ_STATUS] & 0x7f) | 0x80; // Force FIQ is always '1'

		case FIQ_REQUEST:
			return (ioc_regs[FIQ_STATUS] & ioc_regs[FIQ_MASK]);

		case FIQ_MASK:
			return (ioc_regs[FIQ_MASK]);

		case T0_LATCH_LO: return ioc_timerout[0]&0xff;
		case T0_LATCH_HI: return (ioc_timerout[0]>>8)&0xff;

		case T1_LATCH_LO: return ioc_timerout[1]&0xff;
		case T1_LATCH_HI: return (ioc_timerout[1]>>8)&0xff;

		case T2_LATCH_LO: return ioc_timerout[2]&0xff;
		case T2_LATCH_HI: return (ioc_timerout[2]>>8)&0xff;

		case T3_LATCH_LO: return ioc_timerout[3]&0xff;
		case T3_LATCH_HI: return (ioc_timerout[3]>>8)&0xff;
		default:
			if(!IOC_LOG)
				logerror("IOC: R %s = %02x (PC=%x) %02x\n", ioc_regnames[offset&0x1f], ioc_regs[offset&0x1f], cpu_get_pc( &space->device() ),offset & 0x1f);
			break;
	}

	return ioc_regs[offset&0x1f];
}

/* TODO: should be a 8-bit handler */
static WRITE32_HANDLER( ioc_ctrl_w )
{
	if(IOC_LOG)
	logerror("IOC: W %02x @ reg %s (PC=%x)\n", data&0xff, ioc_regnames[offset&0x1f], cpu_get_pc( &space->device() ));

	switch (offset&0x1f)
	{
		case CONTROL:	// I2C bus control
			//logerror("IOC I2C: CLK %d DAT %d\n", (data>>1)&1, data&1);
			i2cmem_sda_write(space->machine().device("i2cmem"), data & 0x01);
			i2cmem_scl_write(space->machine().device("i2cmem"), (data & 0x02) >> 1);
			i2c_clk = (data & 2) >> 1;
			break;

		case KART:
			#if 0
			if(data == 0x0d)
				printf("\n");
			else
				printf("%c",data);
			#endif
			break;

		case IRQ_MASK_A:
			ioc_regs[IRQ_MASK_A] = data & 0xff;

			if(data & 0x80) //force an IRQ
				archimedes_request_irq_a(space->machine(),ARCHIMEDES_IRQA_FORCE);

			if(data & 0x08) //set up the VBLANK timer
				vbl_timer->adjust(space->machine().primary_screen->time_until_pos(vidc_regs[0xb4]));

			break;

		case FIQ_MASK:
			ioc_regs[FIQ_MASK] = data & 0xff;

			if(data & 0x80) //force a FIRQ
				archimedes_request_fiq(space->machine(),ARCHIMEDES_FIQ_FORCE);

			break;

		case IRQ_REQUEST_A: 	// IRQ clear A
			ioc_regs[IRQ_STATUS_A] &= ~(data&0xff);

			// if that did it, clear the IRQ
			//if (ioc_regs[IRQ_STATUS_A] == 0)
			{
				//printf("IRQ clear A\n");
				cputag_set_input_line(space->machine(), "maincpu", ARM_IRQ_LINE, CLEAR_LINE);
			}
			break;

		case T0_LATCH_LO:
		case T0_LATCH_HI:
			ioc_regs[offset&0x1f] = data & 0xff;
			break;

		case T1_LATCH_LO:
		case T1_LATCH_HI:
			ioc_regs[offset&0x1f] = data & 0xff;
			break;

		case T2_LATCH_LO:
		case T2_LATCH_HI:
			ioc_regs[offset&0x1f] = data & 0xff;
			break;

		case T3_LATCH_LO:
		case T3_LATCH_HI:
			ioc_regs[offset&0x1f] = data & 0xff;
			break;

		case T0_LATCH:	// Timer 0 latch
			latch_timer_cnt(0);
			break;

		case T1_LATCH:	// Timer 1 latch
			latch_timer_cnt(1);
			break;

		case T2_LATCH:	// Timer 2 latch
			latch_timer_cnt(2);
			break;

		case T3_LATCH:	// Timer 3 latch
			latch_timer_cnt(3);
			break;

		case T0_GO:	// Timer 0 start
			ioc_timercnt[0] = ioc_regs[T0_LATCH_HI]<<8 | ioc_regs[T0_LATCH_LO];
			a310_set_timer(0);
			break;

		case T1_GO:	// Timer 1 start
			ioc_timercnt[1] = ioc_regs[T1_LATCH_HI]<<8 | ioc_regs[T1_LATCH_LO];
			a310_set_timer(1);
			break;

		case T2_GO:	// Timer 2 start
			ioc_timercnt[2] = ioc_regs[T2_LATCH_HI]<<8 | ioc_regs[T2_LATCH_LO];
			a310_set_timer(2);
			break;

		case T3_GO:	// Timer 3 start
			ioc_timercnt[3] = ioc_regs[T3_LATCH_HI]<<8 | ioc_regs[T3_LATCH_LO];
			a310_set_timer(3);
			break;

		default:
			if(!IOC_LOG)
				logerror("IOC: W %02x @ reg %s (PC=%x)\n", data&0xff, ioc_regnames[offset&0x1f], cpu_get_pc( &space->device() ));

			ioc_regs[offset&0x1f] = data & 0xff;
			break;
	}
}

READ32_HANDLER(archimedes_ioc_r)
{
	UINT32 ioc_addr;
	device_t *fdc = (device_t *)space->machine().device("wd1772");

	ioc_addr = offset*4;

	switch((ioc_addr & 0x300000) >> 20)
	{
		/*82c711*/
		case 0:
			logerror("82c711 read at address %08x\n",ioc_addr);
			return 0;
		case 2:
		case 3:
		{
			switch((ioc_addr & 0x70000) >> 16)
			{
				case 0: return ioc_ctrl_r(space,offset,mem_mask);
				case 1:
					if (fdc) {
						logerror("17XX: R @ addr %x mask %08x\n", offset*4, mem_mask);
						return wd17xx_data_r(fdc, offset&0xf);
					} else {
						logerror("Read from FDC device?\n");
						return 0;
					}
				case 2:
					logerror("IOC: Econet Read %08x\n",ioc_addr);
					return 0xffff;
				case 3:
					logerror("IOC: Serial Read\n");
					return 0xffff;
				case 4:
					logerror("IOC: Internal Podule Read\n");
					return 0xffff;
				case 5:
					if (fdc) {
						switch(ioc_addr & 0xfffc)
						{
							case 0x50: return 0; //fdc type, new model returns 5 here
						}
					}

					logerror("IOC: Internal Latches Read %08x\n",ioc_addr);

					return 0xffff;
			}
		}
	}

	logerror("IOC: Unknown read at %08x\n",ioc_addr);

	return 0;
}

WRITE32_HANDLER(archimedes_ioc_w)
{
	UINT32 ioc_addr;
	device_t *fdc = (device_t *)space->machine().device("wd1772");

	ioc_addr = offset*4;

	switch((ioc_addr & 0x300000) >> 20)
	{
		/*82c711*/
		case 0:
			logerror("82c711 write %08x to address %08x\n",data,ioc_addr);
			return;
		case 2:
		case 3:
		{
			switch((ioc_addr & 0x70000) >> 16)
			{
				case 0: ioc_ctrl_w(space,offset,data,mem_mask); return;
				case 1:
						if (fdc) {
							logerror("17XX: %x to addr %x mask %08x\n", data, offset*4, mem_mask);
							wd17xx_data_w(fdc, offset&0xf, data&0xff);
						} else {
							logerror("Write to FDC device?\n");
						}
						return;
				case 2:
					logerror("IOC: Econet Write %02x at %08x\n",data,ioc_addr);
					return;
				case 3:
					logerror("IOC: Serial Write %02x (%c) at %08x\n",data,data,ioc_addr);
					return;
				case 4:
					logerror("IOC: Internal Podule Write\n");
					return;
				case 5:
					if (fdc) {
						switch(ioc_addr & 0xfffc)
						{
							case 0x18: // latch B
								wd17xx_dden_w(fdc, BIT(data, 1));
								return;

							case 0x40: // latch A
								if (data & 1) { wd17xx_set_drive(fdc,0); }
								if (data & 2) {	wd17xx_set_drive(fdc,1); }
								if (data & 4) { wd17xx_set_drive(fdc,2); }
								if (data & 8) {	wd17xx_set_drive(fdc,3); }

								wd17xx_set_side(fdc,(data & 0x10)>>4);
								//bit 5 is motor on
								return;
						}
					}
					break;
			}
		}
	}


	logerror("(PC=%08x) I/O: W %x @ %x (mask %08x)\n", cpu_get_pc(&space->device()), data, (offset*4)+0x3000000, mem_mask);
}

READ32_HANDLER(archimedes_vidc_r)
{
	return 0;
}

static void vidc_dynamic_res_change(running_machine &machine)
{
	/* sanity checks - first pass */
	/*
        total cycles + border end
    */
	if(vidc_regs[VIDC_HCR] && vidc_regs[VIDC_HBER] &&
	   vidc_regs[VIDC_VCR] && vidc_regs[VIDC_VBER])
	{
		/* sanity checks - second pass */
		/*
        total cycles >= border end >= border start
        */
		if((vidc_regs[VIDC_HCR] >= vidc_regs[VIDC_HBER]) &&
		   (vidc_regs[VIDC_HBER] >= vidc_regs[VIDC_HBSR]) &&
		   (vidc_regs[VIDC_VCR] >= vidc_regs[VIDC_VBER]) &&
		   (vidc_regs[VIDC_VBER] >= vidc_regs[VIDC_VBSR]))
		{
			rectangle visarea;
			attoseconds_t refresh;

			visarea.min_x = 0;
			visarea.min_y = 0;
			visarea.max_x = vidc_regs[VIDC_HBER] - vidc_regs[VIDC_HBSR] - 1;
			visarea.max_y = vidc_regs[VIDC_VBER] - vidc_regs[VIDC_VBSR];

			logerror("Configuring: htotal %d vtotal %d border %d x %d display %d x %d\n",
				vidc_regs[VIDC_HCR], vidc_regs[VIDC_VCR],
				visarea.max_x, visarea.max_y,
				vidc_regs[VIDC_HDER]-vidc_regs[VIDC_HDSR],vidc_regs[VIDC_VDER]-vidc_regs[VIDC_VDSR]+1);

			/* FIXME: pixel clock */
			refresh = HZ_TO_ATTOSECONDS(pixel_rate[vidc_pixel_clk]*2) * vidc_regs[VIDC_HCR] * vidc_regs[VIDC_VCR];

			machine.primary_screen->configure(vidc_regs[VIDC_HCR], vidc_regs[VIDC_VCR], visarea, refresh);
		}
	}
}

WRITE32_HANDLER(archimedes_vidc_w)
{
	UINT32 reg = data>>24;
	UINT32 val = data & 0xffffff;
	//#ifdef DEBUG
	static const char *const vrnames[] =
	{
		"horizontal total",
		"horizontal sync width",
		"horizontal border start",
		"horizontal display start",
		"horizontal display end",
		"horizontal border end",
		"horizontal cursor start",
		"horizontal interlace",
		"vertical total",
		"vertical sync width",
		"vertical border start",
		"vertical display start",
		"vertical display end",
		"vertical border end",
		"vertical cursor start",
		"vertical cursor end",
	};
	//#endif


	// 0x00 - 0x3c Video Palette Logical Colors (16 colors)
	// 0x40 Border Color
	// 0x44 - 0x4c Cursor Palette Logical Colors
	if (reg >= 0x00 && reg <= 0x4c)
	{
		int r,g,b;

		//i = (val & 0x1000) >> 12; //supremacy bit
		b = (val & 0x0f00) >> 8;
		g = (val & 0x00f0) >> 4;
		r = (val & 0x000f) >> 0;

		if(reg == 0x40 && val & 0xfff)
			logerror("WARNING: border color write here (PC=%08x)!\n",cpu_get_pc(&space->device()));

		palette_set_color_rgb(space->machine(), reg >> 2, pal4bit(r), pal4bit(g), pal4bit(b) );

		/* handle 8bpp colors here */
		if(reg <= 0x3c)
		{
			int i;

			for(i=0;i<0x100;i+=0x10)
			{
				b = ((val & 0x700) >> 8) | ((i & 0x80) >> 4);
				g = ((val & 0x030) >> 4) | ((i & 0x20) >> 3) | ((i & 0x40) >> 3);
				r = ((val & 0x007) >> 0) | ((i & 0x10) >> 1);

				palette_set_color_rgb(space->machine(), (reg >> 2) + 0x100 + i, pal4bit(r), pal4bit(g), pal4bit(b) );
			}
		}

	}
	else if (reg >= 0x60 && reg <= 0x7c)
	{
		vidc_stereo_reg[(reg >> 2) & 7] = val & 0x07;

//      popmessage("%02x %02x %02x %02x %02x %02x %02x %02x",vidc_stereo_reg[0],vidc_stereo_reg[1],vidc_stereo_reg[2],vidc_stereo_reg[3]
//      ,vidc_stereo_reg[4],vidc_stereo_reg[5],vidc_stereo_reg[6],vidc_stereo_reg[7]);
	}
	else if (reg >= 0x80 && reg <= 0xbc)
	{
		switch(reg)
		{
			case VIDC_HCR:  vidc_regs[VIDC_HCR] =  ((val >> 14)<<1)+1;	break;
//          case VIDC_HSWR: vidc_regs[VIDC_HSWR] = (val >> 14)+1;   break;
			case VIDC_HBSR: vidc_regs[VIDC_HBSR] = ((val >> 14)<<1)+1;	break;
			case VIDC_HDSR: vidc_regs[VIDC_HDSR] = (val >> 14); 	break;
			case VIDC_HDER: vidc_regs[VIDC_HDER] = (val >> 14); 	break;
			case VIDC_HBER: vidc_regs[VIDC_HBER] = ((val >> 14)<<1)+1;	break;
//          #define VIDC_HCSR       0x98
//          #define VIDC_HIR        0x9c

			case VIDC_VCR:  vidc_regs[VIDC_VCR] = ((val >> 14)<<1)+1;	break;
//          #define VIDC_VSWR       0xa4
			case VIDC_VBSR: vidc_regs[VIDC_VBSR] = (val >> 14)+1;	break;
			case VIDC_VDSR: vidc_regs[VIDC_VDSR] = (val >> 14)+1;	break;
			case VIDC_VDER: vidc_regs[VIDC_VDER] = (val >> 14)+1;	break;
			case VIDC_VBER: vidc_regs[VIDC_VBER] = (val >> 14)+1;	break;
//          #define VIDC_VCSR       0xb8
//          #define VIDC_VCER       0xbc
		}


		//#ifdef DEBUG
		logerror("VIDC: %s = %d\n", vrnames[(reg-0x80)/4], vidc_regs[reg]);
		//#endif

		vidc_dynamic_res_change(space->machine());
	}
	else if(reg == 0xe0)
	{
		vidc_bpp_mode = ((val & 0x0c) >> 2);
		vidc_interlace = ((val & 0x40) >> 6);
		vidc_pixel_clk = (val & 0x03);
		vidc_dynamic_res_change(space->machine());
	}
	else
	{
		logerror("VIDC: %x to register %x\n", val, reg);
		vidc_regs[reg] = val&0xffff;
	}
}

READ32_HANDLER(archimedes_memc_r)
{
	return 0;
}

WRITE32_HANDLER(archimedes_memc_w)
{
	// is it a register?
	if ((data & 0x0fe00000) == 0x03600000)
	{
		switch ((data >> 17) & 7)
		{
			case 0: /* video init */
				vidc_vidinit = ((data>>2)&0x7fff)*16;
				//logerror("MEMC: VIDINIT %08x\n",vidc_vidinit);
				break;

			case 1: /* video start */
				vidc_vidstart = 0x2000000 | (((data>>2)&0x7fff)*16);
				//logerror("MEMC: VIDSTART %08x\n",vidc_vidstart);
				break;

			case 2: /* video end */
				vidc_vidend = 0x2000000 | (((data>>2)&0x7fff)*16);
				//logerror("MEMC: VIDEND %08x\n",vidc_vidend);
				break;

			case 4:	/* sound start */
				//logerror("MEMC: SNDSTART %08x\n",data);
				vidc_sndstart = 0x2000000 | ((data>>2)&0x7fff)*16;
				ioc_regs[IRQ_STATUS_B] &= ~ARCHIMEDES_IRQB_SOUND_EMPTY;
				break;

			case 5: /* sound end */
				//logerror("MEMC: SNDEND %08x\n",data);
				vidc_sndend = 0x2000000 | ((data>>2)&0x7fff)*16;
				break;

			case 6:
				vidc_sndcur = 0;
				archimedes_request_irq_b(space->machine(), ARCHIMEDES_IRQB_SOUND_EMPTY);
				break;

			case 7:	/* Control */
				memc_pagesize = ((data>>2) & 3);

				logerror("(PC = %08x) MEMC: %x to Control (page size %d, %s, %s)\n", cpu_get_pc(&space->device()), data & 0x1ffc, page_sizes[memc_pagesize], ((data>>10)&1) ? "Video DMA on" : "Video DMA off", ((data>>11)&1) ? "Sound DMA on" : "Sound DMA off");

				video_dma_on = ((data>>10)&1);
				audio_dma_on = ((data>>11)&1);

				if ((data>>10)&1)
				{
					vidc_vidcur = 0;
					vid_timer->adjust(space->machine().primary_screen->time_until_pos(vidc_regs[0xb4]));
				}

				if ((data>>11)&1)
				{
					double sndhz;

					/* FIXME: is the frequency correct? */
					sndhz = (250000.0 / 2) / (double)((vidc_regs[0xc0]&0xff)+2);

					printf("MEMC: Starting audio DMA at %f Hz, buffer from %x to %x\n", sndhz, vidc_sndstart, vidc_sndend);

					snd_timer->adjust(attotime::zero, 0, attotime::from_hz(sndhz));
				}

				break;

			default:
				logerror("MEMC: %x to Unk reg %d\n", data&0x1ffff, (data >> 17) & 7);
				break;
		}
	}
	else
	{
		logerror("MEMC non-reg: W %x @ %x (mask %08x)\n", data, offset, mem_mask);
	}
}

/*
          22 2222 1111 1111 1100 0000 0000
          54 3210 9876 5432 1098 7654 3210
4k  page: 11 1LLL LLLL LLLL LLAA MPPP PPPP
8k  page: 11 1LLL LLLL LLLM LLAA MPPP PPPP
16k page: 11 1LLL LLLL LLxM LLAA MPPP PPPP
32k page: 11 1LLL LLLL LxxM LLAA MPPP PPPP
       3   8    2   9    0    f    f

L - logical page
P - physical page
A - access permissions
M - MEMC number (for machines with multiple MEMCs)

The logical page is encoded with bits 11+10 being the most significant bits
(in that order), and the rest being bit 22 down.

The physical page is encoded differently depending on the page size :

4k  page:   bits 6-0 being bits 6-0
8k  page:   bits 6-1 being bits 5-0, bit 0 being bit 6
16k page:   bits 6-2 being bits 4-0, bits 1-0 being bits 6-5
32k page:   bits 6-3 being bits 4-0, bit 0 being bit 4, bit 2 being bit 5, bit
            1 being bit 6
*/

WRITE32_HANDLER(archimedes_memc_page_w)
{
	UINT32 log, phys, memc;

//  perms = (data & 0x300)>>8;
	log = phys = memc = 0;

	switch (memc_pagesize)
	{
		case 0:
			phys = data & 0x7f;
			log = ((data & 0x7ff000)>>12) | ((data & 0xc00)<<1);
			memc = (data & 0x80) ? 1 : 0;
			break;

		case 1:
			phys = ((data & 0x7f) >> 1) | ((data & 1) << 6);
			log = ((data & 0x7fe000)>>13) | (data & 0xc00);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			break;

		case 2:
			phys = ((data & 0x7f) >> 2) | ((data & 3) << 5);
			log = ((data & 0x7fc000)>>14) | ((data & 0xc00)>>1);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			break;

		case 3:
			phys = ((data & 0x7f) >> 3) | ((data & 1)<<4) | ((data & 2) << 5) | ((data & 4)<<3);
			log = ((data & 0x7f8000)>>15) | ((data & 0xc00)>>2);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			//printf("Mapping %08X to %08X\n",0x2000000+(phys*32768),(((data >> 15)&0xff)|((data >> 2)&0x300)));
			break;
	}

//  log >>= (12 + memc_pagesize);

	// always make sure ROM mode is disconnected when this occurs
	memc_latchrom = 0;

	// now go ahead and set the mapping in the page table
	memc_pages[log] = phys + (memc*0x80);

//  printf("PC=%08x = MEMC_PAGE(%d): W %08x: log %x to phys %x, MEMC %d, perms %d\n", cpu_get_pc(&space->device()),memc_pagesize, data, log, phys, memc, perms);
}

