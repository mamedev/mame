// license:LGPL-2.1+
// copyright-holders:Angelo Salese, R. Belmont, Juergen Buchmueller
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
#include "includes/archimds.h"
#include "debugger.h"

static const int page_sizes[4] = { 4096, 8192, 16384, 32768 };
static const UINT32 pixel_rate[4] = { 8000000, 12000000, 16000000, 24000000};

#define IOC_LOG 0

/* TODO: fix pending irqs */
void archimedes_state::archimedes_request_irq_a(int mask)
{
	m_ioc_regs[IRQ_STATUS_A] |= mask;

	if (m_ioc_regs[IRQ_STATUS_A] & m_ioc_regs[IRQ_MASK_A])
	{
		m_maincpu->set_input_line(ARM_IRQ_LINE, ASSERT_LINE);
	}

	if ((m_ioc_regs[IRQ_STATUS_A] & m_ioc_regs[IRQ_MASK_A]) == 0)
	{
		m_maincpu->set_input_line(ARM_IRQ_LINE, CLEAR_LINE);
	}
}

void archimedes_state::archimedes_request_irq_b(int mask)
{
	m_ioc_regs[IRQ_STATUS_B] |= mask;

	if (m_ioc_regs[IRQ_STATUS_B] & m_ioc_regs[IRQ_MASK_B])
	{
		generic_pulse_irq_line(m_maincpu, ARM_IRQ_LINE, 1);
		//m_maincpu->set_input_line(ARM_IRQ_LINE, CLEAR_LINE);
		//m_maincpu->set_input_line(ARM_IRQ_LINE, ASSERT_LINE);
	}
}

void archimedes_state::archimedes_request_fiq(int mask)
{
	m_ioc_regs[FIQ_STATUS] |= mask;

	//printf("STATUS:%02x IRQ:%02x MASK:%02x\n",m_ioc_regs[FIQ_STATUS],mask,m_ioc_regs[FIQ_MASK]);

	if (m_ioc_regs[FIQ_STATUS] & m_ioc_regs[FIQ_MASK])
	{
		generic_pulse_irq_line(m_maincpu, ARM_FIRQ_LINE, 1);

		//m_maincpu->set_input_line(ARM_FIRQ_LINE, CLEAR_LINE);
		//m_maincpu->set_input_line(ARM_FIRQ_LINE, ASSERT_LINE);
	}
}

void archimedes_state::archimedes_clear_irq_a(int mask)
{
	m_ioc_regs[IRQ_STATUS_A] &= ~mask;
	archimedes_request_irq_a(0);
}

void archimedes_state::archimedes_clear_irq_b(int mask)
{
	m_ioc_regs[IRQ_STATUS_B] &= ~mask;
	//archimedes_request_irq_b(0);
}

void archimedes_state::archimedes_clear_fiq(int mask)
{
	m_ioc_regs[FIQ_STATUS] &= ~mask;
	//archimedes_request_fiq(0);
}

void archimedes_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_VBLANK: vidc_vblank();break;
		case TIMER_VIDEO: vidc_video_tick(); break;
		case TIMER_AUDIO: vidc_audio_tick(); break;
		case TIMER_IOC: ioc_timer(param); break;
	}
}


void archimedes_state::vidc_vblank()
{
	archimedes_request_irq_a(ARCHIMEDES_IRQA_VBL);

	// set up for next vbl
	m_vbl_timer->adjust(m_screen->time_until_pos(m_vidc_regs[0xb4]));
}

/* video DMA */
/* TODO: what type of DMA this is, burst or cycle steal? Docs doesn't explain it (4 usec is the DRAM refresh). */
/* TODO: change m_region_vram into proper alloc array */
/* TODO: Erotictac and Poizone sets up vidinit register AFTER vidend, for double buffering? (fixes Poizone "Eterna" logo display on attract) */
void archimedes_state::vidc_video_tick()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	static UINT8 *vram = m_region_vram->base();
	UINT32 size;
	UINT32 m_vidc_ccur;
	UINT32 offset_ptr;

	size = m_vidc_vidend-m_vidc_vidstart+0x10;

	offset_ptr = m_vidc_vidstart+m_vidc_vidinit;
	if(offset_ptr >= m_vidc_vidend+0x10) // TODO: correct?
		offset_ptr = m_vidc_vidstart;

	//popmessage("%08x %08x %08x",m_vidc_vidstart,m_vidc_vidinit,m_vidc_vidend);

	for(m_vidc_vidcur = 0;m_vidc_vidcur < size;m_vidc_vidcur++)
	{
		vram[m_vidc_vidcur] = (space.read_byte(offset_ptr));
		offset_ptr++;
		if(offset_ptr >= m_vidc_vidend+0x10) // TODO: correct?
			offset_ptr = m_vidc_vidstart;
	}

	if(m_cursor_enabled == true)
	{
		for(m_vidc_ccur = 0;m_vidc_ccur < 0x200;m_vidc_ccur++)
			m_cursor_vram[m_vidc_ccur] = (space.read_byte(m_vidc_cinit+m_vidc_ccur));
	}

	if(m_video_dma_on)
	{
		m_vid_timer->adjust(m_screen->time_until_pos(m_vidc_regs[0xb4]));
	}
	else
		m_vid_timer->adjust(attotime::never);
}

