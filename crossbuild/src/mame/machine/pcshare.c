/***************************************************************************

    machine/pcshare.c

    Functions to emulate general aspects of the machine
    (RAM, ROM, interrupts, I/O ports)

    The information herein is heavily based on
    'Ralph Browns Interrupt List'
    Release 52, Last Change 20oct96

    TODO:
    clean up (maybe split) the different pieces of hardware
    PIC, PIT, DMA... add support for LPT, COM (almost done)
    emulation of a serial mouse on a COM port (almost done)
    support for Game Controller port at 0x0201
    support for XT harddisk (under the way, see machine/pc_hdc.c)
    whatever 'hardware' comes to mind,
    maybe SoundBlaster? EGA? VGA?

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "memconv.h"
#include "machine/8255ppi.h"

#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/mc146818.h"
#include "machine/pcshare.h"

#include "machine/8237dma.h"
#include "machine/pckeybrd.h"

#ifdef MESS
#include "machine/uart8250.h"
#include "video/pc_vga.h"
#include "video/pc_cga.h"
#include "video/pc_mda.h"
#include "video/pc_aga.h"

#include "includes/pc_mouse.h"
#include "machine/pc_fdc.h"

#include "includes/pclpt.h"
#include "machine/centroni.h"

#include "machine/pc_hdc.h"
#include "machine/nec765.h"
#include "audio/pc.h"

#include "mscommon.h"
#endif /* MESS */

#define VERBOSE_DBG 0       /* general debug messages */
#define DBG_LOG(N,M,A) \
	if(VERBOSE_DBG>=N){ if( M )logerror("%11.6f: %-24s",attotime_to_double(timer_get_time()),(char*)M ); logerror A; }

#define VERBOSE_JOY 0		/* JOY (joystick port) */
#define JOY_LOG(N,M,A) \
	if(VERBOSE_JOY>=N){ if( M )logerror("%11.6f: %-24s",attotime_to_double(timer_get_time()),(char*)M ); logerror A; }

#define FDC_DMA 2


static emu_timer *pc_keyboard_timer;

static TIMER_CALLBACK( pc_keyb_timer );

#define LOG_PORT80 0



/* ---------------------------------------------------------------------- */

#ifdef MESS
/* called when a interrupt is set/cleared from com hardware */
static void pc_com_interrupt(int nr, int state)
{
	static const int irq[4] = {4, 3, 4, 3};

	/* issue COM1/3 IRQ4, COM2/4 IRQ3 */
	pic8259_set_irq_line(0, irq[nr], state);
}

/* called when com registers read/written - used to update peripherals that
are connected */
static void pc_com_refresh_connected(int n, int data)
{
	/* mouse connected to this port? */
	if (readinputport(3) & (0x80>>n))
		pc_mouse_handshake_in(n,data);
}

/* PC interface to PC-com hardware. Done this way because PCW16 also
uses PC-com hardware and doesn't have the same setup! */
static const uart8250_interface com_interface[4]=
{
	{
		TYPE8250,
		1843200,
		pc_com_interrupt,
		NULL,
		pc_com_refresh_connected
	},
	{
		TYPE8250,
		1843200,
		pc_com_interrupt,
		NULL,
		pc_com_refresh_connected
	},
	{
		TYPE8250,
		1843200,
		pc_com_interrupt,
		NULL,
		pc_com_refresh_connected
	},
	{
		TYPE8250,
		1843200,
		pc_com_interrupt,
		NULL,
		pc_com_refresh_connected
	}
};
#endif /* MESS */



static void pc_timer0_w(int state)
{
	pic8259_set_irq_line(0, 0, state);
}



/*
 * timer0   heartbeat IRQ
 * timer1   DRAM refresh (ignored)
 * timer2   PIO port C pin 4 and speaker polling
 */
static const struct pit8253_config pc_pit8253_config =
{
	TYPE8253,
	{
		{
			4772720/4,				/* heartbeat IRQ */
			pc_timer0_w,
			NULL
		}, {
			4772720/4,				/* dram refresh */
			NULL,
			NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			NULL,
#ifdef MESS
			pc_sh_speaker_change_clock
#else
			NULL //pc_sh_speaker_change_clock
#endif /* MESS */
		}
	}
};

