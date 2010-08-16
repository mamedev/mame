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

#ifdef MESS
#include "machine/wd17xx.h"
#endif

static const int page_sizes[4] = { 4096, 8192, 16384, 32768 };

UINT32 *archimedes_memc_physmem;
static UINT32 memc_pagesize;
static int memc_latchrom;
static INT16 memc_pages[(32*1024*1024)/(4096)];	// the logical RAM area is 32 megs, and the smallest page size is 4k
static UINT32 vidc_regs[256];
static UINT8 ioc_regs[0x80/4];
static UINT32 ioc_timercnt[4], ioc_timerout[4];
static UINT32 vidc_sndstart, vidc_sndend, vidc_sndcur;

static emu_timer *vbl_timer, *timer[4], *snd_timer;

void archimedes_request_irq_a(running_machine *machine, int mask)
{
	ioc_regs[4] |= mask;

	if (ioc_regs[6] & mask)
	{
		cputag_set_input_line(machine, "maincpu", ARM_IRQ_LINE, ASSERT_LINE);
	}
}

void archimedes_request_irq_b(running_machine *machine, int mask)
{
	ioc_regs[8] |= mask;

	if (ioc_regs[10] & mask)
	{
		generic_pulse_irq_line(machine->device("maincpu"), ARM_IRQ_LINE);
	}
}

void archimedes_request_fiq(running_machine *machine, int mask)
{
	ioc_regs[12] |= mask;

	if (ioc_regs[14] & mask)
	{
		generic_pulse_irq_line(machine->device("maincpu"), ARM_FIRQ_LINE);
	}
}

void archimedes_clear_irq_a(running_machine *machine, int mask)
{
	ioc_regs[4] &= ~mask;
}

void archimedes_clear_irq_b(running_machine *machine, int mask)
{
	ioc_regs[8] &= ~mask;
}

void archimedes_clear_fiq(running_machine *machine, int mask)
{
	ioc_regs[12] &= ~mask;
}

static TIMER_CALLBACK( vidc_vblank )
{
	archimedes_request_irq_a(machine, ARCHIMEDES_IRQA_VBL);

	// set up for next vbl
	timer_adjust_oneshot(vbl_timer, machine->primary_screen->time_until_pos(vidc_regs[0xb4]), 0);
}

static TIMER_CALLBACK( a310_audio_tick )
{
	vidc_sndcur++;

	if (vidc_sndcur >= vidc_sndend)
	{
		archimedes_request_irq_b(machine, ARCHIMEDES_IRQB_SOUND_EMPTY);
	}
}

