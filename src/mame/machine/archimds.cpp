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
#include "includes/archimds.h"
#include "cpu/arm/arm.h"
#include "debugger.h"

static const int page_sizes[4] = { 4096, 8192, 16384, 32768 };

#define IOC_LOG 0
#define CRTC_LOG 0

/* TODO: fix pending irqs */
void archimedes_state::archimedes_request_irq_a(int mask)
{
	m_ioc_regs[IRQ_STATUS_A] |= mask;

	if ((m_ioc_regs[IRQ_STATUS_A] & m_ioc_regs[IRQ_MASK_A]) || (m_ioc_regs[IRQ_STATUS_B] & m_ioc_regs[IRQ_MASK_B]))
		m_maincpu->set_input_line(ARM_IRQ_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(ARM_IRQ_LINE, CLEAR_LINE);
}

void archimedes_state::archimedes_request_irq_b(int mask)
{
	m_ioc_regs[IRQ_STATUS_B] |= mask;

	if ((m_ioc_regs[IRQ_STATUS_A] & m_ioc_regs[IRQ_MASK_A]) || (m_ioc_regs[IRQ_STATUS_B] & m_ioc_regs[IRQ_MASK_B]))
		m_maincpu->set_input_line(ARM_IRQ_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(ARM_IRQ_LINE, CLEAR_LINE);
}

void archimedes_state::archimedes_request_fiq(int mask)
{
	m_ioc_regs[FIQ_STATUS] |= mask;

	//printf("STATUS:%02x IRQ:%02x MASK:%02x\n",m_ioc_regs[FIQ_STATUS],mask,m_ioc_regs[FIQ_MASK]);

	if (m_ioc_regs[FIQ_STATUS] & m_ioc_regs[FIQ_MASK])
	{
		m_maincpu->pulse_input_line(ARM_FIRQ_LINE, m_maincpu->minimum_quantum_time());

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
	archimedes_request_irq_b(0);
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
		case TIMER_IOC: ioc_timer(param); break;
	}
}


WRITE_LINE_MEMBER( archimedes_state::vblank_irq )
{
	if (state)
	{
		archimedes_request_irq_a(ARCHIMEDES_IRQA_VBL);
		if (m_video_dma_on)
			vidc_video_tick();
	}
}

WRITE_LINE_MEMBER( archimedes_state::sound_drq )
{
	if (state)
		vidc_audio_tick();
}


/* video DMA */
// TODO: what type of DMA this is, burst or cycle steal? Docs doesn't explain it (4 usec is the DRAM refresh). */
// TODO: Erotictac and Poizone sets up vidinit register AFTER vidend, for double buffering? (fixes Poizone "Eterna" logo display on attract)
// TODO: understand how to make quazer to work (sets video DMA param in-flight)
void archimedes_state::vidc_video_tick()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint32_t size;
	uint32_t offset_ptr;

	size = (m_vidc_vidend - m_vidc_vidstart + 0x10) & 0x1fffff;

	offset_ptr = m_vidc_vidinit;
	if(offset_ptr >= m_vidc_vidend+0x10) // TODO: correct?
		offset_ptr = m_vidc_vidstart;

	//popmessage("%08x %08x %08x",m_vidc_vidstart,m_vidc_vidinit,m_vidc_vidend);

	for(m_vidc_vidcur = 0;m_vidc_vidcur < size;m_vidc_vidcur++)
	{
		m_vidc->write_vram(m_vidc_vidcur, space.read_byte(offset_ptr));
		offset_ptr++;
		if(offset_ptr >= m_vidc_vidend+0x10) // TODO: correct?
			offset_ptr = m_vidc_vidstart;
	}

	if(m_cursor_enabled == true)
	{
		uint32_t ccur_size = m_vidc->get_cursor_size();

		for(uint32_t ccur = 0; ccur < ccur_size; ccur++)
			m_vidc->write_cram(ccur, space.read_byte(m_vidc_cinit+ccur));
	}
}

/* audio DMA */
void archimedes_state::vidc_audio_tick()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t ch;

	for(ch=0; ch<8; ch++)
		m_vidc->write_dac(ch, (space.read_byte(m_vidc_sndcur + ch)));

	m_vidc_sndcur+=8;

	if (m_vidc_sndcur >= m_vidc_sndendcur)
	{
		archimedes_request_irq_b(ARCHIMEDES_IRQB_SOUND_EMPTY);

		// TODO: nuke this implementation detail, repeated below
		m_vidc->update_sound_mode(m_audio_dma_on);
		if(!m_audio_dma_on)
		{
			for(ch=0; ch<8; ch++)
				m_vidc->clear_dac(ch);
		}
		else
		{
			//printf("Chaining to next: start %x end %x\n", m_vidc_sndstart, m_vidc_sndend);
			m_vidc_sndcur = m_vidc_sndstart;
			m_vidc_sndendcur = m_vidc_sndend;
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

	// keep FIQ line ASSERTED if there are active requests
	if (m_ioc_regs[FIQ_STATUS] & m_ioc_regs[FIQ_MASK])
		archimedes_request_fiq(0);

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
	m_memc_pagesize = 0;

	m_timer[0] = timer_alloc(TIMER_IOC);
	m_timer[1] = timer_alloc(TIMER_IOC);
	m_timer[2] = timer_alloc(TIMER_IOC);
	m_timer[3] = timer_alloc(TIMER_IOC);
	m_timer[0]->adjust(attotime::never);
	m_timer[1]->adjust(attotime::never);
	m_timer[2]->adjust(attotime::never);
	m_timer[3]->adjust(attotime::never);
}

READ32_MEMBER(archimedes_state::archimedes_memc_logical_r)
{
	uint32_t page, poffs;

	// are we mapping in the boot ROM?
	if (m_memc_latchrom)
	{
		uint32_t *rom;

		rom = (uint32_t *)m_region_maincpu->base();

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
	uint32_t page, poffs;

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
	uint32_t page, poffs;

	// are we mapping in the boot ROM?
	if (m_memc_latchrom)
	{
		uint32_t *rom;

		rom = (uint32_t *)m_region_maincpu->base();

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
	m_archimedes_memc_physmem = reinterpret_cast<uint32_t *>(memshare("physicalram")->ptr());
//  address_space &space = m_maincpu->space(AS_PROGRAM);
//  space.set_direct_update_handler(direct_update_delegate(&a310_setopbase, &machine));
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
	m_ioc_timerout[tmr] = m_ioc_timercnt[tmr] - (uint32_t)time;
}

bool archimedes_state::check_floppy_ready()
{
	floppy_image_device *floppy = nullptr;

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
		return !floppy->ready_r();

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
			uint8_t i2c_data = 1;
			bool floppy_ready_state;

			if ( m_i2cmem )
			{
				i2c_data = (m_i2cmem->read_sda() & 1);
			}

			floppy_ready_state = check_floppy_ready();

			return (m_vidc->flyback_r()<<7) | (m_ioc_regs[CONTROL] & 0x78) | (floppy_ready_state<<2) | (m_i2c_clk<<1) | i2c_data;
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
				logerror("IOC: R %s = %02x (PC=%x) %02x\n", ioc_regnames[offset&0x1f], m_ioc_regs[offset&0x1f], m_maincpu->pc(), offset & 0x1f);
			break;
	}

	return m_ioc_regs[offset&0x1f];
}

/* TODO: should be a 8-bit handler */
WRITE32_MEMBER( archimedes_state::ioc_ctrl_w )
{
	if(IOC_LOG)
	logerror("IOC: W %02x @ reg %s (PC=%x)\n", data&0xff, ioc_regnames[offset&0x1f], m_maincpu->pc());

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

			//m_ioc_regs[CONTROL] = data & 0x38;
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

			//if(data & 0x08) //set up the VBLANK timer
			//  m_vbl_timer->adjust(m_screen->time_until_pos(m_vidc_vblank_time));

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
				logerror("IOC: W %02x @ reg %s (PC=%x)\n", data&0xff, ioc_regnames[offset&0x1f], m_maincpu->pc());

			m_ioc_regs[offset&0x1f] = data & 0xff;
			break;
	}
}

READ32_MEMBER(archimedes_state::archimedes_ioc_r)
{
	uint32_t ioc_addr;

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
						return m_fdc->read((ioc_addr >> 2) & 0x03);
					} else {
						logerror("Read from FDC device?\n");
						return 0;
					}
				case 2:
					// RTFM joystick interface routes here
					// TODO: slot interface for econet (reads registers 0 and 1 during boot)
					switch(ioc_addr)
					{
						case 0x3a0000:
							return 0xed; // ID for econet
						case 0x3a0004:
							return m_joy[0].read_safe(0xff);
						case 0x3a0008:
							// Top Banana reads there and do various checks,
							// disallowing player 1 joy use if they fails (?)
							return m_joy[1].read_safe(0xff);
					}

					logerror("IOC: Econet Read %08x at PC=%08x\n",ioc_addr, m_maincpu->pc());
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
						// TODO: IOEB slot interface
						switch(ioc_addr & 0xfffc)
						{
							case 0x18: return 0xff; // FDC latch B
							case 0x40: return 0xff; // FDC latch A
							case 0x50: return 0; //fdc type, an 82c711 returns 5 here
							case 0x70: return 0x0f; // monitor type, TBD
							case 0x74: return 0xff; // unknown
							case 0x78: // serial joystick?
							case 0x7c:
								logerror("FDC: reading Joystick port %04x at PC=%08x\n",ioc_addr, m_maincpu->pc());
								return 0xff;

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
	uint32_t ioc_addr;

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
							m_fdc->write((ioc_addr >> 2) & 0x03, data);
							return;
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
							// serial joy port (!JS application)
							case 0x10:
							{
								// compared to RTFM they reversed bits 0-3 (or viceversa, dunno what came out first)
								// for pragmatic convenience we bitswap here, but this should really be a slot option at some point.
								// TODO: understand how player 2 inputs routes, related somehow to CONTROL bit 6 (cfr. blitz in SW list)
								// TODO: paradr2k polls here with bit 7 and fails detection (Vertical Twist)
								uint8_t cur_joy_in = bitswap<8>(m_joy[0].read_safe(0xff),7,6,5,4,0,1,2,3);

								m_joy_serial_data = (data & 0xff) ^ 0xff;
								bool serial_on = false;

								if (m_joy_serial_data == 0x20)
									serial_on = true;
								else if (m_joy_serial_data & cur_joy_in)
									serial_on = true;


								// wants printer irq for some reason (connected on parallel?)
								if (serial_on == true)
								{
									archimedes_request_irq_a(ARCHIMEDES_IRQA_PRINTER_BUSY);
									//m_ioc_regs[CONTROL] |= 0x40;
								}
								else
								{
									archimedes_clear_irq_a(ARCHIMEDES_IRQA_PRINTER_BUSY);
									//m_ioc_regs[CONTROL] &= ~0x40;
								}

								return;
							}
							case 0x18: // latch B
								/*
								---- x--- floppy controller reset
								*/
								m_fdc->dden_w(BIT(data, 1));
								if (!(data & 8))
									m_fdc->soft_reset();
								if(data & ~0xa)
									printf("%02x Latch B\n",data);
								return;

							case 0x40: // latch A
								/*
								-x-- ---- In Use Control (floppy?)
								*/
								floppy_image_device *floppy = nullptr;

								if (!(data & 1)) { m_floppy_select = 0; floppy = m_floppy0->get_device(); }
								if (!(data & 2)) { m_floppy_select = 1; floppy = m_floppy1->get_device(); }
								if (!(data & 4)) { m_floppy_select = 2; floppy = nullptr; } // floppy 2
								if (!(data & 8)) { m_floppy_select = 3; floppy = nullptr; } // floppy 3

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


	logerror("(PC=%08x) I/O: W %x @ %x (mask %08x)\n", m_maincpu->pc(), data, (offset*4)+0x3000000, mem_mask);
}

WRITE32_MEMBER(archimedes_state::archimedes_memc_w)
{
	// is it a register?
	if ((data & 0x0fe00000) == 0x03600000)
	{
		switch ((data >> 17) & 7)
		{
			case 0: /* video init */
				m_vidc_vidinit = 0x2000000 | ((data>>2)&0x7fff)*16;
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
				m_cursor_enabled = true;
				m_vidc->set_cursor_enable(m_cursor_enabled);
				m_vidc_cinit = 0x2000000 | (((data>>2)&0x7fff)*16);
				//printf("MEMC: CURSOR INIT %08x\n",((data>>2)&0x7fff)*16);
				break;

			case 4: /* sound start */
				archimedes_clear_irq_b(ARCHIMEDES_IRQB_SOUND_EMPTY);
				m_vidc_sndstart = 0x2000000 | ((data>>2)&0x7fff)*16;
				//printf("MEMC: SNDSTART %08x\n",m_vidc_sndstart);
				break;

			case 5: /* sound end */
				// end buffer is actually +16 bytes wrt sound start
				// TODO: it actually don't apply for ertictac and poizone?
				m_vidc_sndend = 0x2000000 | (((data>>2)+1)&0x7fff)*16;
				//printf("MEMC: SNDEND %08x\n",m_vidc_sndend);
				break;

			case 6:
				//printf("MEMC: SNDPTR\n");
				m_vidc_sndcur = m_vidc_sndstart;
				m_vidc_sndendcur = m_vidc_sndend;
				archimedes_request_irq_b(ARCHIMEDES_IRQB_SOUND_EMPTY);
				break;

			case 7: /* Control */
				m_memc_pagesize = ((data>>2) & 3);

				logerror("(PC = %08x) MEMC: %x to Control (page size %d, %s, %s)\n", m_maincpu->pc(), data & 0x1ffc, page_sizes[m_memc_pagesize], ((data>>10)&1) ? "Video DMA on" : "Video DMA off", ((data>>11)&1) ? "Sound DMA on" : "Sound DMA off");

				m_video_dma_on = BIT(data, 10);
				m_audio_dma_on = BIT(data, 11);

				if (m_video_dma_on)
				{
					m_vidc_vidcur = 0;
					// TODO: update internally
				}
				else
				{
					m_cursor_enabled = false;
					m_vidc->set_cursor_enable(m_cursor_enabled);
				}

				m_vidc->update_sound_mode(m_audio_dma_on);
				if (m_audio_dma_on)
				{
					//printf("MEMC: Starting audio DMA at %d uSec, buffer from %x to %x\n", ((m_vidc_regs[0xc0]&0xff)-2)*8, m_vidc_sndstart, m_vidc_sndend);

					//printf("MEMC: audio DMA start, sound freq %d, sndhz = %f\n", (m_vidc_regs[0xc0] & 0xff)-2, sndhz);

					m_vidc_sndcur = m_vidc_sndstart;
					m_vidc_sndendcur = m_vidc_sndend;
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
	uint32_t log, phys, memc;

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

//  printf("PC=%08x = MEMC_PAGE(%d): W %08x: log %x to phys %x, MEMC %d, perms %d\n", m_maincpu->pc(),memc_pagesize, data, log, phys, memc, perms);
}