/* audio DMA */
void archimedes_state::vidc_audio_tick()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 ulaw_comp;
	INT16 res;
	UINT8 ch;
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
		UINT8 ulaw_temp = (space.read_byte(m_vidc_sndstart+m_vidc_sndcur + ch)) ^ 0xff;

		ulaw_comp = (ulaw_temp>>1) | ((ulaw_temp&1)<<7);

		res = mulawTable[ulaw_comp];

		m_dac[ch & 7]->write_signed16(res^0x8000);
	}

	m_vidc_sndcur+=8;

	if (m_vidc_sndcur >= (m_vidc_sndend-m_vidc_sndstart)+0x10)
	{
		m_vidc_sndcur = 0;
		archimedes_request_irq_b(ARCHIMEDES_IRQB_SOUND_EMPTY);

		if(!m_audio_dma_on)
		{
			m_snd_timer->adjust(attotime::never);
			for(ch=0;ch<8;ch++)
			{
				m_dac[ch & 7]->write_signed16(0x8000);
			}
		}
	}
}

void archimedes_state::a310_set_timer(int tmr)
{
	double freq;

	switch(tmr)
	{
		case 0:
		case 1:
			m_timer[tmr]->adjust(attotime::from_usec(m_ioc_timercnt[tmr]/2), tmr); // TODO: ARM timings are quite off there, it should be latch and not latch/2
			break;
		case 2:
			freq = 1000000.0 / (double)(m_ioc_timercnt[tmr]+1);
			m_timer[tmr]->adjust(attotime::from_hz(freq), tmr);
			break;
		case 3:
			freq = 1000000.0 / (double)((m_ioc_timercnt[tmr]+1)*16);
			m_timer[tmr]->adjust(attotime::from_hz(freq), tmr);
			break;
	}
}

// param
void archimedes_state::ioc_timer(int param)
{
	// all timers always run
	a310_set_timer(param);

	// but only timers 0 and 1 generate IRQs
	switch (param)
	{
		case 0:
			archimedes_request_irq_a(ARCHIMEDES_IRQA_TIMER0);
			break;

		case 1:
			archimedes_request_irq_a(ARCHIMEDES_IRQA_TIMER1);
			break;
	}
}

void archimedes_state::archimedes_reset()
{
	int i;

	m_memc_latchrom = 1;            // map in the boot ROM

	// kill all memc mappings
	for (i = 0; i < (32*1024*1024)/(4096); i++)
	{
		m_memc_pages[i] = -1;       // indicate unmapped
	}

	m_ioc_regs[IRQ_STATUS_A] = 0x10 | 0x80; //set up POR (Power On Reset) and Force IRQ at start-up
	m_ioc_regs[IRQ_STATUS_B] = 0x00; //set up IL[1] On
	m_ioc_regs[FIQ_STATUS] = 0x80;   //set up Force FIQ
	m_ioc_regs[CONTROL] = 0xff;
}

void archimedes_state::archimedes_init()
{
	static const char *const dac_names[8] = { "dac0", "dac1", "dac2", "dac3", "dac4", "dac5", "dac6", "dac7" };

	m_memc_pagesize = 0;

	m_vbl_timer = timer_alloc(TIMER_VBLANK);
	m_vbl_timer->adjust(attotime::never);

	m_timer[0] = timer_alloc(TIMER_IOC);
	m_timer[1] = timer_alloc(TIMER_IOC);
	m_timer[2] = timer_alloc(TIMER_IOC);
	m_timer[3] = timer_alloc(TIMER_IOC);
	m_timer[0]->adjust(attotime::never);
	m_timer[1]->adjust(attotime::never);
	m_timer[2]->adjust(attotime::never);
	m_timer[3]->adjust(attotime::never);

	m_vid_timer = timer_alloc(TIMER_VIDEO);
	m_snd_timer = timer_alloc(TIMER_AUDIO);
	m_snd_timer->adjust(attotime::never);

	for ( int i = 0; i < 8; i++ )
	{
		m_dac[i] = machine().device<dac_device>(dac_names[i]);
	}
}