static const struct pit8253_config pc_pit8254_config =
{
	TYPE8254,
	{
		{
			4772720/4,				/* heartbeat IRQ */
			pc_timer0_w,
			NULL
		}, {
			4772720/4,				/* dram refresh */
			NULL,
			NULL
		}, {
			4772720/4,				/* pio port c pin 4, and speaker polling enough */
			NULL,
#ifdef MESS
			pc_sh_speaker_change_clock
#else
			NULL //pc_sh_speaker_change_clock
#endif /* MESS */
		}
	}
};


#ifdef MESS
static const PC_LPT_CONFIG lpt_config[3]={
	{
		1,
		LPT_UNIDIRECTIONAL,
		NULL
	},
	{
		1,
		LPT_UNIDIRECTIONAL,
		NULL
	},
	{
		1,
		LPT_UNIDIRECTIONAL,
		NULL
	}
};

static const CENTRONICS_CONFIG cent_config[3]={
	{
		PRINTER_IBM,
		pc_lpt_handshake_in
	},
	{
		PRINTER_IBM,
		pc_lpt_handshake_in
	},
	{
		PRINTER_IBM,
		pc_lpt_handshake_in
	}
};
#endif



/*************************************************************************
 *
 *      PC DMA stuff
 *
 *************************************************************************/

static UINT8 dma_offset[2][4];
static UINT8 at_pages[0x10];
static offs_t pc_page_offset_mask;



READ8_HANDLER(pc_page_r)
{
	return 0xFF;
}



WRITE8_HANDLER(pc_page_w)
{
	switch(offset % 4) {
	case 1:
		dma_offset[0][2] = data;
		break;
	case 2:
		dma_offset[0][3] = data;
		break;
	case 3:
		dma_offset[0][0] = dma_offset[0][1] = data;
		break;
	}
}



READ16_HANDLER(pc_page16le_r) { return read16le_with_read8_handler(pc_page_r, offset, mem_mask); }
WRITE16_HANDLER(pc_page16le_w) { write16le_with_write8_handler(pc_page_w, offset, data, mem_mask); }