static void a310_set_timer(int tmr)
{
	double freq;

	if(ioc_timercnt[tmr] != 0) // FIXME: dmdtouch does a divide by zero?
	{
		freq = 2000000.0 / (double)ioc_timercnt[tmr];
	//  logerror("IOC: starting timer %d, %d ticks, freq %f Hz\n", tmr, ioc_timercnt[tmr], freq);
		timer_adjust_oneshot(timer[tmr], ATTOTIME_IN_HZ(freq), tmr);
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

void archimedes_reset(running_machine *machine)
{
	int i;

	memc_latchrom = 1;			// map in the boot ROM

	// kill all memc mappings
	for (i = 0; i < (32*1024*1024)/(4096); i++)
	{
		memc_pages[i] = -1;		// indicate unmapped
	}

	ioc_regs[4] = 0x10; //set up POR (Power On Reset) at start-up
	ioc_regs[8] = 0x02; //set up IL[1] On
}

void archimedes_init(running_machine *machine)
{
	memc_pagesize = 0;

	vbl_timer = timer_alloc(machine, vidc_vblank, NULL);
	timer_adjust_oneshot(vbl_timer, attotime_never, 0);

	timer[0] = timer_alloc(machine, ioc_timer, NULL);
	timer[1] = timer_alloc(machine, ioc_timer, NULL);
	timer[2] = timer_alloc(machine, ioc_timer, NULL);
	timer[3] = timer_alloc(machine, ioc_timer, NULL);
	timer_adjust_oneshot(timer[0], attotime_never, 0);
	timer_adjust_oneshot(timer[1], attotime_never, 0);
	timer_adjust_oneshot(timer[2], attotime_never, 0);
	timer_adjust_oneshot(timer[3], attotime_never, 0);

	snd_timer = timer_alloc(machine, a310_audio_tick, NULL);
	timer_adjust_oneshot(snd_timer, attotime_never, 0);
}

READ32_HANDLER(archimedes_memc_logical_r)
{
	UINT32 page, poffs;

	// are we mapping in the boot ROM?
	if (memc_latchrom)
	{
		UINT32 *rom;

		rom = (UINT32 *)memory_region(space->machine, "maincpu");

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
			logerror("ARCHIMEDES_MEMC: Reading unmapped page, what do we do?\n");
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
			logerror("ARCHIMEDES_MEMC: Writing unmapped page, what do we do?\n");
		}
	}
}

static DIRECT_UPDATE_HANDLER( a310_setopbase )
{
	// if we're not in logical memory, MAME can do the right thing
	if (address > 0x1ffffff)
	{
		return address;
	}

	// if the boot ROM is mapped in, do some trickery to make it show up
	if (memc_latchrom)
	{
		direct->bytemask = 0x1fffff;
		direct->bytestart = 0;
		direct->byteend = 0x1fffff;
		direct->raw = direct->decrypted = memory_region(space->machine, "maincpu");
	}
	else	// executing from logical memory
	{
		UINT32 page = address / page_sizes[memc_pagesize];

		direct->bytemask = page_sizes[memc_pagesize]-1;
		direct->bytestart = page * page_sizes[memc_pagesize];
		direct->byteend = direct->bytestart + direct->bytemask;
		direct->raw = direct->decrypted = (UINT8 *)&archimedes_memc_physmem[(memc_pages[page] * page_sizes[memc_pagesize])>>2];
	}

	return ~0;
}

void archimedes_driver_init(running_machine *machine)
{
	memory_set_direct_update_handler( cputag_get_address_space( machine, "maincpu", ADDRESS_SPACE_PROGRAM ), a310_setopbase);
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
	double time = attotime_to_double(timer_timeelapsed(timer[tmr]));
	time *= 2000000.0;	// find out how many 2 MHz ticks have gone by
	ioc_timerout[tmr] = ioc_timercnt[tmr] - (UINT32)time;
}

READ32_HANDLER(archimedes_ioc_r)
{
	#ifdef MESS
	running_device *fdc = (running_device *)space->machine->device("wd1772");
	#endif
	if (offset*4 >= 0x200000 && offset*4 < 0x300000)
	{
		switch (offset & 0x1f)
		{
			case 1:	// keyboard read
				archimedes_request_irq_b(space->machine, ARCHIMEDES_IRQB_KBD_XMIT_EMPTY);
				break;

			case 16:	// timer 0 read
				return ioc_timerout[0]&0xff;
			case 17:
				return (ioc_timerout[0]>>8)&0xff;
			case 20:	// timer 1 read
				return ioc_timerout[1]&0xff;
			case 21:
				return (ioc_timerout[1]>>8)&0xff;
			case 24:	// timer 2 read
				return ioc_timerout[2]&0xff;
			case 25:
				return (ioc_timerout[2]>>8)&0xff;
			case 28:	// timer 3 read
				return ioc_timerout[3]&0xff;
			case 29:
				return (ioc_timerout[3]>>8)&0xff;
		}

		logerror("IOC: R %s = %02x (PC=%x) %02x\n", ioc_regnames[offset&0x1f], ioc_regs[offset&0x1f], cpu_get_pc( space->cpu ),offset & 0x1f);
		return ioc_regs[offset&0x1f];
	}
	#ifdef MESS
	else if (offset*4 >= 0x310000 && offset*4 < 0x310040)
	{
		logerror("17XX: R @ addr %x mask %08x\n", offset*4, mem_mask);
		return wd17xx_data_r(fdc, offset&0xf);
	}
	#endif
	else
	{
		logerror("IOC: R @ %x (mask %08x)\n", (offset*4)+0x3000000, mem_mask);
	}


	return 0;
}

WRITE32_HANDLER(archimedes_ioc_w)
{
	#ifdef MESS
	running_device *fdc = (running_device *)space->machine->device("wd1772");
	#endif

	if (offset*4 >= 0x200000 && offset*4 < 0x300000)
	{
//     	logerror("IOC: W %02x @ reg %s (PC=%x)\n", data&0xff, ioc_regnames[offset&0x1f], cpu_get_pc( space->cpu ));

		switch (offset&0x1f)
		{
			case 0:	// I2C bus control
				//logerror("IOC I2C: CLK %d DAT %d\n", (data>>1)&1, data&1);
				break;

			case 5: 	// IRQ clear A
				ioc_regs[4] &= ~(data&0xff);

				// if that did it, clear the IRQ
				if (ioc_regs[4] == 0)
				{
					cputag_set_input_line(space->machine, "maincpu", ARM_IRQ_LINE, CLEAR_LINE);
				}
				break;

			case 16:
			case 17:
				ioc_regs[offset&0x1f] = data & 0xff;
				break;

			case 20:
			case 21:
				ioc_regs[offset&0x1f] = data & 0xff;
				break;

			case 24:
			case 25:
				ioc_regs[offset&0x1f] = data & 0xff;
				break;

			case 28:
			case 29:
				ioc_regs[offset&0x1f] = data & 0xff;
				break;

			case 19:	// Timer 0 latch
				latch_timer_cnt(0);
				break;

			case 23:	// Timer 1 latch
				latch_timer_cnt(1);
				break;

			case 27:	// Timer 2 latch
				latch_timer_cnt(2);
				break;

			case 31:	// Timer 3 latch
				latch_timer_cnt(3);
				break;

			case 18:	// Timer 0 start
				ioc_timercnt[0] = ioc_regs[17]<<8 | ioc_regs[16];
				a310_set_timer(0);
				break;

			case 22:	// Timer 1 start
				ioc_timercnt[1] = ioc_regs[21]<<8 | ioc_regs[20];
				a310_set_timer(1);
				break;

			case 26:	// Timer 2 start
				ioc_timercnt[2] = ioc_regs[25]<<8 | ioc_regs[24];
				a310_set_timer(2);
				break;

			case 30:	// Timer 3 start
				ioc_timercnt[3] = ioc_regs[29]<<8 | ioc_regs[28];
				a310_set_timer(3);
				break;

			default:
				ioc_regs[offset&0x1f] = data & 0xff;
				break;
		}
	}
	#ifdef MESS
	else if (offset*4 >= 0x310000 && offset*4 < 0x310040)
	{
		logerror("17XX: %x to addr %x mask %08x\n", data, offset*4, mem_mask);
		wd17xx_data_w(fdc, offset&0xf, data&0xff);
	}
	else if (offset*4 == 0x350018)
	{
		// latch A
		if (data & 1)
		{
			wd17xx_set_drive(fdc,0);
		}
		if (data & 2)
		{
			wd17xx_set_drive(fdc,1);
		}
		if (data & 4)
		{
			wd17xx_set_drive(fdc,2);
		}
		if (data & 8)
		{
			wd17xx_set_drive(fdc,3);
		}

		wd17xx_set_side(fdc,(data & 0x10)>>4);

	}
	else if (offset*4 == 0x350040)
	{
		// latch B
		wd17xx_dden_w(fdc, BIT(data, 1));
	}
	#endif
	else
	{
		logerror("I/O: W %x @ %x (mask %08x)\n", data, (offset*4)+0x3000000, mem_mask);
	}
}

READ32_HANDLER(archimedes_vidc_r)
{
	return 0;
}

WRITE32_HANDLER(archimedes_vidc_w)
{
	UINT32 reg = data>>24;
	UINT32 val = data & 0xffffff;
	#ifdef DEBUG
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
	#endif

	// 0x00 - 0x3c Video Palette Logical Colors (16 colors)
	// 0x40 Border Color
	// 0x44 - 0x4c Cursor Palette Logical Colors
	if (reg >= 0x00 && reg <= 0x4c)
	{
		int r,g,b;

		//TODO: 8bpp mode uses a different formula
		//i = (val & 0x1000) >> 12; //supremacy bit
		b = (val & 0x0f00) >> 8;
		g = (val & 0x00f0) >> 4;
		r = (val & 0x000f) >> 0;

		palette_set_color_rgb(space->machine, reg >> 2, pal4bit(r), pal4bit(g), pal4bit(b) );
	}
	else if (reg >= 0x80 && reg <= 0xbc)
	{
		#ifdef DEBUG
		logerror("VIDC: %s = %d\n", vrnames[(reg-0x80)/4], val>>12);
		#endif

		if ((reg == 0xb0) & ((val>>12) != 0))
		{
			rectangle visarea;

			visarea.min_x = 0;
			visarea.min_y = 0;
			visarea.max_x = vidc_regs[0x94] - vidc_regs[0x88];
			visarea.max_y = vidc_regs[0xb4] - vidc_regs[0xa8];

			logerror("Configuring: htotal %d vtotal %d vis %d,%d\n",
				vidc_regs[0x80], vidc_regs[0xa0],
				visarea.max_x, visarea.max_y);

			space->machine->primary_screen->configure(vidc_regs[0x80], vidc_regs[0xa0], visarea, space->machine->primary_screen->frame_period().attoseconds);

			// slightly hacky: fire off a VBL right now.  the BIOS doesn't wait long enough otherwise.
			timer_adjust_oneshot(vbl_timer, attotime_zero, 0);
		}

		vidc_regs[reg] = val>>12;
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
			case 4:	/* sound start */
				vidc_sndstart = ((data>>2)&0x7fff)*16;
				break;

			case 5: /* sound end */
				vidc_sndend = ((data>>2)&0x7fff)*16;
				break;

			case 7:	/* Control */
				memc_pagesize = ((data>>2) & 3);

				logerror("MEMC: %x to Control (page size %d, %s, %s)\n", data & 0x1ffc, page_sizes[memc_pagesize], ((data>>10)&1) ? "Video DMA on" : "Video DMA off", ((data>>11)&1) ? "Sound DMA on" : "Sound DMA off");

				if ((data>>11)&1)
				{
					double sndhz;

					sndhz = 250000.0 / (double)((vidc_regs[0xc0]&0xff)+2);

					logerror("MEMC: Starting audio DMA at %f Hz, buffer from %x to %x\n", sndhz, vidc_sndstart, vidc_sndend);

					vidc_sndcur = vidc_sndstart;

					timer_adjust_periodic(snd_timer, ATTOTIME_IN_HZ(sndhz), 0, ATTOTIME_IN_HZ(sndhz));
				}
				else
				{
					timer_adjust_oneshot(snd_timer, attotime_never, 0);
					dac_signed_data_w(space->machine->device("dac"), 0x80);
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
	UINT32 log, phys, memc, perms;

	perms = (data & 0x300)>>8;
	log = phys = memc = 0;

	switch (memc_pagesize)
	{
		case 0:
			phys = data & 0x7f;
			log = (data & 0xc00)>>10;
			log <<= 23;
			log |= (data & 0x7ff000);
			memc = (data & 0x80) ? 1 : 0;
			break;

		case 1:
			phys = ((data & 0x7f) >> 1) | (data & 1) ? 0x40 : 0;
			log = (data & 0xc00)>>10;
			log <<= 23;
			log |= (data & 0x7fe000);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			break;

		case 2:
			phys = ((data & 0x7f) >> 2) | ((data & 3) << 5);
			log = (data & 0xc00)>>10;
			log <<= 23;
			log |= (data & 0x7fc000);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			break;

		case 3:
			phys = ((data & 0x7f) >> 3) | (data & 1)<<4 | (data & 2) << 5 | (data & 4)<<3;
			log = (data & 0xc00)>>10;
			log <<= 23;
			log |= (data & 0x7f8000);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			break;
	}

	log >>= (12 + memc_pagesize);

	// always make sure ROM mode is disconnected when this occurs
	memc_latchrom = 0;

	// now go ahead and set the mapping in the page table
	memc_pages[log] = phys * memc;

//  printf("MEMC_PAGE(%d): W %08x: log %x to phys %x, MEMC %d, perms %d\n", memc_pagesize, data, log, phys, memc, perms);
}