READ32_MEMBER(archimedes_state::archimedes_memc_logical_r)
{
	UINT32 page, poffs;

	// are we mapping in the boot ROM?
	if (m_memc_latchrom)
	{
		UINT32 *rom;

		rom = (UINT32 *)m_region_maincpu->base();

		return rom[offset & 0x1fffff];
	}
	else
	{
		// figure out the page number and offset in the page
		page = (offset<<2) / page_sizes[m_memc_pagesize];
		poffs = (offset<<2) % page_sizes[m_memc_pagesize];

//      printf("Reading offset %x (addr %x): page %x (size %d %d) offset %x ==> %x %x\n", offset, offset<<2, page, memc_pagesize, page_sizes[memc_pagesize], poffs, memc_pages[page], memc_pages[page]*page_sizes[memc_pagesize]);

		if (m_memc_pages[page] != -1)
		{
			return m_archimedes_memc_physmem[((m_memc_pages[page] * page_sizes[m_memc_pagesize]) + poffs)>>2];
		}
		else
		{
			//printf("ARCHIMEDES_MEMC: Reading unmapped page %02x\n",page);
			return 0xdeadbeef;
		}
	}

	// never executed
	//return 0;
}



WRITE32_MEMBER(archimedes_state::archimedes_memc_logical_w)
{
	UINT32 page, poffs;

	// if the boot ROM is mapped, ignore writes
	if (m_memc_latchrom)
	{
		return;
	}
	else
	{
		// figure out the page number and offset in the page
		page = (offset<<2) / page_sizes[m_memc_pagesize];
		poffs = (offset<<2) % page_sizes[m_memc_pagesize];

//      printf("Writing offset %x (addr %x): page %x (size %d %d) offset %x ==> %x %x\n", offset, offset<<2, page, memc_pagesize, page_sizes[memc_pagesize], poffs, memc_pages[page], memc_pages[page]*page_sizes[memc_pagesize]);

		if (m_memc_pages[page] != -1)
		{
			COMBINE_DATA(&m_archimedes_memc_physmem[((m_memc_pages[page] * page_sizes[m_memc_pagesize]) + poffs)>>2]);
		}
		else
		{
			//printf("ARCHIMEDES_MEMC: Writing unmapped page %02x, what do we do?\n",page);
		}
	}
}

/* Aristocrat Mark 5 - same as normal AA except with Dram emulator */
READ32_MEMBER(archimedes_state::aristmk5_drame_memc_logical_r)
{
	UINT32 page, poffs;

	// are we mapping in the boot ROM?
	if (m_memc_latchrom)
	{
		UINT32 *rom;

		rom = (UINT32 *)m_region_maincpu->base();

		return rom[offset & 0x1fffff];
	}
	else
	{
		// figure out the page number and offset in the page
		page = (offset<<2) / page_sizes[m_memc_pagesize];
		poffs = (offset<<2) % page_sizes[m_memc_pagesize];



		if (m_memc_pages[page] != -1)
		{
			/******************* DRAM Emulator - gal20v - Aristocrat Mark 5 ************************
			A Dynamic RAM emulator is provided which avoids the need to execute code
			in DRAM in those regulatory environments where it is not needed.

			When pin 5 of U36 ( gal20v ) is low, the pin 25 output is high and enables the
			logic buffer inputs and provides a fixed jmp address to a plurality
			of rom addresses ( 0xEAD0000A  shown on logic buffer arrangement in schematics )

			In this state, DRAM memory space is disabled.

			****************************************************************************************/
			if(!(m_memc_pages[page] & 0x10)  && (offset <= 0x3ff))
				return 0xEAD0000A;
			return m_archimedes_memc_physmem[((m_memc_pages[page] * page_sizes[m_memc_pagesize]) + poffs)>>2];
		}
		else
		{
			//printf("ARCHIMEDES_MEMC: Reading unmapped page %02x\n",page);
			return 0xdeadbeef;
		}
	}

	// never executed
	//return 0;
}

void archimedes_state::archimedes_driver_init()
{
	m_archimedes_memc_physmem = reinterpret_cast<UINT32 *>(memshare("physicalram")->ptr());
//  address_space &space = m_maincpu->space(AS_PROGRAM);
//  space.set_direct_update_handler(direct_update_delegate(FUNC(a310_setopbase), &machine));
}