READ8_HANDLER(at_page8_r)
{
	UINT8 data = at_pages[offset % 0x10];

	switch(offset % 8) {
	case 1:
		data = dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}



WRITE8_HANDLER(at_page8_w)
{
	at_pages[offset % 0x10] = data;

	if (LOG_PORT80 && (offset == 0))
		logerror(" at_page8_w(): Port 80h <== 0x%02x (PC=0x%08x)\n", data, (unsigned) activecpu_get_reg(REG_PC));

	switch(offset % 8) {
	case 1:
		dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}



READ32_HANDLER(at_page32_r)
{
	return read32le_with_read8_handler(at_page8_r, offset, mem_mask);
}



WRITE32_HANDLER(at_page32_w)
{
	write32le_with_write8_handler(at_page8_w, offset, data, mem_mask);
}



static UINT8 pc_dma_read_byte(int channel, offs_t offset)
{
	UINT8 result;
	offs_t page_offset = (((offs_t) dma_offset[0][channel]) << 16)
		& pc_page_offset_mask;

	cpuintrf_push_context(0);
	result = program_read_byte(page_offset + offset);
	cpuintrf_pop_context();

	return result;
}



static void pc_dma_write_byte(int channel, offs_t offset, UINT8 data)
{
	offs_t page_offset = (((offs_t) dma_offset[0][channel]) << 16)
		& pc_page_offset_mask;

	cpuintrf_push_context(0);
	program_write_byte(page_offset + offset, data);
	cpuintrf_pop_context();
}



static const struct dma8237_interface pc_dma =
{
	0,
	1.0e-6,	// 1us

	pc_dma_read_byte,
	pc_dma_write_byte,

#ifdef MESS
	{ 0, 0, pc_fdc_dack_r, pc_hdc_dack_r },
	{ 0, 0, pc_fdc_dack_w, pc_hdc_dack_w },
	pc_fdc_set_tc_state
#else
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 },
	0
#endif
};



/* ----------------------------------------------------------------------- */

#ifdef MESS
static void pc_fdc_interrupt(int state)
{
	pic8259_set_irq_line(0, 6, state);
}



static void pc_fdc_dma_drq(int state, int read_)
{
	dma8237_drq_write(0, FDC_DMA, state);
}



static const struct pc_fdc_interface fdc_interface =
{
	NEC765A,
	pc_fdc_interrupt,
	pc_fdc_dma_drq,
};
#endif /* MESS */



/* ----------------------------------------------------------------------- */

static void pc_pic_set_int_line(int which, int interrupt)
{
	switch(which)
	{
		case 0:
			/* Master */
			cpunum_set_input_line(Machine, 0, 0, interrupt ? HOLD_LINE : CLEAR_LINE);
			break;

		case 1:
			/* Slave */
			pic8259_set_irq_line(0, 2, interrupt);
			break;
	}
}



void init_pc_common(UINT32 flags)
{
#ifdef MESS
	/* MESS managed RAM */
	if (mess_ram)
		memory_set_bankptr(10, mess_ram);
#endif /* MESS */

	/* PIT */
	if (flags & PCCOMMON_TIMER_8254)
		pit8253_init(1, &pc_pit8254_config);
	else if (flags & PCCOMMON_TIMER_8253)
		pit8253_init(1, &pc_pit8253_config);

#ifdef MESS
	/* FDC/HDC hardware */
	pc_fdc_init(&fdc_interface);
	pc_hdc_setup();

	/* com hardware */
	uart8250_init(0, com_interface);
	uart8250_reset(0);
	uart8250_init(1, com_interface+1);
	uart8250_reset(1);
	uart8250_init(2, com_interface+2);
	uart8250_reset(2);
	uart8250_init(3, com_interface+3);
	uart8250_reset(3);

	pc_lpt_config(0, lpt_config);
	centronics_config(0, cent_config);
	pc_lpt_set_device(0, &CENTRONICS_PRINTER_DEVICE);
	pc_lpt_config(1, lpt_config+1);
	centronics_config(1, cent_config+1);
	pc_lpt_set_device(1, &CENTRONICS_PRINTER_DEVICE);
	pc_lpt_config(2, lpt_config+2);
	centronics_config(2, cent_config+2);
	pc_lpt_set_device(2, &CENTRONICS_PRINTER_DEVICE);

	/* serial mouse */
	pc_mouse_set_serial_port(0);
	pc_mouse_initialise();
#endif /* MESS */

	/* PC-XT keyboard */
	if (flags & PCCOMMON_KEYBOARD_AT)
		at_keyboard_init(AT_KEYBOARD_TYPE_AT);
	else
		at_keyboard_init(AT_KEYBOARD_TYPE_PC);
	at_keyboard_set_scan_code_set(1);

	/* PIC */
	pic8259_init(2, pc_pic_set_int_line);

	/* DMA */
	if (flags & PCCOMMON_DMA8237_AT)
	{
		dma8237_init(2);
		dma8237_config(0, &pc_dma);
		pc_page_offset_mask = 0xFF0000;
	}
	else
	{
		dma8237_init(1);
		dma8237_config(0, &pc_dma);
		pc_page_offset_mask = 0x0F0000;
	}

	pc_keyboard_timer = timer_alloc(pc_keyb_timer, NULL);
}

/*
   keyboard seams to permanently sent data clocked by the mainboard
   clock line low for longer means "resync", keyboard sends 0xaa as answer
   will become automatically 0x00 after a while
*/

static struct {
	UINT8 data;
	int on;
} pc_keyb= { 0 };

UINT8 pc_keyb_read(void)
{
	return pc_keyb.data;
}



static TIMER_CALLBACK( pc_keyb_timer )
{
	at_keyboard_reset();
	pc_keyboard();
}



void pc_keyb_set_clock(int on)
{
	attotime keyb_delay = STATIC_ATTOTIME_IN_MSEC(5);

	on = on ? 1 : 0;

	if (pc_keyb.on != on)
	{
		if (on)
			timer_adjust_oneshot(pc_keyboard_timer, keyb_delay, 0);
		else
			timer_reset(pc_keyboard_timer, attotime_never);

		pc_keyb.on = on;
	}
}

void pc_keyb_clear(void)
{
	pc_keyb.data = 0;
}

void pc_keyboard(void)
{
	int data;

	at_keyboard_polling();

	if (pc_keyb.on)
	{
		if ( (data=at_keyboard_read())!=-1) {
			pc_keyb.data = data;
			DBG_LOG(1,"KB_scancode",("$%02x\n", pc_keyb.data));
			pic8259_set_irq_line(0, 1, 1);
			pic8259_set_irq_line(0, 1, 0);
		}
	}
}
