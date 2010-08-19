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

#ifdef MESS
#include "machine/wd17xx.h"
#endif

static const int page_sizes[4] = { 4096, 8192, 16384, 32768 };

UINT32 *archimedes_memc_physmem;
static UINT32 memc_pagesize;
static int memc_latchrom;
static UINT32 ioc_timercnt[4], ioc_timerout[4];
static UINT32 vidc_vidstart, vidc_vidend, vidc_vidinit,vidc_vidcur;
static UINT32 vidc_sndstart, vidc_sndend, vidc_sndcur;
static UINT8 video_dma_on;
UINT8 i2c_clk;
INT16 memc_pages[(32*1024*1024)/(4096)];	// the logical RAM area is 32 megs, and the smallest page size is 4k
UINT32 vidc_regs[256];
UINT8 ioc_regs[0x80/4];
UINT8 vidc_bpp_mode;
UINT8 vidc_interlace;

static emu_timer *timer[4], *snd_timer, *vid_timer;
emu_timer  *vbl_timer;

void archimedes_request_irq_a(running_machine *machine, int mask)
{
	ioc_regs[IRQ_STATUS_A] |= mask;

	if (ioc_regs[IRQ_MASK_A] & mask)
	{
		cputag_set_input_line(machine, "maincpu", ARM_IRQ_LINE, ASSERT_LINE);
	}
}

void archimedes_request_irq_b(running_machine *machine, int mask)
{
	ioc_regs[IRQ_STATUS_B] |= mask;

	if (ioc_regs[IRQ_MASK_B] & mask)
	{
		generic_pulse_irq_line(machine->device("maincpu"), ARM_IRQ_LINE);
	}
}

void archimedes_request_fiq(running_machine *machine, int mask)
{
	ioc_regs[FIQ_STATUS] |= mask;

	if (ioc_regs[FIQ_MASK] & mask)
	{
		generic_pulse_irq_line(machine->device("maincpu"), ARM_FIRQ_LINE);
	}
}

void archimedes_clear_irq_a(running_machine *machine, int mask)
{
	ioc_regs[IRQ_STATUS_A] &= ~mask;
}

void archimedes_clear_irq_b(running_machine *machine, int mask)
{
	ioc_regs[IRQ_STATUS_B] &= ~mask;
}

void archimedes_clear_fiq(running_machine *machine, int mask)
{
	ioc_regs[FIQ_STATUS] &= ~mask;
}

static TIMER_CALLBACK( vidc_vblank )
{
	archimedes_request_irq_a(machine, ARCHIMEDES_IRQA_VBL);

	// set up for next vbl
	timer_adjust_oneshot(vbl_timer, machine->primary_screen->time_until_pos(vidc_regs[0xb4]), 0);
}

/* at about every ~4/4 USEC do a DMA transfer byte */
static TIMER_CALLBACK( vidc_video_tick )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);
	static UINT8 *vram = memory_region(machine,"vram");

	vram[vidc_vidcur] = (space->read_byte(vidc_vidstart+vidc_vidcur));

	vidc_vidcur++;

	if(video_dma_on)
	{
		if (vidc_vidcur >= vidc_vidend)
			vidc_vidcur = 0;

		timer_adjust_oneshot(vid_timer, ATTOTIME_IN_USEC(1), 0);
	}
	else
		timer_adjust_oneshot(vid_timer, attotime_never, 0);
}

static TIMER_CALLBACK( vidc_audio_tick )
{
	address_space *space = cputag_get_address_space(machine, "maincpu", ADDRESS_SPACE_PROGRAM);

	dac_signed_data_w(space->machine->device("dac"), (space->read_byte(vidc_sndcur)));

	vidc_sndcur++;

	if (vidc_sndcur >= vidc_sndend)
	{
		archimedes_request_irq_b(machine, ARCHIMEDES_IRQB_SOUND_EMPTY);

		/* TODO */
		timer_adjust_oneshot(snd_timer, attotime_never, 0);
		dac_signed_data_w(space->machine->device("dac"), 0x80);
	}
}