static const char *const ioc_regnames[] =
{
	"(rw) Control",                 // 0
	"(read) Keyboard receive (write) keyboard send",    // 4
	"?",
	"?",
	"(read) IRQ status A",              // 10
	"(read) IRQ request A (write) IRQ clear",   // 14
	"(rw) IRQ mask A",              // 18
	"?",
	"(read) IRQ status B",      // 20
	"(read) IRQ request B",     // 24
	"(rw) IRQ mask B",      // 28
	"?",
	"(read) FIQ status",        // 30
	"(read) FIQ request",       // 34
	"(rw) FIQ mask",        // 38
	"?",
	"(read) Timer 0 count low (write) Timer 0 latch low",       // 40
	"(read) Timer 0 count high (write) Timer 0 latch high",     // 44
	"(write) Timer 0 go command",                   // 48
	"(write) Timer 0 latch command",                // 4c
	"(read) Timer 1 count low (write) Timer 1 latch low",       // 50
	"(read) Timer 1 count high (write) Timer 1 latch high",     // 54
	"(write) Timer 1 go command",                   // 58
	"(write) Timer 1 latch command",                // 5c
	"(read) Timer 2 count low (write) Timer 2 latch low",       // 60
	"(read) Timer 2 count high (write) Timer 2 latch high",     // 64
	"(write) Timer 2 go command",                   // 68
	"(write) Timer 2 latch command",                // 6c
	"(read) Timer 3 count low (write) Timer 3 latch low",       // 70
	"(read) Timer 3 count high (write) Timer 3 latch high",     // 74
	"(write) Timer 3 go command",                   // 78
	"(write) Timer 3 latch command"                 // 7c
};

void archimedes_state::latch_timer_cnt(int tmr)
{
	double time = m_timer[tmr]->elapsed().as_double();
	time *= 2000000.0;  // find out how many 2 MHz ticks have gone by
	m_ioc_timerout[tmr] = m_ioc_timercnt[tmr] - (UINT32)time;
}

bool archimedes_state::check_floppy_ready()
{
	floppy_image_device *floppy = NULL;

	if(!m_fdc)
		return false;

	switch(m_floppy_select & 3)
	{
		case 0:
			floppy = m_floppy0->get_device(); break;
		case 1:
			floppy = m_floppy1->get_device(); break;
	}

	if(floppy)
		return floppy->ready_r();

	return false;
}

/* TODO: should be a 8-bit handler */
READ32_MEMBER( archimedes_state::ioc_ctrl_r )
{
	if(IOC_LOG)
	logerror("IOC: R %s = %02x (PC=%x) %02x\n", ioc_regnames[offset&0x1f], m_ioc_regs[offset&0x1f], m_maincpu->pc(),offset & 0x1f);

	switch (offset & 0x1f)
	{
		case CONTROL:
		{
			UINT8 i2c_data = 1;
			UINT8 flyback; //internal name for vblank here
			int vert_pos;
			bool floppy_ready_state;

			vert_pos = m_screen->vpos();
			flyback = (vert_pos <= m_vidc_regs[VIDC_VDSR] || vert_pos >= m_vidc_regs[VIDC_VDER]) ? 0x80 : 0x00;

			if ( m_i2cmem )
			{
				i2c_data = (m_i2cmem->read_sda() & 1);
			}

			floppy_ready_state = check_floppy_ready();

			return (flyback) | (m_ioc_regs[CONTROL] & 0x78) | (floppy_ready_state<<2) | (m_i2c_clk<<1) | i2c_data;
		}

		case KART:  // keyboard read
			return m_kart->read(space,0);

		case IRQ_STATUS_A:
			return (m_ioc_regs[IRQ_STATUS_A] & 0x7f) | 0x80; // Force IRQ is always '1'

		case IRQ_REQUEST_A:
			return (m_ioc_regs[IRQ_STATUS_A] & m_ioc_regs[IRQ_MASK_A]);

		case IRQ_MASK_A:
			return (m_ioc_regs[IRQ_MASK_A]);

		case IRQ_STATUS_B:
			return (m_ioc_regs[IRQ_STATUS_B]);

		case IRQ_REQUEST_B:
			return (m_ioc_regs[IRQ_STATUS_B] & m_ioc_regs[IRQ_MASK_B]);

		case IRQ_MASK_B:
			return (m_ioc_regs[IRQ_MASK_B]);

		case FIQ_STATUS:
			return (m_ioc_regs[FIQ_STATUS] & 0x7f) | 0x80; // Force FIQ is always '1'

		case FIQ_REQUEST:
			return (m_ioc_regs[FIQ_STATUS] & m_ioc_regs[FIQ_MASK]);

		case FIQ_MASK:
			return (m_ioc_regs[FIQ_MASK]);

		case T0_LATCH_LO: return m_ioc_timerout[0]&0xff;
		case T0_LATCH_HI: return (m_ioc_timerout[0]>>8)&0xff;

		case T1_LATCH_LO: return m_ioc_timerout[1]&0xff;
		case T1_LATCH_HI: return (m_ioc_timerout[1]>>8)&0xff;

		case T2_LATCH_LO: return m_ioc_timerout[2]&0xff;
		case T2_LATCH_HI: return (m_ioc_timerout[2]>>8)&0xff;

		case T3_LATCH_LO: return m_ioc_timerout[3]&0xff;
		case T3_LATCH_HI: return (m_ioc_timerout[3]>>8)&0xff;
		default:
			if(!IOC_LOG)
				logerror("IOC: R %s = %02x (PC=%x) %02x\n", ioc_regnames[offset&0x1f], m_ioc_regs[offset&0x1f], space.device() .safe_pc( ),offset & 0x1f);
			break;
	}

	return m_ioc_regs[offset&0x1f];
}

/* TODO: should be a 8-bit handler */
WRITE32_MEMBER( archimedes_state::ioc_ctrl_w )
{
	if(IOC_LOG)
	logerror("IOC: W %02x @ reg %s (PC=%x)\n", data&0xff, ioc_regnames[offset&0x1f], space.device() .safe_pc( ));

	switch (offset&0x1f)
	{
		case CONTROL:   // I2C bus control
			//logerror("IOC I2C: CLK %d DAT %d\n", (data>>1)&1, data&1);
			if ( m_i2cmem )
			{
				m_i2cmem->write_sda(data & 0x01);
				m_i2cmem->write_scl((data & 0x02) >> 1);
			}
			m_i2c_clk = (data & 2) >> 1;
			//TODO: does writing bit 2 here causes a fdc force ready?
			/*
			-x-- ---- Printer ack
			--x- ---- Sound mute
			---x ---- Aux I/O connector
			---- -x-- Floppy ready
			---- --x- I2C clock
			---- ---x I2C data
			*/
			//if(data & 0x40)
			//  popmessage("Muting sound, contact MAME/MESSdev");
			break;

		case KART:
			m_kart->write(space,0,data);
			break;

		case IRQ_MASK_A:
			m_ioc_regs[IRQ_MASK_A] = data & 0xff;

			/* bit 7 forces an IRQ trap */
			archimedes_request_irq_a((data & 0x80) ? ARCHIMEDES_IRQA_FORCE : 0);

			if(data & 0x08) //set up the VBLANK timer
				m_vbl_timer->adjust(m_screen->time_until_pos(m_vidc_regs[0xb4]));

			break;

		case IRQ_MASK_B:
			m_ioc_regs[IRQ_MASK_B] = data & 0xff;

			archimedes_request_irq_b(0);
			break;

		case FIQ_MASK:
			m_ioc_regs[FIQ_MASK] = data & 0xff;

			/* bit 7 forces a FIRQ trap */
			archimedes_request_fiq((data & 0x80) ? ARCHIMEDES_FIQ_FORCE : 0);
			break;

		case IRQ_REQUEST_A:     // IRQ clear A
			m_ioc_regs[IRQ_STATUS_A] &= ~(data&0xff);

			// check pending irqs
			archimedes_request_irq_a(0);
			break;

		case T0_LATCH_LO:
		case T0_LATCH_HI:
			m_ioc_regs[offset&0x1f] = data & 0xff;
			break;

		case T1_LATCH_LO:
		case T1_LATCH_HI:
			m_ioc_regs[offset&0x1f] = data & 0xff;
			break;

		case T2_LATCH_LO:
		case T2_LATCH_HI:
			m_ioc_regs[offset&0x1f] = data & 0xff;
			break;

		case T3_LATCH_LO:
		case T3_LATCH_HI:
			m_ioc_regs[offset&0x1f] = data & 0xff;
			break;

		case T0_LATCH:  // Timer 0 latch
			latch_timer_cnt(0);
			break;

		case T1_LATCH:  // Timer 1 latch
			latch_timer_cnt(1);
			break;

		case T2_LATCH:  // Timer 2 latch
			latch_timer_cnt(2);
			break;

		case T3_LATCH:  // Timer 3 latch
			latch_timer_cnt(3);
			break;

		case T0_GO: // Timer 0 start
			m_ioc_timercnt[0] = m_ioc_regs[T0_LATCH_HI]<<8 | m_ioc_regs[T0_LATCH_LO];
			a310_set_timer(0);
			break;

		case T1_GO: // Timer 1 start
			m_ioc_timercnt[1] = m_ioc_regs[T1_LATCH_HI]<<8 | m_ioc_regs[T1_LATCH_LO];
			a310_set_timer(1);
			break;

		case T2_GO: // Timer 2 start
			m_ioc_timercnt[2] = m_ioc_regs[T2_LATCH_HI]<<8 | m_ioc_regs[T2_LATCH_LO];
			a310_set_timer(2);
			break;

		case T3_GO: // Timer 3 start
			m_ioc_timercnt[3] = m_ioc_regs[T3_LATCH_HI]<<8 | m_ioc_regs[T3_LATCH_LO];
			a310_set_timer(3);
			break;

		default:
			if(!IOC_LOG)
				logerror("IOC: W %02x @ reg %s (PC=%x)\n", data&0xff, ioc_regnames[offset&0x1f], space.device() .safe_pc( ));

			m_ioc_regs[offset&0x1f] = data & 0xff;
			break;
	}
}