static void a310_set_timer(int tmr)
{
	double freq;

	if(ioc_timercnt[tmr] != 0) // FIXME: dmdtouch does a divide by zero?
	{
		freq = 2000000.0 / (double)ioc_timercnt[tmr];
//	  logerror("IOC: starting timer %d, %d ticks, freq %f Hz\n", tmr, ioc_timercnt[tmr], freq);
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

	ioc_regs[IRQ_STATUS_A] = 0x10 | 0x80; //set up POR (Power On Reset) and Force IRQ at start-up
	ioc_regs[IRQ_STATUS_B] = 0x02; //set up IL[1] On
	ioc_regs[FIQ_STATUS] = 0x80;   //set up Force FIQ
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

	vid_timer = timer_alloc(machine, vidc_video_tick, NULL);
	snd_timer = timer_alloc(machine, vidc_audio_tick, NULL);
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
			logerror("ARCHIMEDES_MEMC: Reading unmapped page %02x\n",page);
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
			logerror("ARCHIMEDES_MEMC: Writing unmapped page %02x, what do we do?\n",page);
		}
	}
}

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
		direct.explicit_configure(0x000000, 0x1fffff, 0x1fffff, *direct.space().m_machine.region("maincpu"));
	}
	else	// executing from logical memory
	{
		offs_t pagesize = page_sizes[memc_pagesize];
		UINT32 page = address / pagesize;
		
		direct.explicit_configure(page * pagesize, page * pagesize - 1, pagesize - 1, &archimedes_memc_physmem[(memc_pages[page] * pagesize)>>2]);
	}

	return ~0;
}