READ32_MEMBER(archimedes_state::archimedes_ioc_r)
{
	UINT32 ioc_addr;

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
					if (m_fdc)
					{
						//printf("17XX: R @ addr %x mask %08x\n", offset*4, mem_mask);
						switch(ioc_addr & 0xc)
						{
							case 0x00: return m_fdc->status_r();
							case 0x04: return m_fdc->track_r();
							case 0x08: return m_fdc->sector_r();
							case 0x0c: return m_fdc->data_r();
						}

						return 0;
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
					if (m_fdc)
					{
						switch(ioc_addr & 0xfffc)
						{
							case 0x18: return 0xff; // FDC latch B
							case 0x40: return 0xff; // FDC latch A
							case 0x50: return 0; //fdc type, new model returns 5 here
							case 0x70: return 0x0F;
							case 0x74: return 0xFF; // unknown
//                          case 0x78: /* joystick */
//                          case 0x7c:
						}
					}

					//printf("IOC: Internal Latches Read %08x\n",ioc_addr);

					return 0xffff;
			}
		}
	}

	logerror("IOC: Unknown read at %08x\n",ioc_addr);

	return 0;
}

WRITE32_MEMBER(archimedes_state::archimedes_ioc_w)
{
	UINT32 ioc_addr;

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
						if (m_fdc)
						{
							//printf("17XX: %x to addr %x mask %08x\n", data, offset*4, mem_mask);
							switch(ioc_addr & 0xc)
							{
								case 0x00:
									m_fdc->cmd_w(data);
									return;

								case 0x04:
									m_fdc->track_w(data);
									return;

								case 0x08:
									m_fdc->sector_w(data);
									return;

									case 0x0c:
									m_fdc->data_w(data);
									return;
							}
						}
						else
						{
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
					if (m_fdc)
					{
						switch(ioc_addr & 0xfffc)
						{
							case 0x18: // latch B
								/*
								---- x--- floppy controller reset
								*/
								m_fdc->dden_w(BIT(data, 1));
								if(data & 8)
									m_fdc->soft_reset();
								if(data & ~0xa)
									printf("%02x Latch B\n",data);
								return;

							case 0x40: // latch A
								/*
								-x-- ---- In Use Control (floppy?)
								*/
								floppy_image_device *floppy = NULL;

								if (!(data & 1)) { m_floppy_select = 0; floppy = m_floppy0->get_device(); }
								if (!(data & 2)) { m_floppy_select = 1; floppy = m_floppy1->get_device(); }
								if (!(data & 4)) { m_floppy_select = 2; floppy = NULL; } // floppy 2
								if (!(data & 8)) { m_floppy_select = 3; floppy = NULL; } // floppy 3

								m_fdc->set_floppy(floppy);

								if(floppy)
								{
									floppy->mon_w(BIT(data, 5));
									floppy->ss_w(!(BIT(data, 4)));
								}
								//bit 5 is motor on
								return;
						}

						//printf("%08x\n",ioc_addr);
					}
					break;
			}
		}
	}


	logerror("(PC=%08x) I/O: W %x @ %x (mask %08x)\n", space.device().safe_pc(), data, (offset*4)+0x3000000, mem_mask);
}

READ32_MEMBER(archimedes_state::archimedes_vidc_r)
{
	return 0;
}

void archimedes_state::vidc_dynamic_res_change()
{
	/* sanity checks - first pass */
	/*
	    total cycles + border end
	*/
	if(m_vidc_regs[VIDC_HCR] && m_vidc_regs[VIDC_HBER] &&
		m_vidc_regs[VIDC_VCR] && m_vidc_regs[VIDC_VBER])
	{
		/* sanity checks - second pass */
		/*
		total cycles >= border end >= border start
		*/
		if((m_vidc_regs[VIDC_HCR] >= m_vidc_regs[VIDC_HBER]) &&
			(m_vidc_regs[VIDC_HBER] >= m_vidc_regs[VIDC_HBSR]) &&
			(m_vidc_regs[VIDC_VCR] >= m_vidc_regs[VIDC_VBER]) &&
			(m_vidc_regs[VIDC_VBER] >= m_vidc_regs[VIDC_VBSR]))
		{
			rectangle visarea;
			attoseconds_t refresh;

			visarea.min_x = 0;
			visarea.min_y = 0;
			visarea.max_x = m_vidc_regs[VIDC_HBER] - m_vidc_regs[VIDC_HBSR] - 1;
			visarea.max_y = (m_vidc_regs[VIDC_VBER] - m_vidc_regs[VIDC_VBSR]) * (m_vidc_interlace+1);

			//logerror("Configuring: htotal %d vtotal %d border %d x %d display %d x %d\n",
			//  m_vidc_regs[VIDC_HCR], m_vidc_regs[VIDC_VCR],
			//  visarea.max_x, visarea.max_y,
			//  m_vidc_regs[VIDC_HDER]-m_vidc_regs[VIDC_HDSR],m_vidc_regs[VIDC_VDER]-m_vidc_regs[VIDC_VDSR]+1);

			/* FIXME: pixel clock */
			refresh = HZ_TO_ATTOSECONDS(pixel_rate[m_vidc_pixel_clk]*2) * m_vidc_regs[VIDC_HCR] * m_vidc_regs[VIDC_VCR];

			m_screen->configure(m_vidc_regs[VIDC_HCR], m_vidc_regs[VIDC_VCR] * (m_vidc_interlace+1), visarea, refresh);
		}
	}
}

WRITE32_MEMBER(archimedes_state::archimedes_vidc_w)
{
	UINT32 reg = data>>24;
	UINT32 val = data & 0xffffff;
	//#ifdef MAME_DEBUG
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
	if (reg <= 0x4c)
	{
		int r,g,b;

		//i = (val & 0x1000) >> 12; //supremacy bit
		b = (val & 0x0f00) >> 8;
		g = (val & 0x00f0) >> 4;
		r = (val & 0x000f) >> 0;

		if(reg == 0x40 && val & 0xfff)
			logerror("WARNING: border color write here (PC=%08x)!\n",space.device().safe_pc());

		m_palette->set_pen_color(reg >> 2, pal4bit(r), pal4bit(g), pal4bit(b) );

		/* handle 8bpp colors here */
		if(reg <= 0x3c)
		{
			int i;

			for(i=0;i<0x100;i+=0x10)
			{
				b = ((val & 0x700) >> 8) | ((i & 0x80) >> 4);
				g = ((val & 0x030) >> 4) | ((i & 0x20) >> 3) | ((i & 0x40) >> 3);
				r = ((val & 0x007) >> 0) | ((i & 0x10) >> 1);

				m_palette->set_pen_color((reg >> 2) + 0x100 + i, pal4bit(r), pal4bit(g), pal4bit(b) );
			}
		}

	}
	else if (reg >= 0x60 && reg <= 0x7c)
	{
		m_vidc_stereo_reg[(reg >> 2) & 7] = val & 0x07;

//      popmessage("%02x %02x %02x %02x %02x %02x %02x %02x",vidc_stereo_reg[0],vidc_stereo_reg[1],vidc_stereo_reg[2],vidc_stereo_reg[3]
//      ,vidc_stereo_reg[4],vidc_stereo_reg[5],vidc_stereo_reg[6],vidc_stereo_reg[7]);
	}
	else if (reg >= 0x80 && reg <= 0xbc)
	{
		switch(reg)
		{
			case VIDC_HCR:  m_vidc_regs[VIDC_HCR] =  ((val >> 14)<<1)+1;    break;
//          case VIDC_HSWR: m_vidc_regs[VIDC_HSWR] = (val >> 14)+1;   break;
			case VIDC_HBSR: m_vidc_regs[VIDC_HBSR] = ((val >> 14)<<1)+1;    break;
			case VIDC_HDSR: m_vidc_regs[VIDC_HDSR] = (val >> 14);   break;
			case VIDC_HDER: m_vidc_regs[VIDC_HDER] = (val >> 14);   break;
			case VIDC_HBER: m_vidc_regs[VIDC_HBER] = ((val >> 14)<<1)+1;    break;
//          #define VIDC_HCSR       0x98
//          #define VIDC_HIR        0x9c

			case VIDC_VCR:  m_vidc_regs[VIDC_VCR] = ((val >> 14)<<1)+1; break;
//          #define VIDC_VSWR       0xa4
			case VIDC_VBSR: m_vidc_regs[VIDC_VBSR] = (val >> 14)+1; break;
			case VIDC_VDSR: m_vidc_regs[VIDC_VDSR] = (val >> 14)+1; break;
			case VIDC_VDER: m_vidc_regs[VIDC_VDER] = (val >> 14)+1; break;
			case VIDC_VBER: m_vidc_regs[VIDC_VBER] = (val >> 14)+1; break;
//          #define VIDC_VCSR       0xb8
//          #define VIDC_VCER       0xbc
		}


		//#ifdef MAME_DEBUG
		if(reg != VIDC_VCSR && reg != VIDC_VCER && reg != VIDC_HCSR)
		logerror("VIDC: %s = %d\n", vrnames[(reg-0x80)/4], m_vidc_regs[reg]);
		//#endif

		vidc_dynamic_res_change();
	}
	else if(reg == 0xe0)
	{
		m_vidc_bpp_mode = ((val & 0x0c) >> 2);
		m_vidc_interlace = ((val & 0x40) >> 6);
		m_vidc_pixel_clk = (val & 0x03);
		//todo: vga/svga modes sets 0x1000
		vidc_dynamic_res_change();
	}
	else
	{
		logerror("VIDC: %x to register %x\n", val, reg);
		m_vidc_regs[reg] = val&0xffff;
	}
}