void archimedes_driver_init(running_machine *machine)
{
	address_space *space = machine->device<arm_device>("maincpu")->space(AS_PROGRAM);
	space->set_direct_update_handler(direct_update_delegate_create_static(a310_setopbase, *machine));
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

/* TODO: should be a 8-bit handler */
static READ32_HANDLER( ioc_ctrl_r )
{
	//if(((offset & 0x1f) != 16) && ((offset & 0x1f) != 17) && ((offset & 0x1f) != 24) && ((offset & 0x1f) != 25))
	//logerror("IOC: R %s = %02x (PC=%x) %02x\n", ioc_regnames[offset&0x1f], ioc_regs[offset&0x1f], cpu_get_pc( space->cpu ),offset & 0x1f);

	switch (offset & 0x1f)
	{
		case CONTROL:
		{
			UINT8 i2c_data;
			static UINT8 flyback;
			int vert_pos;

			vert_pos = space->machine->primary_screen->vpos();
			flyback = (vert_pos <= vidc_regs[VIDC_VDSR] || vert_pos >= vidc_regs[VIDC_VDER]) ? 0x80 : 0x00;

			i2c_data = (i2cmem_sda_read(space->machine->device("i2cmem")) & 1);

			return (flyback) | (ioc_regs[CONTROL] & 0x7c) | (i2c_clk<<1) | i2c_data;
		}

		case 1:	// keyboard read
			archimedes_request_irq_b(space->machine, ARCHIMEDES_IRQB_KBD_XMIT_EMPTY);
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

	return ioc_regs[offset&0x1f];
}

/* TODO: should be a 8-bit handler */
static WRITE32_HANDLER( ioc_ctrl_w )
{
	if(((offset & 0x1f) != 16) && ((offset & 0x1f) != 17) && ((offset & 0x1f) != 24) && ((offset & 0x1f) != 25))
     	if((offset & 0x1f) != 1)
     		logerror("IOC: W %02x @ reg %s (PC=%x)\n", data&0xff, ioc_regnames[offset&0x1f], cpu_get_pc( space->cpu ));

	switch (offset&0x1f)
	{
		case 0:	// I2C bus control
			//logerror("IOC I2C: CLK %d DAT %d\n", (data>>1)&1, data&1);
			i2cmem_sda_write(space->machine->device("i2cmem"), data & 0x01);
			i2cmem_scl_write(space->machine->device("i2cmem"), (data & 0x02) >> 1);
			i2c_clk = (data & 2) >> 1;
			break;

		case 1:
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
				archimedes_request_irq_a(space->machine,ARCHIMEDES_IRQA_FORCE);

			break;

		case FIQ_MASK:
			ioc_regs[FIQ_MASK] = data & 0xff;

			if(data & 0x80) //force a FIRQ
				archimedes_request_fiq(space->machine,ARCHIMEDES_FIQ_FORCE);

			break;

		case 5: 	// IRQ clear A
			ioc_regs[IRQ_STATUS_A] &= ~(data&0xff);

			// if that did it, clear the IRQ
			if (ioc_regs[IRQ_STATUS_A] == 0)
			{
				printf("IRQ clear A\n");
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

READ32_HANDLER(archimedes_ioc_r)
{
	UINT32 ioc_addr;
	#ifdef MESS
	running_device *fdc = (running_device *)space->machine->device("wd1772");
	#endif

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
					#ifdef MESS
						logerror("17XX: R @ addr %x mask %08x\n", offset*4, mem_mask);
						return wd17xx_data_r(fdc, offset&0xf);
					#else
						logerror("Read from FDC device?\n");
						return 0;
					#endif
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
	#ifdef MESS
	running_device *fdc = (running_device *)space->machine->device("wd1772");
	#endif

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
					#ifdef MESS
						logerror("17XX: %x to addr %x mask %08x\n", data, offset*4, mem_mask);
						wd17xx_data_w(fdc, offset&0xf, data&0xff);
					#else
						logerror("Write to FDC device?\n");
					#endif
						return;
				case 2:
					logerror("IOC: Econet Write %02x at %08x\n",data,ioc_addr);
					return;
				case 3:
					logerror("IOC: Serial Write\n");
					return;
				case 4:
					logerror("IOC: Internal Podule Write\n");
					return;
				case 5:
					switch(ioc_addr & 0xfffc)
					{
						#ifdef MESS
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
						#endif
					}
					break;
			}
		}
	}


	logerror("I/O: W %x @ %x (mask %08x)\n", data, (offset*4)+0x3000000, mem_mask);
}

READ32_HANDLER(archimedes_vidc_r)
{
	return 0;
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
			logerror("WARNING: border color write here (PC=%08x)!\n",cpu_get_pc(space->cpu));

		palette_set_color_rgb(space->machine, reg >> 2, pal4bit(r), pal4bit(g), pal4bit(b) );

		/* handle 8bpp colors here */
		if(reg <= 0x3c)
		{
			int i;

			for(i=0;i<0x100;i+=0x10)
			{
				b = ((val & 0x700) >> 8) | ((i & 0x80) >> 4);
				g = ((val & 0x030) >> 4) | ((i & 0x20) >> 3) | ((i & 0x40) >> 3);
				r = ((val & 0x007) >> 0) | ((i & 0x10) >> 1);

				palette_set_color_rgb(space->machine, (reg >> 2) + 0x100 + i, pal4bit(r), pal4bit(g), pal4bit(b) );
			}
		}

	}
	else if (reg >= 0x80 && reg <= 0xbc)
	{
		switch(reg)
		{
			case VIDC_HCR:  vidc_regs[VIDC_HCR] =  ((val >> 14)<<1)+1; 	break;
//			case VIDC_HSWR: vidc_regs[VIDC_HSWR] = (val >> 14)+1; 	break;
			case VIDC_HBSR: vidc_regs[VIDC_HBSR] = (val >> 14)+1; 	break;
			case VIDC_HDSR: vidc_regs[VIDC_HDSR] = (val >> 14); 	break;
			case VIDC_HDER: vidc_regs[VIDC_HDER] = (val >> 14); 	break;
			case VIDC_HBER: vidc_regs[VIDC_HBER] = (val >> 14)+1; 	break;
//			#define VIDC_HCSR		0x98
//			#define VIDC_HIR		0x9c

			case VIDC_VCR:  vidc_regs[VIDC_VCR] = ((val >> 14)<<1)+1;	break;
//			#define VIDC_VSWR		0xa4
			case VIDC_VBSR: vidc_regs[VIDC_VBSR] = (val >> 14)+1; 	break;
			case VIDC_VDSR: vidc_regs[VIDC_VDSR] = (val >> 14)+1;	break;
			case VIDC_VDER: vidc_regs[VIDC_VDER] = (val >> 14)+1;	break;
			case VIDC_VBER: vidc_regs[VIDC_VBER] = (val >> 14)+1;	break;
//			#define VIDC_VCSR		0xb8
//			#define VIDC_VCER		0xbc
		}


		//#ifdef DEBUG
		logerror("VIDC: %s = %d\n", vrnames[(reg-0x80)/4], vidc_regs[reg]);
		//#endif

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

				visarea.min_x = 0;
				visarea.min_y = 0;
				visarea.max_x = vidc_regs[VIDC_HBER] - vidc_regs[VIDC_HBSR] - 1;
				visarea.max_y = vidc_regs[VIDC_VBER] - vidc_regs[VIDC_VBSR];

				//printf("Configuring: htotal %d vtotal %d border %d x %d display %d x %d\n",
				//	vidc_regs[VIDC_HCR], vidc_regs[VIDC_VCR],
				//	visarea.max_x, visarea.max_y,
				//	vidc_regs[VIDC_HDER]-vidc_regs[VIDC_HDSR],vidc_regs[VIDC_VDER]-vidc_regs[VIDC_VDSR]+1);

				space->machine->primary_screen->configure(vidc_regs[VIDC_HCR], vidc_regs[VIDC_VCR], visarea, space->machine->primary_screen->frame_period().attoseconds);
			}
		}

	}
	else if(reg == 0xe0)
	{
		vidc_bpp_mode = ((val & 0xc) >> 2);
		vidc_interlace = ((val & 0x40) >> 6);
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
				break;

			case 1: /* video start */
				vidc_vidstart = ((data>>2)&0x7fff)*16;
				break;

			case 2: /* video end */
				vidc_vidend = ((data>>2)&0x7fff)*16;
				break;

			case 4:	/* sound start */
				vidc_sndstart = ((data>>2)&0x7fff)*16;
				break;

			case 5: /* sound end */
				vidc_sndend = ((data>>2)&0x7fff)*16;
				break;

			case 7:	/* Control */
				memc_pagesize = ((data>>2) & 3);

				logerror("MEMC: %x to Control (page size %d, %s, %s)\n", data & 0x1ffc, page_sizes[memc_pagesize], ((data>>10)&1) ? "Video DMA on" : "Video DMA off", ((data>>11)&1) ? "Sound DMA on" : "Sound DMA off");

				video_dma_on = ((data>>10)&1);

				if ((data>>10)&1)
				{
					vidc_vidcur = 0;
					timer_adjust_oneshot(vid_timer, ATTOTIME_IN_USEC(1), 0);
				}

				if ((data>>11)&1)
				{
					double sndhz;

					/* FIXME: is the frequency correct? */
					sndhz = (250000.0) / (double)((vidc_regs[0xc0]&0xff)+2);

					logerror("MEMC: Starting audio DMA at %f Hz, buffer from %x to %x\n", sndhz, vidc_sndstart, vidc_sndend);

					vidc_sndcur = vidc_sndstart;

					timer_adjust_periodic(snd_timer, ATTOTIME_IN_HZ(sndhz), 0, ATTOTIME_IN_HZ(sndhz));
				}
				else
				{
					//timer_adjust_oneshot(snd_timer, attotime_never, 0);
					//dac_signed_data_w(space->machine->device("dac"), 0x80);
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
			log = ((data & 0x7ff000)>>12) | (data & 0xc00);
			memc = (data & 0x80) ? 1 : 0;
			break;

		case 1:
			phys = ((data & 0x7f) >> 1) | ((data & 1) << 6);
			log = ((data & 0x7fe000)>>13) | (data & 0xc00);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			break;

		case 2:
			phys = ((data & 0x7f) >> 2) | ((data & 3) << 5);
			log = ((data & 0x7fc000)>>14) | (data & 0xc00);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			break;

		case 3:
			phys = ((data & 0x7f) >> 3) | ((data & 1)<<4) | ((data & 2) << 5) | ((data & 4)<<3);
			log = ((data & 0x7f8000)>>15) | (data & 0xc00);
			memc = ((data & 0x80) ? 1 : 0) | ((data & 0x1000) ? 2 : 0);
			//printf("Mapping %08X to %08X\n",0x2000000+(phys*32768),(((data >> 15)&0xff)|((data >> 2)&0x300)));
			break;
	}

//	log >>= (12 + memc_pagesize);

	// always make sure ROM mode is disconnected when this occurs
	memc_latchrom = 0;

	// now go ahead and set the mapping in the page table
	memc_pages[log] = phys + (memc*0x80);

//  printf("MEMC_PAGE(%d): W %08x: log %x to phys %x, MEMC %d, perms %d\n", memc_pagesize, data, log, phys, memc, perms);
}