READ32_MEMBER(archimedes_state::archimedes_memc_r)
{
	return 0;
}

WRITE32_MEMBER(archimedes_state::archimedes_memc_w)
{
	// is it a register?
	if ((data & 0x0fe00000) == 0x03600000)
	{
		switch ((data >> 17) & 7)
		{
			case 0: /* video init */
				m_cursor_enabled = false;
				m_vidc_vidinit = ((data>>2)&0x7fff)*16;
				//printf("MEMC: VIDINIT %08x\n",m_vidc_vidinit);
				break;

			case 1: /* video start */
				m_vidc_vidstart = 0x2000000 | (((data>>2)&0x7fff)*16);
				//printf("MEMC: VIDSTART %08x\n",m_vidc_vidstart);
				break;

			case 2: /* video end */
				m_vidc_vidend = 0x2000000 | (((data>>2)&0x7fff)*16);
				//printf("MEMC: VIDEND %08x\n",m_vidc_vidend);
				break;

			case 3: /* cursor init */
				//m_cursor_enabled = true;
				m_vidc_cinit = 0x2000000 | (((data>>2)&0x7fff)*16);
				//printf("MEMC: CURSOR INIT %08x\n",((data>>2)&0x7fff)*16);
				break;

			case 4: /* sound start */
				//logerror("MEMC: SNDSTART %08x\n",data);
				archimedes_clear_irq_b(ARCHIMEDES_IRQB_SOUND_EMPTY);
				m_vidc_sndstart = 0x2000000 | ((data>>2)&0x7fff)*16;
				break;

			case 5: /* sound end */
				//logerror("MEMC: SNDEND %08x\n",data);
				m_vidc_sndend = 0x2000000 | ((data>>2)&0x7fff)*16;
				break;

			case 6:
				m_vidc_sndcur = 0;
				archimedes_request_irq_b(ARCHIMEDES_IRQB_SOUND_EMPTY);
				break;

			case 7: /* Control */
				m_memc_pagesize = ((data>>2) & 3);

				logerror("(PC = %08x) MEMC: %x to Control (page size %d, %s, %s)\n", space.device().safe_pc(), data & 0x1ffc, page_sizes[m_memc_pagesize], ((data>>10)&1) ? "Video DMA on" : "Video DMA off", ((data>>11)&1) ? "Sound DMA on" : "Sound DMA off");

				m_video_dma_on = ((data>>10)&1);
				m_audio_dma_on = ((data>>11)&1);

				if ((data>>10)&1)
				{
					m_vidc_vidcur = 0;
					m_vid_timer->adjust(m_screen->time_until_pos(m_vidc_regs[0xb4]));
				}

				if ((data>>11)&1)
				{
					double sndhz;

					/* FIXME: is the frequency correct? */
					sndhz = (250000.0 / 2) / (double)((m_vidc_regs[0xc0]&0xff)+2);

					printf("MEMC: Starting audio DMA at %f Hz, buffer from %x to %x\n", sndhz, m_vidc_sndstart, m_vidc_sndend);

					m_snd_timer->adjust(attotime::zero, 0, attotime::from_hz(sndhz));
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

WRITE32_MEMBER(archimedes_state::archimedes_memc_page_w)
{
	UINT32 log, phys, memc;

//  perms = (data & 0x300)>>8;
	log = phys = memc = 0;

	switch (m_memc_pagesize)
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
	m_memc_latchrom = 0;

	// now go ahead and set the mapping in the page table
	m_memc_pages[log] = phys + (memc*0x80);

//  printf("PC=%08x = MEMC_PAGE(%d): W %08x: log %x to phys %x, MEMC %d, perms %d\n", space.device().safe_pc(),memc_pagesize, data, log, phys, memc, perms);
}
