/*
    machine/rmnimbus.c

    Machine driver for the Research Machines Nimbus.

    Phill Harvey-Smith
    2009-11-29.

    80186 internal DMA/Timer/PIC code borrowed from Compis driver.
    Perhaps this needs merging into the 80186 core.....
*/

/*

    SCSI/SASI drives supported by RM Nimbus machines

Native SCSI - format with HDFORM.EXE

Drive           Capacity    Tracks  Heads   Sec/Track       Blocks
RO652-20        20MB        306     4       34              41616
ST225N          20MB        615     4       17              41721
ST125N          20MB        407     4       26              41921
8425S-30        20MB                                        41004
CP3020          20MB        623     2       33              41118
ST225NP         20MB        615     4       17              41720
CP3040          40MB        1026    2       40              82080

Via Xebec S1410 SASI to MFM bridge board - format with WINFORM.EXE
NP05-10S         8MB        160     6       17              16320
NP04-20T        16MB        320     6       17              32640
NP03-20         15MB        306     6       17              31212
R352-10         10MB        306     4       17              20808
NP04-50         40MB        699     7       17              83181
NP04-55         44MB        754     7       17              89726

Via Adaptec ACB4070 SCSI to RLL bridge board - format with ADAPT.EXE
NEC D5147       60MB        615     8       26              127920
ST227R          60MB        820     6       26              127920

After formating, the drives need to have a partition table put on them with
STAMP.EXE and then formatted in the normal way for a dos system drive with
Format /s.

The tracks, heads and sectors/track can be used with chdman createhd
to create a blank hard disk which can then be formatted with the RM tools.
The important thing when doing this is to make sure that if using the Native
SCSI tools, that the disk has the  same number of blocks as specified above,
even if you have to use unusual geometry to do so !

for example:

chdman createhd -o ST125N.chd -chs 407,4,26 -ss 512

*/


#include "emu.h"
#include "debugger.h"
#include "cpu/i86/i86.h"
#include "debug/debugcpu.h"
#include "debug/debugcon.h"
#include "imagedev/flopdrv.h"
#include "machine/ram.h"
#include "machine/er59256.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/i8251.h"
#include "machine/ctronics.h"
#include "machine/6522via.h"
#include "machine/scsibus.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"

#include "includes/rmnimbus.h"



/*-------------------------------------------------------------------------*/
/* Defines, constants, and global variables                                */
/*-------------------------------------------------------------------------*/

/* CPU 80186 */
#define LATCH_INTS          1
#define LOG_PORTS           0
#define LOG_INTERRUPTS      0
#define LOG_INTERRUPTS_EXT  0
#define LOG_TIMER           0
#define LOG_OPTIMIZATION    0
#define LOG_DMA             0
#define CPU_RESUME_TRIGGER  7123
#define LOG_KEYBOARD        0
#define LOG_SIO             0
#define LOG_DISK_FDD        0
#define LOG_DISK_HDD        0
#define LOG_DISK            0
#define LOG_PC8031          0
#define LOG_PC8031_186      0
#define LOG_PC8031_PORT     0
#define LOG_IOU             0
#define LOG_SOUND           0
#define LOG_RAM             0

/* Debugging */

#define DEBUG_SET(flags)    ((state->m_debug_machine & (flags))==(flags))

#define DEBUG_NONE          0x0000000
#define DMA_BREAK           0x0000001
#define DECODE_BIOS         0x0000002
#define DECODE_BIOS_RAW     0x0000004
#define DECODE_DOS21        0x0000008


/* Z80 SIO */

const z80sio_interface nimbus_sio_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(rmnimbus_state,sio_interrupt),         /* interrupt handler */
	DEVCB_NULL, //sio_dtr_w,             /* DTR changed handler */
	DEVCB_NULL,                     /* RTS changed handler */
	DEVCB_NULL,                     /* BREAK changed handler */
	DEVCB_DRIVER_MEMBER16(rmnimbus_state,sio_serial_transmit),  /* transmit handler */
	DEVCB_DRIVER_MEMBER16(rmnimbus_state,sio_serial_receive)        /* receive handler */
};

/* Floppy drives WD2793 */




const wd17xx_interface nimbus_wd17xx_interface =
{
	DEVCB_LINE_GND,
	DEVCB_DRIVER_LINE_MEMBER(rmnimbus_state,nimbus_fdc_intrq_w),
	DEVCB_DRIVER_LINE_MEMBER(rmnimbus_state,nimbus_fdc_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};


static const UINT16 def_config[16] =
{
	0x0280, 0x017F, 0xE822, 0x8129,
	0x0329, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x8796, 0x2025, 0xB9E6
};

/* Memory controler */

/* IO Unit */

/* Sound */


static void drq_callback(running_machine &machine, int which);
static void nimbus_recalculate_ints(running_machine &machine);

static void execute_debug_irq(running_machine &machine, int ref, int params, const char *param[]);
static void execute_debug_intmasks(running_machine &machine, int ref, int params, const char *param[]);
static void nimbus_debug(running_machine &machine, int ref, int params, const char *param[]);

static int instruction_hook(device_t &device, offs_t curpc);
static void decode_subbios(device_t *device,offs_t pc, UINT8 raw_flag);
static void decode_dos21(device_t *device,offs_t pc);
static void decode_dssi_generic(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_fill_area(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_plot_character_string(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_set_new_clt(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_plonk_char(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);
static void decode_dssi_f_rw_sectors(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag);

static void nimbus_bank_memory(running_machine &machine);
static void memory_reset(running_machine &machine);
static void fdc_reset(running_machine &machine);
static void set_disk_int(running_machine &machine, int state);
static void hdc_reset(running_machine &machine);
static void hdc_ctrl_write(running_machine &machine, UINT8 data);
static void hdc_post_rw(running_machine &machine);
static void hdc_drq(running_machine &machine);

static void keyboard_reset(running_machine &machine);


static void pc8031_reset(running_machine &machine);
static void iou_reset(running_machine &machine);
static void rmni_sound_reset(running_machine &machine);

static void mouse_js_reset(running_machine &machine);



/*************************************
 *
 *  80186 interrupt controller
 *
 *************************************/
static IRQ_CALLBACK(int_callback)
{
	rmnimbus_state *state = device->machine().driver_data<rmnimbus_state>();
	UINT8   vector;
	UINT16  old;
	UINT16  oldreq;

	if (LOG_INTERRUPTS)
		logerror("(%f) **** Acknowledged interrupt vector %02X\n", device->machine().time().as_double(), state->m_i186.intr.poll_status & 0x1f);

	/* clear the interrupt */
	device->execute().set_input_line(0, CLEAR_LINE);
	state->m_i186.intr.pending = 0;

	oldreq=state->m_i186.intr.request;

	/* clear the request and set the in-service bit */
#if LATCH_INTS
	state->m_i186.intr.request &= ~state->m_i186.intr.ack_mask;
#else
	state->m_i186.intr.request &= ~(state->m_i186.intr.ack_mask & 0x0f);
#endif

	if((LOG_INTERRUPTS) && (state->m_i186.intr.request!=oldreq))
		logerror("i186.intr.request changed from %02X to %02X\n",oldreq,state->m_i186.intr.request);

	old=state->m_i186.intr.in_service;

	state->m_i186.intr.in_service |= state->m_i186.intr.ack_mask;

	if((LOG_INTERRUPTS) && (state->m_i186.intr.in_service!=old))
		logerror("i186.intr.in_service changed from %02X to %02X\n",old,state->m_i186.intr.in_service);

	if (state->m_i186.intr.ack_mask == 0x0001)
	{
		switch (state->m_i186.intr.poll_status & 0x1f)
		{
			case 0x08:  state->m_i186.intr.status &= ~0x01; break;
			case 0x12:  state->m_i186.intr.status &= ~0x02; break;
			case 0x13:  state->m_i186.intr.status &= ~0x04; break;
		}
	}
	state->m_i186.intr.ack_mask = 0;

	/* a request no longer pending */
	state->m_i186.intr.poll_status &= ~0x8000;

	/* return the vector */
	switch(state->m_i186.intr.poll_status & 0x1F)
	{
		case 0x0C   : vector=(state->m_i186.intr.ext[0] & EXTINT_CTRL_CASCADE) ? state->m_i186.intr.ext_vector[0] : (state->m_i186.intr.poll_status & 0x1f); break;
		case 0x0D   : vector=(state->m_i186.intr.ext[1] & EXTINT_CTRL_CASCADE) ? state->m_i186.intr.ext_vector[1] : (state->m_i186.intr.poll_status & 0x1f); break;
		default :
			vector=state->m_i186.intr.poll_status & 0x1f; break;
	}

	if (LOG_INTERRUPTS)
	{
		logerror("i186.intr.ext[0]=%04X i186.intr.ext[1]=%04X\n",state->m_i186.intr.ext[0],state->m_i186.intr.ext[1]);
		logerror("Ext vectors : %02X %02X\n",state->m_i186.intr.ext_vector[0],state->m_i186.intr.ext_vector[1]);
		logerror("Int %02X Calling vector %02X\n",state->m_i186.intr.poll_status,vector);
	}

	return vector;
}


static void update_interrupt_state(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	int new_vector = 0;
	int Priority;
	int IntNo;

	if (LOG_INTERRUPTS)
		logerror("update_interrupt_status: req=%04X stat=%04X serv=%04X priority_mask=%4X\n", state->m_i186.intr.request, state->m_i186.intr.status, state->m_i186.intr.in_service, state->m_i186.intr.priority_mask);

	/* loop over priorities */
	for (Priority = 0; Priority <= state->m_i186.intr.priority_mask; Priority++)
	{
		/* note: by checking 4 bits, we also verify that the mask is off */
		if ((state->m_i186.intr.timer & 0x0F) == Priority)
		{
			/* if we're already servicing something at this level, don't generate anything new */
			if (state->m_i186.intr.in_service & 0x01)
				return;

			/* if there's something pending, generate an interrupt */
			if (state->m_i186.intr.status & 0x07)
			{
				if (state->m_i186.intr.status & 1)
					new_vector = 0x08;
				else if (state->m_i186.intr.status & 2)
					new_vector = 0x12;
				else if (state->m_i186.intr.status & 4)
					new_vector = 0x13;
				else
					popmessage("Invalid timer interrupt!");

				/* set the clear mask and generate the int */
				state->m_i186.intr.ack_mask = 0x0001;
				goto generate_int;
			}
		}

		/* check DMA interrupts */
		for (IntNo = 0; IntNo < 2; IntNo++)
			if ((state->m_i186.intr.dma[IntNo] & 0x0F) == Priority)
			{
				/* if we're already servicing something at this level, don't generate anything new */
				if (state->m_i186.intr.in_service & (0x04 << IntNo))
					return;

				/* if there's something pending, generate an interrupt */
				if (state->m_i186.intr.request & (0x04 << IntNo))
				{
					new_vector = 0x0a + IntNo;

					/* set the clear mask and generate the int */
					state->m_i186.intr.ack_mask = 0x0004 << IntNo;
					goto generate_int;
				}
			}

		/* check external interrupts */
		for (IntNo = 0; IntNo < 4; IntNo++)
			if ((state->m_i186.intr.ext[IntNo] & 0x0F) == Priority)
			{
				if (LOG_INTERRUPTS)
					logerror("Int%d priority=%d\n",IntNo,Priority);

				/* if we're already servicing something at this level, don't generate anything new */
				if (state->m_i186.intr.in_service & (0x10 << IntNo))
					return;

				/* if there's something pending, generate an interrupt */
				if (state->m_i186.intr.request & (0x10 << IntNo))
				{
					/* otherwise, generate an interrupt for this request */
					new_vector = 0x0c + IntNo;

					/* set the clear mask and generate the int */
					state->m_i186.intr.ack_mask = 0x0010 << IntNo;
					goto generate_int;
				}
			}
	}
	return;

generate_int:
	/* generate the appropriate interrupt */
	state->m_i186.intr.poll_status = 0x8000 | new_vector;
	if (!state->m_i186.intr.pending)
		machine.device(MAINCPU_TAG)->execute().set_input_line(0, ASSERT_LINE);
	state->m_i186.intr.pending = 1;
	machine.scheduler().trigger(CPU_RESUME_TRIGGER);
	if (LOG_OPTIMIZATION) logerror("  - trigger due to interrupt pending\n");
	if (LOG_INTERRUPTS) logerror("(%f) **** Requesting interrupt vector %02X\n", machine.time().as_double(), new_vector);
}


static void handle_eoi(running_machine &machine,int data)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	int Priority;
	int IntNo;
	int handled=0;

	/* specific case */
	if (!(data & 0x8000))
	{
		/* turn off the appropriate in-service bit */
		switch (data & 0x1f)
		{
			case 0x08:  state->m_i186.intr.in_service &= ~0x01; break;
			case 0x12:  state->m_i186.intr.in_service &= ~0x01; break;
			case 0x13:  state->m_i186.intr.in_service &= ~0x01; break;
			case 0x0a:  state->m_i186.intr.in_service &= ~0x04; break;
			case 0x0b:  state->m_i186.intr.in_service &= ~0x08; break;
			case 0x0c:  state->m_i186.intr.in_service &= ~0x10; break;
			case 0x0d:  state->m_i186.intr.in_service &= ~0x20; break;
			case 0x0e:  state->m_i186.intr.in_service &= ~0x40; break;
			case 0x0f:  state->m_i186.intr.in_service &= ~0x80; break;
			default:    logerror("%05X:ERROR - 80186 EOI with unknown vector %02X\n", machine.device(MAINCPU_TAG)->safe_pc(), data & 0x1f);
		}
		if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for vector %02X\n", machine.time().as_double(), data & 0x1f);
	}

	/* non-specific case */
	else
	{
		/* loop over priorities */
		for (Priority = 0; ((Priority <= 7) && !handled); Priority++)
		{
			/* check for in-service timers */
			if ((state->m_i186.intr.timer & 0x07) == Priority && (state->m_i186.intr.in_service & 0x01))
			{
				state->m_i186.intr.in_service &= ~0x01;
				if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for timer\n", machine.time().as_double());
				handled=1;
			}

			/* check for in-service DMA interrupts */
			for (IntNo = 0; ((IntNo < 2) && !handled) ; IntNo++)
				if ((state->m_i186.intr.dma[IntNo] & 0x07) == Priority && (state->m_i186.intr.in_service & (0x04 << IntNo)))
				{
					state->m_i186.intr.in_service &= ~(0x04 << IntNo);
					if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for DMA%d\n", machine.time().as_double(), IntNo);
					handled=1;
				}

			/* check external interrupts */
			for (IntNo = 0; ((IntNo < 4) && !handled) ; IntNo++)
				if ((state->m_i186.intr.ext[IntNo] & 0x07) == Priority && (state->m_i186.intr.in_service & (0x10 << IntNo)))
				{
					state->m_i186.intr.in_service &= ~(0x10 << IntNo);
					if (LOG_INTERRUPTS) logerror("(%f) **** Got EOI for INT%d\n", machine.time().as_double(), IntNo);
					handled=1;
				}
		}
	}
	nimbus_recalculate_ints(machine);
}

/* Trigger an external interupt, optionally supplying the vector to take */
static void external_int(running_machine &machine, UINT16 intno, UINT8 vector)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	if (LOG_INTERRUPTS_EXT) logerror("generating external int %02X, vector %02X\n",intno,vector);

	// Only 4 external ints
	if(intno>3)
	{
		logerror("external_int() invalid external interupt no : 0x%02X (can only be 0..3)\n",intno);
		return;
	}

	// Only set external vector if cascade mode enabled, only valid for
	// int 0 & int 1
	if (intno<2)
	{
		if(state->m_i186.intr.ext[intno] & EXTINT_CTRL_CASCADE)
			state->m_i186.intr.ext_vector[intno]=vector;
	}

	// Turn on the requested request bit and handle interrupt
	state->m_i186.intr.request |= (0x010 << intno);
	update_interrupt_state(machine);
}

static void nimbus_recalculate_ints(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	if((state->m_iou_reg092 & DISK_INT_ENABLE) && state->m_nimbus_drives.int_ff)
	{
		state->m_nimbus_drives.int_ff=0;
		external_int(machine,0,EXTERNAL_INT_DISK);
	}
}

/*************************************
 *
 *  80186 internal timers
 *
 *************************************/

TIMER_CALLBACK_MEMBER(rmnimbus_state::internal_timer_int)
{
	int which = param;
	struct timer_state *t = &m_i186.timer[which];

	if (LOG_TIMER) logerror("Hit interrupt callback for timer %d\n", which);

	/* set the max count bit */
	t->control |= 0x0020;

	/* request an interrupt */
	if (t->control & 0x2000)
	{
		m_i186.intr.status |= 0x01 << which;
		update_interrupt_state(machine());
		if (LOG_TIMER) logerror("  Generating timer interrupt\n");
	}

	/* if we're continuous, reset */
	if (t->control & 0x0001)
	{
		int count = t->maxA ? t->maxA : 0x10000;
		t->int_timer->adjust((attotime::from_hz(2000000) * count), which);
		if (LOG_TIMER) logerror("  Repriming interrupt\n");
	}
	else
		t->int_timer->adjust(attotime::never, which);
}


static void internal_timer_sync(running_machine &machine, int which)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	struct timer_state *t = &state->m_i186.timer[which];

	/* if we have a timing timer running, adjust the count */
	if (t->time_timer_active)
	{
		attotime current_time = t->time_timer->elapsed();
		int net_clocks = ((current_time - t->last_time) * 2000000).seconds;
		t->last_time = current_time;

		/* set the max count bit if we passed the max */
		if ((int)t->count + net_clocks >= t->maxA)
			t->control |= 0x0020;

		/* set the new count */
		if (t->maxA != 0)
			t->count = (t->count + net_clocks) % t->maxA;
		else
			t->count = t->count + net_clocks;
	}
}


static void internal_timer_update(running_machine &machine,
									int which,
									int new_count,
									int new_maxA,
									int new_maxB,
									int new_control)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	struct timer_state *t = &state->m_i186.timer[which];
	int update_int_timer = 0;

	if (LOG_TIMER)
		logerror("internal_timer_update: %d, new_count=%d, new_maxA=%d, new_maxB=%d,new_control=%d\n",which,new_count,new_maxA,new_maxB,new_control);

	/* if we have a new count and we're on, update things */
	if (new_count != -1)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(machine, which);
			update_int_timer = 1;
		}
		t->count = new_count;
	}

	/* if we have a new max and we're on, update things */
	if (new_maxA != -1 && new_maxA != t->maxA)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(machine, which);
			update_int_timer = 1;
		}
		t->maxA = new_maxA;
		if (new_maxA == 0)
		{
				new_maxA = 0x10000;
		}
	}

	/* if we have a new max and we're on, update things */
	if (new_maxB != -1 && new_maxB != t->maxB)
	{
		if (t->control & 0x8000)
		{
			internal_timer_sync(machine, which);
			update_int_timer = 1;
		}

		t->maxB = new_maxB;

		if (new_maxB == 0)
		{
				new_maxB = 0x10000;
		}
	}


	/* handle control changes */
	if (new_control != -1)
	{
		int diff;

		/* merge back in the bits we don't modify */
		new_control = (new_control & ~0x1fc0) | (t->control & 0x1fc0);

		/* handle the /INH bit */
		if (!(new_control & 0x4000))
			new_control = (new_control & ~0x8000) | (t->control & 0x8000);
		new_control &= ~0x4000;

		/* check for control bits we don't handle */
		diff = new_control ^ t->control;
		if (diff & 0x001c)
			logerror("%05X:ERROR! -unsupported timer mode %04X\n",
				machine.device(MAINCPU_TAG)->safe_pc(), new_control);

		/* if we have real changes, update things */
		if (diff != 0)
		{

			/* if we're going off, make sure our timers are gone */
			if ((diff & 0x8000) && !(new_control & 0x8000))
			{
				/* compute the final count */
				internal_timer_sync(machine, which);

				/* nuke the timer and force the interrupt timer to be recomputed */
				t->time_timer->adjust(attotime::never, which);
				t->time_timer_active = 0;
				update_int_timer = 1;
			}

			/* if we're going on, start the timers running */
			else if ((diff & 0x8000) && (new_control & 0x8000))
			{
				/* start the timing */
				t->time_timer->adjust(attotime::never, which);
				t->time_timer_active = 1;
				update_int_timer = 1;
			}

			/* if something about the interrupt timer changed, force an update */
			if (!(diff & 0x8000) && (diff & 0x2000))
			{
				internal_timer_sync(machine, which);
				update_int_timer = 1;
			}
		}

		/* set the new control register */
		t->control = new_control;
	}

	/* update the interrupt timer */
	if (update_int_timer)
	{
		if ((t->control & 0x8000) && (t->control & 0x2000))
		{
			int diff = t->maxA - t->count;
			if (diff <= 0)
				diff += 0x10000;
			t->int_timer->adjust(attotime::from_hz(2000000) * diff, which);
			if (LOG_TIMER) logerror("Set interrupt timer for %d\n", which);
		}
		else
		{
			t->int_timer->adjust(attotime::never, which);
		}
	}
}



/*************************************
 *
 *  80186 internal DMA
 *
 *************************************/

TIMER_CALLBACK_MEMBER(rmnimbus_state::dma_timer_callback)
{
	int which = param;
	struct dma_state *d = &m_i186.dma[which];

	/* complete the status update */
	d->control &= ~0x0002;
	d->source += d->count;
	d->count = 0;

	/* check for interrupt generation */
	if (d->control & 0x0100)
	{
		if (LOG_DMA>1) logerror("DMA%d timer callback - requesting interrupt: count = %04X, source = %04X\n", which, d->count, d->source);
		m_i186.intr.request |= 0x04 << which;
		update_interrupt_state(machine());
	}
}


static void update_dma_control(running_machine &machine, int which, int new_control)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	struct dma_state *d = &state->m_i186.dma[which];
	int diff;

	/* handle the CHG bit */
	if (!(new_control & CHG_NOCHG))
		new_control = (new_control & ~ST_STOP) | (d->control & ST_STOP);
	new_control &= ~CHG_NOCHG;

	/* check for control bits we don't handle */
	diff = new_control ^ d->control;
	if ((LOG_DMA) && (diff & 0x6811))
		logerror("%05X:ERROR! - unsupported DMA mode %04X\n",
			machine.device(MAINCPU_TAG)->safe_pc(), new_control);
#if 0
	/* if we're going live, set a timer */
	if ((diff & 0x0002) && (new_control & 0x0002))
	{
		/* make sure the parameters meet our expectations */
		if ((new_control & 0xfe00) != 0x1600)
		{
			if (LOG_DMA) logerror("Unexpected DMA control %02X\n", new_control);
		}
		else if (/*!is_redline &&*/ ((d->dest & 1) || (d->dest & 0x3f) > 0x0b))
		{
			if (LOG_DMA) logerror("Unexpected DMA destination %02X\n", d->dest);
		}
		else if (/*is_redline && */ (d->dest & 0xf000) != 0x4000 && (d->dest & 0xf000) != 0x5000)
		{
			if (LOG_DMA) logerror("Unexpected DMA destination %02X\n", d->dest);
		}

		/* otherwise, set a timer */
		else
		{
			d->finished = 0;
		}
	}
#endif

	if (LOG_DMA) logerror("Initiated DMA %d - count = %04X, source = %04X, dest = %04X\n", which, d->count, d->source, d->dest);
	if (DEBUG_SET(DMA_BREAK))
		debugger_break(machine);

	/* set the new control register */
	d->control = new_control;
}

static void drq_callback(running_machine &machine, int which)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	struct dma_state *dma = &state->m_i186.dma[which];
	address_space &memory_space   = machine.device(MAINCPU_TAG)->memory().space(AS_PROGRAM);
	address_space &io_space       = machine.device(MAINCPU_TAG)->memory().space(AS_IO);

	UINT16  dma_word;
	UINT8   dma_byte;
	UINT8   incdec_size;

	if (LOG_DMA>1)
		logerror("Control=%04X, src=%05X, dest=%05X, count=%04X\n",dma->control,dma->source,dma->dest,dma->count);

	if(!(dma->control & ST_STOP))
	{
		logerror("%05X:ERROR! - drq%d with dma channel stopped\n",
		machine.device(MAINCPU_TAG)->safe_pc(), which);

		return;
	}

	address_space &dest_space = (dma->control & DEST_MIO) ? memory_space : io_space;
	address_space &src_space = (dma->control & SRC_MIO) ? memory_space : io_space;

	// Do the transfer
	if(dma->control & BYTE_WORD)
	{
		dma_word=src_space.read_word(dma->source);
		dest_space.write_word(dma->dest,dma_word);
		incdec_size=2;
	}
	else
	{
		dma_byte=src_space.read_byte(dma->source);
		dest_space.write_byte(dma->dest,dma_byte);
		incdec_size=1;
	}

	// Increment or Decrement destination ans source pointers as needed
	switch (dma->control & DEST_INCDEC_MASK)
	{
		case DEST_DECREMENT     : dma->dest -= incdec_size;
		case DEST_INCREMENT     : dma->dest += incdec_size;
	}

	switch (dma->control & SRC_INCDEC_MASK)
	{
		case SRC_DECREMENT     : dma->source -= incdec_size;
		case SRC_INCREMENT     : dma->source += incdec_size;
	}

	// decrement count
	dma->count -= 1;

	// Terminate if count is zero, and terminate flag set
	if((dma->control & TERMINATE_ON_ZERO) && (dma->count==0))
	{
		dma->control &= ~ST_STOP;
		if (LOG_DMA) logerror("DMA terminated\n");
	}

	// Interrupt if count is zero, and interrupt flag set
	if((dma->control & INTERRUPT_ON_ZERO) && (dma->count==0))
	{
		if (LOG_DMA>1) logerror("DMA%d - requesting interrupt: count = %04X, source = %04X\n", which, dma->count, dma->source);
		state->m_i186.intr.request |= 0x04 << which;
		update_interrupt_state(machine);
	}
}

/*-------------------------------------------------------------------------*/
/* Name: rmnimbus                                                            */
/* Desc: CPU - Initialize the 80186 CPU                                    */
/*-------------------------------------------------------------------------*/
static void nimbus_cpu_init(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	logerror("Machine reset\n");

	/* create timers here so they stick around */
	state->m_i186.timer[0].int_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(rmnimbus_state::internal_timer_int),state));
	state->m_i186.timer[1].int_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(rmnimbus_state::internal_timer_int),state));
	state->m_i186.timer[2].int_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(rmnimbus_state::internal_timer_int),state));
	state->m_i186.timer[0].time_timer = machine.scheduler().timer_alloc(FUNC_NULL);
	state->m_i186.timer[1].time_timer = machine.scheduler().timer_alloc(FUNC_NULL);
	state->m_i186.timer[2].time_timer = machine.scheduler().timer_alloc(FUNC_NULL);
	state->m_i186.dma[0].finish_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(rmnimbus_state::dma_timer_callback),state));
	state->m_i186.dma[1].finish_timer = machine.scheduler().timer_alloc(timer_expired_delegate(FUNC(rmnimbus_state::dma_timer_callback),state));
}

static void nimbus_cpu_reset(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	/* reset the interrupt state */
	state->m_i186.intr.priority_mask        = 0x0007;
	state->m_i186.intr.timer            = 0x000f;
	state->m_i186.intr.dma[0]           = 0x000f;
	state->m_i186.intr.dma[1]           = 0x000f;
	state->m_i186.intr.ext[0]           = 0x000f;
	state->m_i186.intr.ext[1]           = 0x000f;
	state->m_i186.intr.ext[2]           = 0x000f;
	state->m_i186.intr.ext[3]           = 0x000f;
	state->m_i186.intr.in_service       = 0x0000;

	/* External vectors by default to internal int 0/1 vectors */
	state->m_i186.intr.ext_vector[0]        = 0x000C;
	state->m_i186.intr.ext_vector[1]        = 0x000D;

	state->m_i186.intr.pending           = 0x0000;
	state->m_i186.intr.ack_mask          = 0x0000;
	state->m_i186.intr.request           = 0x0000;
	state->m_i186.intr.status            = 0x0000;
	state->m_i186.intr.poll_status       = 0x0000;

	logerror("CPU reset done\n");
}

READ16_MEMBER(rmnimbus_state::nimbus_i186_internal_port_r)
{
	int temp, which;

	switch (offset)
	{
		case 0x11:
			logerror("%05X:ERROR - read from 80186 EOI\n", space.device().safe_pc());
			break;

		case 0x12:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt poll\n", space.device().safe_pc());
			if (m_i186.intr.poll_status & 0x8000)
				int_callback(machine().device(MAINCPU_TAG), 0);
			return m_i186.intr.poll_status;

		case 0x13:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt poll status\n", space.device().safe_pc());
			return m_i186.intr.poll_status;

		case 0x14:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt mask\n", space.device().safe_pc());
			temp  = (m_i186.intr.timer  >> 3) & 0x01;
			temp |= (m_i186.intr.dma[0] >> 1) & 0x04;
			temp |= (m_i186.intr.dma[1] >> 0) & 0x08;
			temp |= (m_i186.intr.ext[0] << 1) & 0x10;
			temp |= (m_i186.intr.ext[1] << 2) & 0x20;
			temp |= (m_i186.intr.ext[2] << 3) & 0x40;
			temp |= (m_i186.intr.ext[3] << 4) & 0x80;
			return temp;

		case 0x15:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt priority mask\n", space.device().safe_pc());
			return m_i186.intr.priority_mask;

		case 0x16:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt in-service\n", space.device().safe_pc());
			return m_i186.intr.in_service;

		case 0x17:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt request\n", space.device().safe_pc());
			temp = m_i186.intr.request & ~0x0001;
			if (m_i186.intr.status & 0x0007)
				temp |= 1;
			return temp;

		case 0x18:
			if (LOG_PORTS) logerror("%05X:read 80186 interrupt status\n", space.device().safe_pc());
			return m_i186.intr.status;

		case 0x19:
			if (LOG_PORTS) logerror("%05X:read 80186 timer interrupt control\n", space.device().safe_pc());
			return m_i186.intr.timer;

		case 0x1a:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA 0 interrupt control\n", space.device().safe_pc());
			return m_i186.intr.dma[0];

		case 0x1b:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA 1 interrupt control\n", space.device().safe_pc());
			return m_i186.intr.dma[1];

		case 0x1c:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 0 interrupt control\n", space.device().safe_pc());
			return m_i186.intr.ext[0];

		case 0x1d:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 1 interrupt control\n", space.device().safe_pc());
			return m_i186.intr.ext[1];

		case 0x1e:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 2 interrupt control\n", space.device().safe_pc());
			return m_i186.intr.ext[2];

		case 0x1f:
			if (LOG_PORTS) logerror("%05X:read 80186 INT 3 interrupt control\n", space.device().safe_pc());
			return m_i186.intr.ext[3];

		case 0x28:
		case 0x2c:
		case 0x30:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d count\n", space.device().safe_pc(), (offset - 0x28) / 4);
			which = (offset - 0x28) / 4;
			if (!(offset & 1))
				internal_timer_sync(machine(), which);
			return m_i186.timer[which].count;

		case 0x29:
		case 0x2d:
		case 0x31:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d max A\n", space.device().safe_pc(), (offset - 0x29) / 4);
			which = (offset - 0x29) / 4;
			return m_i186.timer[which].maxA;

		case 0x2a:
		case 0x2e:
			logerror("%05X:read 80186 Timer %d max B\n", space.device().safe_pc(), (offset - 0x2a) / 4);
			which = (offset - 0x2a) / 4;
			return m_i186.timer[which].maxB;

		case 0x2b:
		case 0x2f:
		case 0x33:
			if (LOG_PORTS) logerror("%05X:read 80186 Timer %d control\n", space.device().safe_pc(), (offset - 0x2b) / 4);
			which = (offset - 0x2b) / 4;
			return m_i186.timer[which].control;

		case 0x50:
			if (LOG_PORTS) logerror("%05X:read 80186 upper chip select\n", space.device().safe_pc());
			return m_i186.mem.upper;

		case 0x51:
			if (LOG_PORTS) logerror("%05X:read 80186 lower chip select\n", space.device().safe_pc());
			return m_i186.mem.lower;

		case 0x52:
			if (LOG_PORTS) logerror("%05X:read 80186 peripheral chip select\n", space.device().safe_pc());
			return m_i186.mem.peripheral;

		case 0x53:
			if (LOG_PORTS) logerror("%05X:read 80186 middle chip select\n", space.device().safe_pc());
			return m_i186.mem.middle;

		case 0x54:
			if (LOG_PORTS) logerror("%05X:read 80186 middle P chip select\n", space.device().safe_pc());
			return m_i186.mem.middle_size;

		case 0x60:
		case 0x68:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d lower source address\n", space.device().safe_pc(), (offset - 0x60) / 8);
			which = (offset - 0x60) / 8;
			return m_i186.dma[which].source;

		case 0x61:
		case 0x69:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d upper source address\n", space.device().safe_pc(), (offset - 0x61) / 8);
			which = (offset - 0x61) / 8;
			return m_i186.dma[which].source >> 16;

		case 0x62:
		case 0x6a:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d lower dest address\n", space.device().safe_pc(), (offset - 0x62) / 8);
			which = (offset - 0x62) / 8;
			return m_i186.dma[which].dest;

		case 0x63:
		case 0x6b:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d upper dest address\n", space.device().safe_pc(), (offset - 0x63) / 8);
			which = (offset - 0x63) / 8;
			return m_i186.dma[which].dest >> 16;

		case 0x64:
		case 0x6c:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d transfer count\n", space.device().safe_pc(), (offset - 0x64) / 8);
			which = (offset - 0x64) / 8;
			return m_i186.dma[which].count;

		case 0x65:
		case 0x6d:
			if (LOG_PORTS) logerror("%05X:read 80186 DMA%d control\n", space.device().safe_pc(), (offset - 0x65) / 8);
			which = (offset - 0x65) / 8;
			return m_i186.dma[which].control;

		default:
			logerror("%05X:read 80186 port %02X\n", space.device().safe_pc(), offset);
			break;
	}
	return 0x00;
}

/*************************************
 *
 *  80186 internal I/O writes
 *
 *************************************/

WRITE16_MEMBER(rmnimbus_state::nimbus_i186_internal_port_w)
{
	int temp, which, data16 = data;

	switch (offset)
	{
		case 0x11:
			if (LOG_PORTS) logerror("%05X:80186 EOI = %04X\n", space.device().safe_pc(), data16);
			handle_eoi(machine(),0x8000);
			update_interrupt_state(machine());
			break;

		case 0x12:
			logerror("%05X:ERROR - write to 80186 interrupt poll = %04X\n", space.device().safe_pc(), data16);
			break;

		case 0x13:
			logerror("%05X:ERROR - write to 80186 interrupt poll status = %04X\n", space.device().safe_pc(), data16);
			break;

		case 0x14:
			if (LOG_PORTS) logerror("%05X:80186 interrupt mask = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.timer  = (m_i186.intr.timer  & ~0x08) | ((data16 << 3) & 0x08);
			m_i186.intr.dma[0] = (m_i186.intr.dma[0] & ~0x08) | ((data16 << 1) & 0x08);
			m_i186.intr.dma[1] = (m_i186.intr.dma[1] & ~0x08) | ((data16 << 0) & 0x08);
			m_i186.intr.ext[0] = (m_i186.intr.ext[0] & ~0x08) | ((data16 >> 1) & 0x08);
			m_i186.intr.ext[1] = (m_i186.intr.ext[1] & ~0x08) | ((data16 >> 2) & 0x08);
			m_i186.intr.ext[2] = (m_i186.intr.ext[2] & ~0x08) | ((data16 >> 3) & 0x08);
			m_i186.intr.ext[3] = (m_i186.intr.ext[3] & ~0x08) | ((data16 >> 4) & 0x08);
			update_interrupt_state(machine());
			break;

		case 0x15:
			if (LOG_PORTS) logerror("%05X:80186 interrupt priority mask = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.priority_mask = data16 & 0x0007;
			update_interrupt_state(machine());
			break;

		case 0x16:
			if (LOG_PORTS) logerror("%05X:80186 interrupt in-service = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.in_service = data16 & 0x00ff;
			update_interrupt_state(machine());
			break;

		case 0x17:
			if (LOG_PORTS) logerror("%05X:80186 interrupt request = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.request = (m_i186.intr.request & ~0x00c0) | (data16 & 0x00c0);
			update_interrupt_state(machine());
			break;

		case 0x18:
			if (LOG_PORTS) logerror("%05X:WARNING - wrote to 80186 interrupt status = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.status = (m_i186.intr.status & ~0x8007) | (data16 & 0x8007);
			update_interrupt_state(machine());
			break;

		case 0x19:
			if (LOG_PORTS) logerror("%05X:80186 timer interrupt contol = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.timer = data16 & 0x000f;
			break;

		case 0x1a:
			if (LOG_PORTS) logerror("%05X:80186 DMA 0 interrupt control = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.dma[0] = data16 & 0x000f;
			break;

		case 0x1b:
			if (LOG_PORTS) logerror("%05X:80186 DMA 1 interrupt control = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.dma[1] = data16 & 0x000f;
			break;

		case 0x1c:
			if (LOG_PORTS) logerror("%05X:80186 INT 0 interrupt control = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.ext[0] = data16 & 0x007f;
			break;

		case 0x1d:
			if (LOG_PORTS) logerror("%05X:80186 INT 1 interrupt control = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.ext[1] = data16 & 0x007f;
			break;

		case 0x1e:
			if (LOG_PORTS) logerror("%05X:80186 INT 2 interrupt control = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.ext[2] = data16 & 0x001f;
			break;

		case 0x1f:
			if (LOG_PORTS) logerror("%05X:80186 INT 3 interrupt control = %04X\n", space.device().safe_pc(), data16);
			m_i186.intr.ext[3] = data16 & 0x001f;
			break;

		case 0x28:
		case 0x2c:
		case 0x30:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d count = %04X\n", space.device().safe_pc(), (offset - 0x28) / 4, data16);
			which = (offset - 0x28) / 4;
			internal_timer_update(machine(),which, data16, -1, -1, -1);
			break;

		case 0x29:
		case 0x2d:
		case 0x31:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d max A = %04X\n", space.device().safe_pc(), (offset - 0x29) / 4, data16);
			which = (offset - 0x29) / 4;
			internal_timer_update(machine(),which, -1, data16, -1, -1);
			break;

		case 0x2a:
		case 0x2e:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d max B = %04X\n", space.device().safe_pc(), (offset - 0x2a) / 4, data16);
			which = (offset - 0x2a) / 4;
			internal_timer_update(machine(),which, -1, -1, data16, -1);
			break;

		case 0x2b:
		case 0x2f:
		case 0x33:
			if (LOG_PORTS) logerror("%05X:80186 Timer %d control = %04X\n", space.device().safe_pc(), (offset - 0x2b) / 4, data16);
			which = (offset - 0x2b) / 4;
			internal_timer_update(machine(),which, -1, -1, -1, data16);
			break;

		case 0x50:
			if (LOG_PORTS) logerror("%05X:80186 upper chip select = %04X\n", space.device().safe_pc(), data16);
			m_i186.mem.upper = data16 | 0xc038;
			break;

		case 0x51:
			if (LOG_PORTS) logerror("%05X:80186 lower chip select = %04X\n", space.device().safe_pc(), data16);
			m_i186.mem.lower = (data16 & 0x3fff) | 0x0038; printf("%X",m_i186.mem.lower);
			break;

		case 0x52:
			if (LOG_PORTS) logerror("%05X:80186 peripheral chip select = %04X\n", space.device().safe_pc(), data16);
			m_i186.mem.peripheral = data16 | 0x0038;
			break;

		case 0x53:
			if (LOG_PORTS) logerror("%05X:80186 middle chip select = %04X\n", space.device().safe_pc(), data16);
			m_i186.mem.middle = data16 | 0x01f8;
			break;

		case 0x54:
			if (LOG_PORTS) logerror("%05X:80186 middle P chip select = %04X\n", space.device().safe_pc(), data16);
			m_i186.mem.middle_size = data16 | 0x8038;

			/* we need to do this at a time when the I86 context is swapped in */
			/* this register is generally set once at startup and never again, so it's a good */
			/* time to set it up */
			space.device().execute().set_irq_acknowledge_callback(int_callback);
			break;

		case 0x60:
		case 0x68:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d lower source address = %04X\n", space.device().safe_pc(), (offset - 0x60) / 8, data16);
			which = (offset - 0x60) / 8;
			m_i186.dma[which].source = (m_i186.dma[which].source & ~0x0ffff) | (data16 & 0x0ffff);
			break;

		case 0x61:
		case 0x69:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d upper source address = %04X\n", space.device().safe_pc(), (offset - 0x61) / 8, data16);
			which = (offset - 0x61) / 8;
			m_i186.dma[which].source = (m_i186.dma[which].source & ~0xf0000) | ((data16 << 16) & 0xf0000);
			break;

		case 0x62:
		case 0x6a:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d lower dest address = %04X\n", space.device().safe_pc(), (offset - 0x62) / 8, data16);
			which = (offset - 0x62) / 8;
			m_i186.dma[which].dest = (m_i186.dma[which].dest & ~0x0ffff) | (data16 & 0x0ffff);
			break;

		case 0x63:
		case 0x6b:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d upper dest address = %04X\n", space.device().safe_pc(), (offset - 0x63) / 8, data16);
			which = (offset - 0x63) / 8;
			m_i186.dma[which].dest = (m_i186.dma[which].dest & ~0xf0000) | ((data16 << 16) & 0xf0000);
			break;

		case 0x64:
		case 0x6c:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d transfer count = %04X\n", space.device().safe_pc(), (offset - 0x64) / 8, data16);
			which = (offset - 0x64) / 8;
			m_i186.dma[which].count = data16;
			break;

		case 0x65:
		case 0x6d:
			if (LOG_PORTS) logerror("%05X:80186 DMA%d control = %04X\n", space.device().safe_pc(), (offset - 0x65) / 8, data16);
			which = (offset - 0x65) / 8;
			update_dma_control(machine(), which, data16);
			break;

		case 0x7f:
			if (LOG_PORTS) logerror("%05X:80186 relocation register = %04X\n", space.device().safe_pc(), data16);

			/* we assume here there that this doesn't happen too often */
			/* plus, we can't really remove the old memory range, so we also assume that it's */
			/* okay to leave us mapped where we were */
			temp = (data16 & 0x0fff) << 8;
			if (data16 & 0x1000)
			{
				machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM).install_read_handler(temp, temp + 0xff, read16_delegate(FUNC(rmnimbus_state::nimbus_i186_internal_port_r),this));
				machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM).install_write_handler(temp, temp + 0xff, write16_delegate(FUNC(rmnimbus_state::nimbus_i186_internal_port_w),this));
			}
			else
			{
				temp &= 0xffff;
				machine().device(MAINCPU_TAG)->memory().space(AS_IO).install_read_handler(temp, temp + 0xff, read16_delegate(FUNC(rmnimbus_state::nimbus_i186_internal_port_r),this));
				machine().device(MAINCPU_TAG)->memory().space(AS_IO).install_write_handler(temp, temp + 0xff, write16_delegate(FUNC(rmnimbus_state::nimbus_i186_internal_port_w),this));
			}
			break;

		default:
			logerror("%05X:80186 port %02X = %04X\n", space.device().safe_pc(), offset, data16);
			break;
	}
}

void rmnimbus_state::machine_reset()
{
	/* CPU */
	nimbus_cpu_reset(machine());
	iou_reset(machine());
	fdc_reset(machine());
	hdc_reset(machine());
	keyboard_reset(machine());
	pc8031_reset(machine());
	rmni_sound_reset(machine());
	memory_reset(machine());
	mouse_js_reset(machine());
}

DRIVER_INIT_MEMBER(rmnimbus_state,nimbus)
{
}

void rmnimbus_state::machine_start()
{
	/* init cpu */
	nimbus_cpu_init(machine());

	m_keyboard.keyscan_timer=machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(rmnimbus_state::keyscan_callback),this));
	m_nimbus_mouse.m_mouse_timer=machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(rmnimbus_state::mouse_callback),this));

	/* setup debug commands */
	if (machine().debug_flags & DEBUG_FLAG_ENABLED)
	{
		debug_console_register_command(machine(), "nimbus_irq", CMDFLAG_NONE, 0, 0, 2, execute_debug_irq);
		debug_console_register_command(machine(), "nimbus_intmasks", CMDFLAG_NONE, 0, 0, 0, execute_debug_intmasks);
		debug_console_register_command(machine(), "nimbus_debug", CMDFLAG_NONE, 0, 0, 1, nimbus_debug);

		/* set up the instruction hook */
		machine().device(MAINCPU_TAG)->debug()->set_instruction_hook(instruction_hook);
	}

	m_debug_machine=DEBUG_NONE;
}

static void execute_debug_irq(running_machine &machine, int ref, int params, const char *param[])
{
	int IntNo;
	int Vector;

	if(params>1)
	{
		sscanf(param[0],"%X",&IntNo);
		sscanf(param[1],"%X",&Vector);

		debug_console_printf(machine,"triggering IRQ%d, Vector=%02X\n",IntNo,Vector);
		external_int(machine,IntNo,Vector);
	}
	else
	{
		debug_console_printf(machine,"Error, you must supply an intno and vector to trigger\n");
	}
}


static void execute_debug_intmasks(running_machine &machine, int ref, int params, const char *param[])
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	int IntNo;

	debug_console_printf(machine,"i186.intr.priority_mask=%4X\n",state->m_i186.intr.priority_mask);
	for(IntNo=0; IntNo<4; IntNo++)
	{
		debug_console_printf(machine,"extInt%d mask=%4X\n",IntNo,state->m_i186.intr.ext[IntNo]);
	}

	debug_console_printf(machine,"i186.intr.request   = %04X\n",state->m_i186.intr.request);
	debug_console_printf(machine,"i186.intr.ack_mask  = %04X\n",state->m_i186.intr.ack_mask);
	debug_console_printf(machine,"i186.intr.in_service= %04X\n",state->m_i186.intr.in_service);
}

static void nimbus_debug(running_machine &machine, int ref, int params, const char *param[])
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	if(params>0)
	{
		sscanf(param[0],"%d",&state->m_debug_machine);
	}
	else
	{
		debug_console_printf(machine,"Error usage : nimbus_debug <debuglevel>\n");
		debug_console_printf(machine,"Current debuglevel=%02X\n",state->m_debug_machine);
	}
}

/*-----------------------------------------------
    instruction_hook - per-instruction hook
-----------------------------------------------*/

static int instruction_hook(device_t &device, offs_t curpc)
{
	rmnimbus_state  *state = device.machine().driver_data<rmnimbus_state>();
	address_space   &space = device.memory().space(AS_PROGRAM);
	UINT8           *addr_ptr;

	addr_ptr = (UINT8*)space.get_read_ptr(curpc);

	if ((addr_ptr !=NULL) && (addr_ptr[0]==0xCD))
	{
		if(DEBUG_SET(DECODE_BIOS) && (addr_ptr[1]==0xF0))
		{
			if(DEBUG_SET(DECODE_BIOS_RAW))
				decode_subbios(&device,curpc,1);
			else
				decode_subbios(&device,curpc,0);
		}

		if(DEBUG_SET(DECODE_DOS21) && (addr_ptr[1]==0x21))
			decode_dos21(&device,curpc);
	}

	return 0;
}

#define set_type(type_name)     sprintf(type_str,type_name)
#define set_drv(drv_name)       sprintf(drv_str,drv_name)
#define set_func(func_name)     sprintf(func_str,func_name)

static void decode_subbios(device_t *device,offs_t pc, UINT8 raw_flag)
{
	char    type_str[80];
	char    drv_str[80];
	char    func_str[80];

	void (*dump_dssi)(device_t *,UINT16, UINT16 ,UINT8) = NULL;

	device_t *cpu = device->machine().device(MAINCPU_TAG);

	UINT16  ax = cpu->state().state_int(I8086_AX);
	UINT16  bx = cpu->state().state_int(I8086_BX);
	UINT16  cx = cpu->state().state_int(I8086_CX);
	UINT16  ds = cpu->state().state_int(I8086_DS);
	UINT16  si = cpu->state().state_int(I8086_SI);

	// *** TEMP Don't show f_enquire_display_line calls !
	if((cx==6) && (ax==43))
		return;
	// *** END TEMP

	if(!raw_flag)
	{
		logerror("=======================================================================\n");
		logerror("Sub-bios call at %08X, AX=%04X, BX=%04X, CX=%04X, DS:SI=%04X:%04X\n",pc,ax,bx,cx,ds,si);
	}

	set_type("invalid");
	set_drv("invalid");
	set_func("invalid");

	switch (cx)
	{
		case 0   :
		{
			set_type("t_mummu");
			set_drv("d_mummu");

			switch (ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_add_type_code"); break;
				case 2  : set_func("f_del_typc_code"); break;
				case 3  : set_func("f_get_TCB"); break;
				case 4  : set_func("f_add_driver_code"); break;
				case 5  : set_func("f_del_driver_code"); break;
				case 6  : set_func("f_get_DCB"); break;
				case 7  : set_func("f_get_copyright"); break;
			}
		}; break;

		case 1   :
		{
			set_type("t_character");
			set_drv("d_printer");

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_get_output_status"); break;
				case 2  : set_func("f_output_character"); break;
				case 3  : set_func("f_get_input_status"); break;
				case 4  : set_func("f_get_and_remove"); break;
				case 5  : set_func("f_get_no_remove"); break;
				case 6  : set_func("f_get_last_and_remove"); break;
				case 7  : set_func("f_get_last_no_remove"); break;
				case 8  : set_func("f_set_IO_parameters"); break;
			}
		}; break;

		case 2   :
		{
			set_type("t_disk");

			switch(bx)
			{
				case 0  : set_drv("d_floppy"); break;
				case 1  : set_drv("d_winchester"); break;
				case 2  : set_drv("d_tape"); break;
				case 3  : set_drv("d_rompack"); break;
				case 4  : set_drv("d_eeprom"); break;
			}

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_initialise_unit"); break;
				case 2  : set_func("f_pseudo_init_unit"); break;
				case 3  : set_func("f_get_device_status"); break;
				case 4  : set_func("f_read_n_sectors"); dump_dssi=&decode_dssi_f_rw_sectors; break;
				case 5  : set_func("f_write_n_sectors"); dump_dssi=&decode_dssi_f_rw_sectors;  break;
				case 6  : set_func("f_verify_n_sectors"); break;
				case 7  : set_func("f_media_check"); break;
				case 8  : set_func("f_recalibrate"); break;
				case 9  : set_func("f_motors_off"); break;
			}
			dump_dssi=&decode_dssi_f_rw_sectors;

		}; break;

		case 3   :
		{
			set_type("t_piconet");
			set_drv("d_piconet");

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_get_slave_status"); break;
				case 2  : set_func("f_get_slave_map"); break;
				case 3  : set_func("f_change_slave_addr"); break;
				case 4  : set_func("f_read_slave_control"); break;
				case 5  : set_func("f_write_slave_control"); break;
				case 6  : set_func("f_send_data_byte"); break;
				case 7  : set_func("f_request_data_byte"); break;
				case 8  : set_func("f_send_data_block"); break;
				case 9  : set_func("f_request_data_block"); break;
				case 10 : set_func("f_reset_slave"); break;

			}
		}; break;

		case 4   :
		{
			set_type("t_tick");
			set_drv("d_tick");

			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_ticks_per_second"); break;
				case 2  : set_func("f_link_tick_routine"); break;
				case 3  : set_func("f_unlink_tick_routine"); break;
			}
		}; break;

		case 5   :
		{
			set_type("t_graphics_input");

			switch(bx)
			{
				case 0  : set_drv("d_mouse"); break;
				case 1  : set_drv("d_joystick_1"); break;
				case 2  : set_drv("d_joystick_2"); break;
			}


			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
				case 1  : set_func("f_graphics_input_cold_start"); break;
				case 2  : set_func("f_graphics_input_device_off"); break;
				case 3  : set_func("f_return_button_status"); break;
				case 4  : set_func("f_return_switch_and_button_stat"); break;
				case 5  : set_func("f_start_tracking"); break;
				case 6  : set_func("f_stop_tracking"); break;
				case 7  : set_func("f_enquire_position"); break;
				case 8  : set_func("f_set_position"); break;

				case 10 : set_func("f_return_button_press_info"); break;
				case 11 : set_func("f_return_button_release_info"); break;
				case 12 : set_func("f_set_gain/f_set_squeaks_per_pixel_ratio"); break;
				case 13 : set_func("f_enquire_graphics_in_misc_data"); break;
			}
		}; break;

		case 6   :
		{
			set_type("t_graphics_output");
			set_drv("d_ngc_screen");

			switch(ax)
			{
				case 0  : set_func("f_get_version_number");                 break;
				case 1  : set_func("f_graphics_output_cold_start");         break;
				case 2  : set_func("f_graphics_output_warm_start");         break;
				case 3  : set_func("f_graphics_output_off");                break;
				case 4  : set_func("f_reinit_graphics_output");             break;
				case 5  : set_func("f_polymarker");                         break;
				case 6  : set_func("f_polyline"); dump_dssi=&decode_dssi_f_fill_area;   break;
				case 7  : set_func("f_fill_area"); dump_dssi=&decode_dssi_f_fill_area; break;
				case 8  : set_func("f_flood_fill_area"); break;
				case 9  : set_func("f_plot_character_string"); dump_dssi=&decode_dssi_f_plot_character_string; break;
				case 10 : set_func("f_define_graphics_clipping_area"); break;
				case 11 : set_func("f_enquire_clipping_area_limits"); break;
				case 12 : set_func("f_select_graphics_clipping_area"); break;
				case 13 : set_func("f_enq_selctd_graphics_clip_area"); break;
				case 14 : set_func("f_set_clt_element"); break;
				case 15 : set_func("f_enquire_clt_element"); break;
				case 16 : set_func("f_set_new_clt"); dump_dssi=&decode_dssi_f_set_new_clt; break;
				case 17 : set_func("f_enquire_clt_contents"); break;
				case 18 : set_func("f_define_dithering_pattern"); break;
				case 19 : set_func("f_enquire_dithering_pattern"); break;
				case 20 : set_func("f_draw_sprite"); break;
				case 21 : set_func("f_move_sprite"); break;
				case 22 : set_func("f_erase_sprite"); break;
				case 23 : set_func("f_read_pixel"); break;
				case 24 : set_func("f_read_to_limit"); break;
				case 25 : set_func("f_read_area_pixel"); break;
				case 26 : set_func("f_write_area_pixel"); break;
				case 27 : set_func("f_copy_area_pixel"); break;

				case 29 : set_func("f_read_area_word"); break;
				case 30 : set_func("f_write_area_word"); break;
				case 31 : set_func("f_copy_area_word"); break;
				case 32 : set_func("f_swap_area_word"); break;
				case 33 : set_func("f_set_border_colour"); break;
				case 34 : set_func("f_enquire_border_colour"); break;
				case 35 : set_func("f_enquire_miscellaneous_data"); break;
				case 36  : set_func("f_circle"); break;

				case 38 : set_func("f_arc_of_ellipse"); break;
				case 39 : set_func("f_isin"); break;
				case 40 : set_func("f_icos"); break;
				case 41 : set_func("f_define_hatching_pattern"); break;
				case 42 : set_func("f_enquire_hatching_pattern"); break;
				case 43 : set_func("f_enquire_display_line"); break;
				case 44 : set_func("f_plonk_logo"); break;
			}
		}; break;

		case 7   :
		{
			set_type("t_zend");
			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
			}
		}; break;

		case 8   :
		{
			set_type("t_zep");
			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
			}
		}; break;

		case 9   :
		{
			set_type("t_raw_console");

			switch(bx)
			{
				case 0  :
				{
					set_drv("d_screen");

					switch(ax)
					{
						case 0  : set_func("f_get_version_number"); break;
						case 1  : set_func("f_plonk_char"); dump_dssi=decode_dssi_f_plonk_char; break;
						case 2  : set_func("f_plonk_cursor"); break;
						case 3  : set_func("f_kill_cursor"); break;
						case 4  : set_func("f_scroll"); break;
						case 5  : set_func("f_width"); dump_dssi=decode_dssi_generic;break;
						case 6  : set_func("f_get_char_set"); break;
						case 7  : set_func("f_set_char_set"); break;
						case 8  : set_func("f_reset_char_set"); break;
						case 9  : set_func("f_set_plonk_parameters"); break;
						case 10 : set_func("f_set_cursor_flash_rate"); break;
					}
				}; break;

				case 1  :
				{
					set_drv("d_keyboard");

					switch(ax)
					{
						case 0  : set_func("f_get_version_number"); break;
						case 1  : set_func("f_init_keyboard"); break;
						case 2  : set_func("f_get_last_key_code"); break;
						case 3  : set_func("f_get_bitmap"); break;
					}
				}; break;
			}
		}; break;

		case 10   :
		{

			set_type("t_acoustics");

			switch(bx)
			{
				case 0  :
				{
					set_drv("d_sound");

					switch(ax)
					{
						case 0  : set_func("f_get_version_number"); break;
						case 1  : set_func("f_sound_enable"); break;
						case 2  : set_func("f_play_note"); break;
						case 3  : set_func("f_get_queue_status"); break;
					}
				}; break;

				case 1  :
				{
					set_drv("d_voice");

					switch(ax)
					{
						case 0  : set_func("f_get_version_number"); break;
						case 1  : set_func("f_talk"); break;
						case 2  : set_func("f_wait_and_talk"); break;
						case 3  : set_func("f_test_talking"); break;
					}
				}
			}
		}; break;

		case 11   :
		{
			set_type("t_hard_sums");
			switch(ax)
			{
				case 0  : set_func("f_get_version_number"); break;
			}
		}; break;
	}

	if(raw_flag)
	{
		if(dump_dssi!=NULL)
			dump_dssi(device,ds,si,raw_flag);
	}
	else
	{
		logerror("Type=%s, Driver=%s, Function=%s\n",type_str,drv_str,func_str);

		if(dump_dssi!=NULL)
			dump_dssi(device,ds,si,raw_flag);
		logerror("=======================================================================\n");
	}
}

static void *get_dssi_ptr(address_space &space, UINT16   ds, UINT16 si)
{
	int             addr;

	addr=((ds<<4)+si);
//    OUTPUT_SEGOFS("DS:SI",ds,si);

	return space.get_read_ptr(addr);
}

static void decode_dssi_generic(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	address_space &space = device->machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM);
	UINT16  *params;
	int     count;

	if(raw_flag)
		return;

	params=(UINT16  *)get_dssi_ptr(space,ds,si);

	for(count=0; count<10; count++)
		logerror("%04X ",params[count]);

	logerror("\n");
}


static void decode_dssi_f_fill_area(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	address_space &space = device->machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM);

	UINT16          *addr_ptr;
	t_area_params   *area_params;
	t_nimbus_brush  *brush;
	int             cocount;

	area_params = (t_area_params   *)get_dssi_ptr(space,ds,si);

	if (!raw_flag)
		OUTPUT_SEGOFS("SegBrush:OfsBrush",area_params->seg_brush,area_params->ofs_brush);

	brush=(t_nimbus_brush  *)space.get_read_ptr(LINEAR_ADDR(area_params->seg_brush,area_params->ofs_brush));

	if(raw_flag)
	{
		logerror("\tdw\t%04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X, ",
					brush->style,brush->style_index,brush->colour1,brush->colour2,
					brush->transparency,brush->boundary_spec,brush->boundary_colour,brush->save_colour,
					area_params->count);
	}
	else
	{
		logerror("Brush params\n");
		logerror("Style=%04X,          StyleIndex=%04X\n",brush->style,brush->style_index);
		logerror("Colour1=%04X,        Colour2=%04X\n",brush->colour1,brush->colour2);
		logerror("transparency=%04X,   boundry_spec=%04X\n",brush->transparency,brush->boundary_spec);
		logerror("boundry colour=%04X, save colour=%04X\n",brush->boundary_colour,brush->save_colour);


		OUTPUT_SEGOFS("SegData:OfsData",area_params->seg_data,area_params->ofs_data);
	}

	addr_ptr = (UINT16 *)space.get_read_ptr(LINEAR_ADDR(area_params->seg_data,area_params->ofs_data));
	for(cocount=0; cocount < area_params->count; cocount++)
	{
		if(raw_flag)
		{
			if(cocount!=(area_params->count-1))
				logerror("%04X, %04X, ",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
			else
				logerror("%04X, %04X ",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
		}
		else
			logerror("x=%d y=%d\n",addr_ptr[cocount*2],addr_ptr[(cocount*2)+1]);
	}

	if(raw_flag)
		logerror("\n");
}

static void decode_dssi_f_plot_character_string(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	address_space &space = device->machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM);

	UINT8                   *char_ptr;
	t_plot_string_params    *plot_string_params;
	int                     charno;

	if(raw_flag)
		return;

	plot_string_params=(t_plot_string_params   *)get_dssi_ptr(space,ds,si);

	OUTPUT_SEGOFS("SegFont:OfsFont",plot_string_params->seg_font,plot_string_params->ofs_font);
	OUTPUT_SEGOFS("SegData:OfsData",plot_string_params->seg_data,plot_string_params->ofs_data);

	logerror("x=%d, y=%d, length=%d\n",plot_string_params->x,plot_string_params->y,plot_string_params->length);

	char_ptr=(UINT8*)space.get_read_ptr(LINEAR_ADDR(plot_string_params->seg_data,plot_string_params->ofs_data));

	if (plot_string_params->length==0xFFFF)
		logerror("%s",char_ptr);
	else
		for(charno=0;charno<plot_string_params->length;charno++)
			logerror("%c",char_ptr[charno]);

	logerror("\n");
}

static void decode_dssi_f_set_new_clt(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	address_space &space = device->machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM);
	UINT16  *new_colours;
	int     colour;
	new_colours=(UINT16  *)get_dssi_ptr(space,ds,si);

	if(raw_flag)
		return;

	OUTPUT_SEGOFS("SegColours:OfsColours",ds,si);

	for(colour=0;colour<16;colour++)
		logerror("colour #%02X=%04X\n",colour,new_colours[colour]);

}

static void decode_dssi_f_plonk_char(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	address_space &space = device->machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM);
	UINT16  *params;
	params=(UINT16  *)get_dssi_ptr(space,ds,si);

	if(raw_flag)
		return;

	OUTPUT_SEGOFS("SegParams:OfsParams",ds,si);

	logerror("plonked_char=%c\n",params[0]);
}

static void decode_dssi_f_rw_sectors(device_t *device,UINT16  ds, UINT16 si, UINT8 raw_flag)
{
	address_space &space = device->machine().device(MAINCPU_TAG)->memory().space(AS_PROGRAM);
	UINT16  *params;
	int     param_no;

	if(raw_flag)
		return;

	params=(UINT16  *)get_dssi_ptr(space,ds,si);

	for(param_no=0;param_no<16;param_no++)
		logerror("%04X ",params[param_no]);

	logerror("\n");
}

static void decode_dos21(device_t *device,offs_t pc)
{
	device_t *cpu = device->machine().device(MAINCPU_TAG);

	UINT16  ax = cpu->state().state_int(I8086_AX);
	UINT16  bx = cpu->state().state_int(I8086_BX);
	UINT16  cx = cpu->state().state_int(I8086_CX);
	UINT16  dx = cpu->state().state_int(I8086_DX);
	UINT16  cs = cpu->state().state_int(I8086_CS);
	UINT16  ds = cpu->state().state_int(I8086_DS);
	UINT16  es = cpu->state().state_int(I8086_ES);
	UINT16  ss = cpu->state().state_int(I8086_SS);

	UINT16  si = cpu->state().state_int(I8086_SI);
	UINT16  di = cpu->state().state_int(I8086_DI);
	UINT16  bp = cpu->state().state_int(I8086_BP);

	logerror("=======================================================================\n");
	logerror("DOS Int 0x21 call at %05X\n",pc);
	logerror("AX=%04X, BX=%04X, CX=%04X, DX=%04X\n",ax,bx,cx,dx);
	logerror("CS=%04X, DS=%04X, ES=%04X, SS=%04X\n",cs,ds,es,ss);
	logerror("SI=%04X, DI=%04X, BP=%04X\n",si,di,bp);
	logerror("=======================================================================\n");

	if((ax & 0xff00)==0x0900)
		debugger_break(device->machine());
}


/*
    The Nimbus has 3 banks of memory each of which can be either 16x4164 or 16x41256 giving
    128K or 512K per bank. These banks are as follows :

    bank0   on nimbus motherboard.
    bank1   first half of expansion card.
    bank2   second half of expansion card.

    The valid combinations are :

    bank0       bank1       bank2       total
    128K                                128K
    128K        128K                    256K
    128K        128K        128K        384K
    128K        512K                    640K (1)
    512K        128K                    640K (2)
    512K        512K                    1024K
    512K        512K        512K        1536K

    It will be noted that there are two possible ways of getting 640K, we emulate method 2
    (above).

    To allow for the greatest flexibility, the Nimbus allows 4 methods of mapping the
    banks of ram into the 1M addressable by the 81086.

    With only 128K banks present, they are mapped into the first 3 blocks of 128K in
    the memory map giving a total of up to 384K.

    If any of the blocks are 512K, then the block size is set to 512K and the map arranged
    so that the bottom block is a 512K block (if both 512K and 128K blocks are available).

    This is all determined by the value written to port 80 :-

    port80 = 0x07   start       end
        block0      0x00000     0x1FFFF
        block1      0x20000     0x3FFFF
        block2      0x40000     0x5FFFF

    port80 = 0x1F
        block0      0x00000     0x7FFFF
        block1      0x80000     0xEFFFF (0x9FFFF if 128K (2))

    port80 = 0x0F
        block1      0x00000     0x7FFFF
        block0      0x80000     0xEFFFF (0x9FFFF if 128K (1))

    port80 = 0x17
        block1      0x00000     0x7FFFF
        block2      0x80000     0xEFFFF

*/

struct nimbus_meminfo
{
	offs_t  start;      /* start address of bank */
	offs_t  end;        /* End address of bank */
};

static const struct nimbus_meminfo memmap[] =
{
	{ 0x00000, 0x1FFFF },
	{ 0x20000, 0x3FFFF },
	{ 0x40000, 0x5FFFF },
	{ 0x60000, 0x7FFFF },
	{ 0x80000, 0x9FFFF },
	{ 0xA0000, 0xBFFFF },
	{ 0xC0000, 0xDFFFF },
	{ 0xE0000, 0xEFFFF }
};

struct nimbus_block
{
	int     blockbase;
	int     blocksize;
};

typedef nimbus_block nimbus_blocks[3];

static const nimbus_blocks ramblocks[] =
{
	{{ 0, 128 },    { 000, 000 },   { 000, 000 }} ,
	{{ 0, 128 },    { 128, 128 },   { 000, 000 }} ,
	{{ 0, 128 },    { 128, 128 },   { 256, 128 }} ,
	{{ 0, 512 },    { 000, 000 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 128 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 512 },   { 000, 000 }} ,
	{{ 0, 512 },    { 512, 512 },   { 1024, 512 } }
};

static void nimbus_bank_memory(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	address_space &space = machine.device( MAINCPU_TAG)->memory().space( AS_PROGRAM );
	int     ramsize = machine.device<ram_device>(RAM_TAG)->size();
	int     ramblock = 0;
	int     blockno;
	char    bank[10];
	UINT8   *ram    = &machine.device<ram_device>(RAM_TAG)->pointer()[0];
	UINT8   *map_blocks[3];
	UINT8   *map_base;
	int     map_blockno;
	int     block_ofs;

	UINT8   ramsel = (state->m_mcu_reg080 & 0x1F);

	// Invalid ramsel, return.
	if((ramsel & 0x07)!=0x07)
		return;

	switch (ramsize / 1024)
	{
		case 128    : ramblock=0; break;
		case 256    : ramblock=1; break;
		case 384    : ramblock=2; break;
		case 512    : ramblock=3; break;
		case 640    : ramblock=4; break;
		case 1024   : ramblock=5; break;
		case 1536   : ramblock=6; break;
	}

	map_blocks[0]  = ram;
	map_blocks[1]  = (ramblocks[ramblock][1].blocksize==0) ? NULL : &ram[ramblocks[ramblock][1].blockbase*1024];
	map_blocks[2]  = (ramblocks[ramblock][2].blocksize==0) ? NULL : &ram[ramblocks[ramblock][2].blockbase*1024];

	//if(LOG_RAM) logerror("\n\nmcu_reg080=%02X, ramblock=%d, map_blocks[0]=%X, map_blocks[1]=%X, map_blocks[2]=%X\n",state->m_mcu_reg080,ramblock,(int)map_blocks[0],(int)map_blocks[1],(int)map_blocks[2]);

	for(blockno=0;blockno<8;blockno++)
	{
		sprintf(bank,"bank%d",blockno);

		switch (ramsel)
		{
			case 0x07   : (blockno<3) ? map_blockno=blockno : map_blockno=-1; break;
			case 0x1F   : (blockno<4) ? map_blockno=0 : map_blockno=1; break;
			case 0x0F   : (blockno<4) ? map_blockno=1 : map_blockno=0; break;
			case 0x17   : (blockno<4) ? map_blockno=1 : map_blockno=2; break;
			default     : map_blockno=-1;
		}
		block_ofs=(ramsel==0x07) ? 0 : ((blockno % 4)*128);


		if(LOG_RAM) logerror("mapped %s",bank);

		if((block_ofs < ramblocks[ramblock][map_blockno].blocksize) &&
			(map_blocks[map_blockno]!=NULL) && (map_blockno>-1))
		{
			map_base=(ramsel==0x07) ? map_blocks[map_blockno] : &map_blocks[map_blockno][block_ofs*1024];

			state->membank(bank)->set_base(map_base);
			space.install_readwrite_bank(memmap[blockno].start, memmap[blockno].end, bank);
			//if(LOG_RAM) logerror(", base=%X\n",(int)map_base);
		}
		else
		{
			space.nop_readwrite(memmap[blockno].start, memmap[blockno].end);
			if(LOG_RAM) logerror("NOP\n");
		}
	}
}

READ8_MEMBER(rmnimbus_state::nimbus_mcu_r)
{
	return m_mcu_reg080;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_mcu_w)
{
	m_mcu_reg080=data;

	nimbus_bank_memory(machine());
}

static void memory_reset(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	state->m_mcu_reg080=0x07;
	nimbus_bank_memory(machine);
}

READ16_MEMBER(rmnimbus_state::nimbus_io_r)
{
	int pc=space.device().safe_pc();

	logerror("Nimbus IOR at pc=%08X from %04X mask=%04X, data=%04X\n",pc,(offset*2)+0x30,mem_mask,m_IOPorts[offset]);

	switch (offset*2)
	{
		default         : return m_IOPorts[offset];
	}
	return 0;
}

WRITE16_MEMBER(rmnimbus_state::nimbus_io_w)
{
	int pc=space.device().safe_pc();

	logerror("Nimbus IOW at %08X write of %04X to %04X mask=%04X\n",pc,data,(offset*2)+0x30,mem_mask);

	switch (offset*2)
	{
		default         : COMBINE_DATA(&m_IOPorts[offset]); break;
	}
}


/*
    Keyboard emulation

*/

static void keyboard_reset(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	memset(state->m_keyboard.keyrows,0xFF,NIMBUS_KEYROWS);

	// Setup timer to scan state->m_keyboard.
	state->m_keyboard.keyscan_timer->adjust(attotime::zero, 0, attotime::from_hz(50));
}

static void queue_scancode(running_machine &machine, UINT8 scancode)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	state->m_keyboard.queue[state->m_keyboard.head] = scancode;
	state->m_keyboard.head++;
	state->m_keyboard.head %= ARRAY_LENGTH(state->m_keyboard.queue);
}

static int keyboard_queue_read(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	int data;
	if (state->m_keyboard.tail == state->m_keyboard.head)
		return -1;

	data = state->m_keyboard.queue[state->m_keyboard.tail];

	if (LOG_KEYBOARD)
		logerror("keyboard_queue_read(): Keyboard Read 0x%02x\n",data);

	state->m_keyboard.tail++;
	state->m_keyboard.tail %= ARRAY_LENGTH(state->m_keyboard.queue);
	return data;
}

static void scan_keyboard(running_machine &machine)

{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	UINT8   keyrow;
	UINT8   row;
	UINT8   bitno;
	UINT8   mask;
	static const char *const keynames[] = {
		"KEY0", "KEY1", "KEY2", "KEY3", "KEY4",
		"KEY5", "KEY6", "KEY7", "KEY8", "KEY9",
		"KEY10"
	};

	for(row=0;row<NIMBUS_KEYROWS;row++)
	{
		keyrow=machine.root_device().ioport(keynames[row])->read();

		for(mask=0x80, bitno=7;mask>0;mask=mask>>1, bitno-=1)
		{
			if(!(keyrow & mask) && (state->m_keyboard.keyrows[row] & mask))
			{
				if (LOG_KEYBOARD) logerror("keypress %02X\n",(row<<3)+bitno);
				queue_scancode(machine, (row<<3)+bitno);
			}

			if((keyrow & mask) && !(state->m_keyboard.keyrows[row] & mask))
			{
				if (LOG_KEYBOARD) logerror("keyrelease %02X\n",0x80+(row<<3)+bitno);
				queue_scancode(machine, 0x80+(row<<3)+bitno);
			}
		}

		state->m_keyboard.keyrows[row]=keyrow;
	}
}

TIMER_CALLBACK_MEMBER(rmnimbus_state::keyscan_callback)
{
	scan_keyboard(machine());
}

/*

Z80SIO, used for the keyboard interface

*/

/* Z80 SIO/2 */

WRITE_LINE_MEMBER(rmnimbus_state::sio_interrupt)
{
	if(LOG_SIO)
		logerror("SIO Interrupt state=%02X\n",state);

	// Don't re-trigger if already active !
	if(state!=m_sio_int_state)
	{
		m_sio_int_state=state;

		if(state)
			external_int(machine(),0,EXTERNAL_INT_Z80SIO);
	}
}

#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(rmnimbus_state::sio_dtr_w)
{
	if (offset == 1)
	{
	}
}
#endif

WRITE16_MEMBER(rmnimbus_state::sio_serial_transmit)
{
}

READ16_MEMBER(rmnimbus_state::sio_serial_receive)
{
	if(offset==0)
	{
		return keyboard_queue_read(machine());
	}
	else
		return -1;
}

/* Floppy disk */

static void fdc_reset(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	device_t *fdc = machine.device(FDC_TAG);

	state->m_nimbus_drives.reg400=0;
	state->m_nimbus_drives.reg410_in=0;
	state->m_nimbus_drives.reg410_out=0;
	state->m_nimbus_drives.int_ff=0;
	wd17xx_set_pause_time(fdc,FDC_PAUSE);
}

static void set_disk_int(running_machine &machine, int state)
{
	rmnimbus_state *drvstate = machine.driver_data<rmnimbus_state>();
	if(LOG_DISK)
		logerror("nimbus_drives_intrq = %d\n",state);

	if(drvstate->m_iou_reg092 & DISK_INT_ENABLE)
	{
		drvstate->m_nimbus_drives.int_ff=state;

		if(state)
			external_int(machine,0,EXTERNAL_INT_DISK);
	}
}

WRITE_LINE_MEMBER(rmnimbus_state::nimbus_fdc_intrq_w)
{
	set_disk_int(machine(),state);
}

WRITE_LINE_MEMBER(rmnimbus_state::nimbus_fdc_drq_w)
{
	if(LOG_DISK)
		logerror("nimbus_drives_drq_w(%d)\n", state);

	if(state && FDC_DRQ_ENABLED(this))
		drq_callback(machine(),1);
}

static UINT8 fdc_driveno(UINT8 drivesel)
{
	switch (drivesel)
	{
		case 0x01: return 0;
		case 0x02: return 1;
		case 0x04: return 2;
		case 0x08: return 3;
		case 0x10: return 4;
		case 0x20: return 5;
		case 0x40: return 6;
		case 0x80: return 7;
		default: return 0;
	}
}

/*
    0x410 read bits

    0   Ready from floppy
    1   Index pulse from floppy
    2   Motor on from floppy
    3   MSG from HDD
    4   !BSY from HDD
    5   !I/O from HDD
    6   !C/D
    7   !REQ from HDD
*/

READ8_MEMBER(rmnimbus_state::nimbus_disk_r)
{
	int result = 0;
	device_t *fdc = machine().device(FDC_TAG);

	int pc=space.device().safe_pc();
	rmnimbus_state *state = machine().driver_data<rmnimbus_state>();
	device_t *drive = machine().device(nimbus_wd17xx_interface.floppy_drive_tags[FDC_DRIVE()]);

	switch(offset*2)
	{
		case 0x08 :
			result = wd17xx_status_r(fdc, space, 0);
			if (LOG_DISK_FDD) logerror("Disk status=%2.2X\n",result);
			break;
		case 0x0A :
			result = wd17xx_track_r(fdc, space, 0);
			break;
		case 0x0C :
			result = wd17xx_sector_r(fdc, space, 0);
			break;
		case 0x0E :
			result = wd17xx_data_r(fdc, space, 0);
			break;
		case 0x10 :
			m_nimbus_drives.reg410_in &= ~FDC_BITS_410;
			m_nimbus_drives.reg410_in |= (FDC_MOTOR() ? FDC_MOTOR_MASKI : 0x00);
			m_nimbus_drives.reg410_in |= (floppy_drive_get_flag_state(drive, FLOPPY_DRIVE_INDEX) ? 0x00 : FDC_INDEX_MASK);
			m_nimbus_drives.reg410_in |= (floppy_drive_get_flag_state(drive, FLOPPY_DRIVE_READY) ? FDC_READY_MASK : 0x00);

			// Flip inverted bits
			result=m_nimbus_drives.reg410_in ^ INV_BITS_410;
			break;
		case 0x18 :
			result = m_scsibus->scsi_data_r();
			hdc_post_rw(machine());
		default:
			break;
	}

	if(LOG_DISK_FDD && ((offset*2)<=0x10))
		logerror("Nimbus FDCR at pc=%08X from %04X data=%02X\n",pc,(offset*2)+0x400,result);

	if((LOG_DISK_HDD) && ((offset*2)>=0x10))
		logerror("Nimbus HDCR at pc=%08X from %04X data=%02X\n",pc,(offset*2)+0x400,result);

	return result;
}

/*
    0x400 write bits

    0   drive 0 select
    1   drive 1 select
    2   drive 2 select
    3   drive 3 select
    4   side select
    5   fdc motor on
    6   hdc drq enabled
    7   fdc drq enabled

    0x410 write bits

    0   SCSI reset
    1   SCSI SEL
    2   SCSI IRQ Enable
*/

WRITE8_MEMBER(rmnimbus_state::nimbus_disk_w)
{
	device_t *fdc = machine().device(FDC_TAG);
	int                 pc=space.device().safe_pc();
	UINT8               reg400_old = m_nimbus_drives.reg400;

	if(LOG_DISK_FDD && ((offset*2)<=0x10))
		logerror("Nimbus FDCW at %05X write of %02X to %04X\n",pc,data,(offset*2)+0x400);

	if((LOG_DISK_HDD) && (((offset*2)>=0x10) || (offset==0)))
		logerror("Nimbus HDCW at %05X write of %02X to %04X\n",pc,data,(offset*2)+0x400);

	rmnimbus_state *state = machine().driver_data<rmnimbus_state>();
	switch(offset*2)
	{
		case 0x00 :
			m_nimbus_drives.reg400=data;
			wd17xx_set_drive(fdc,FDC_DRIVE());
			wd17xx_set_side(fdc, FDC_SIDE());

			// Nimbus FDC is hard wired for double density
			//wd17xx_set_density(fdc, DEN_MFM_LO);

			// if we enable hdc drq with a pending condition, act on it
			if((data & HDC_DRQ_MASK) && (~reg400_old & HDC_DRQ_MASK))
				hdc_drq(machine());

			break;
		case 0x08 :
			wd17xx_command_w(fdc, space, 0, data);
			break;
		case 0x0A :
			wd17xx_track_w(fdc, space, 0, data);
			break;
		case 0x0C :
			wd17xx_sector_w(fdc, space, 0, data);
			break;
		case 0x0E :
			wd17xx_data_w(fdc, space, 0, data);
			break;
		case 0x10 :
			hdc_ctrl_write(machine(),data);
			break;

		case 0x18 :
			m_scsibus->scsi_data_w(data);
			hdc_post_rw(machine());
			break;
	}
}

static void hdc_reset(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();

	state->m_nimbus_drives.reg410_in=0;
	state->m_nimbus_drives.reg410_in |= (state->m_scsibus->scsi_req_r() ? 0 : HDC_REQ_MASK);
	state->m_nimbus_drives.reg410_in |= (state->m_scsibus->scsi_cd_r()  ? 0 : HDC_CD_MASK);
	state->m_nimbus_drives.reg410_in |= (state->m_scsibus->scsi_io_r()  ? 0 : HDC_IO_MASK);
	state->m_nimbus_drives.reg410_in |= (state->m_scsibus->scsi_bsy_r() ? 0 : HDC_BSY_MASK);
	state->m_nimbus_drives.reg410_in |= (state->m_scsibus->scsi_msg_r() ? 0 : HDC_MSG_MASK);

	state->m_nimbus_drives.drq_ff=0;
}

static void hdc_ctrl_write(running_machine &machine, UINT8 data)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();

	// If we enable the HDC interupt, and an interrupt is pending, go deal with it.
	if(((data & HDC_IRQ_MASK) && (~state->m_nimbus_drives.reg410_out & HDC_IRQ_MASK)) &&
		((~state->m_nimbus_drives.reg410_in & HDC_INT_TRIGGER)==HDC_INT_TRIGGER))
		set_disk_int(machine,1);

	state->m_nimbus_drives.reg410_out=data;

	state->m_scsibus->scsi_rst_w((data & HDC_RESET_MASK) ? 1 : 0);
	state->m_scsibus->scsi_sel_w((data & HDC_SEL_MASK) ? 1 : 0);
}

static void hdc_post_rw(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();

	if((state->m_nimbus_drives.reg410_in & HDC_REQ_MASK)==0)
		state->m_scsibus->scsi_ack_w(1);

	state->m_nimbus_drives.drq_ff=0;
}

static void hdc_drq(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	if(HDC_DRQ_ENABLED() && state->m_nimbus_drives.drq_ff)
	{
		drq_callback(machine,1);
	}
}

WRITE_LINE_MEMBER( rmnimbus_state::nimbus_scsi_bsy_w )
{
	nimbus_scsi_linechange( HDC_BSY_MASK, state );
}

WRITE_LINE_MEMBER( rmnimbus_state::nimbus_scsi_cd_w )
{
	nimbus_scsi_linechange( HDC_CD_MASK, state );
}

WRITE_LINE_MEMBER( rmnimbus_state::nimbus_scsi_io_w )
{
	nimbus_scsi_linechange( HDC_IO_MASK, state );
}

WRITE_LINE_MEMBER( rmnimbus_state::nimbus_scsi_msg_w )
{
	nimbus_scsi_linechange( HDC_MSG_MASK, state );
}

WRITE_LINE_MEMBER( rmnimbus_state::nimbus_scsi_req_w )
{
	nimbus_scsi_linechange( HDC_REQ_MASK, state );
}

void rmnimbus_state::nimbus_scsi_linechange( UINT8 mask, UINT8 state )
{
	UINT8   last = 0;

	last=m_nimbus_drives.reg410_in & mask;

	if(!state)
		m_nimbus_drives.reg410_in|=mask;
	else
		m_nimbus_drives.reg410_in&=~mask;


	if(HDC_IRQ_ENABLED() && ((~m_nimbus_drives.reg410_in & HDC_INT_TRIGGER)==HDC_INT_TRIGGER))
		set_disk_int(machine(),1);
	else
		set_disk_int(machine(),0);

	switch( mask )
	{
	case HDC_REQ_MASK:
		if (state)
		{
			if(((m_nimbus_drives.reg410_in & HDC_CD_MASK)==HDC_CD_MASK) && (last!=0))
			{
				m_nimbus_drives.drq_ff=1;
				hdc_drq(machine());
			}
		}
		else
		{
			m_scsibus->scsi_ack_w(0);
		}
		break;

	case HDC_IO_MASK:
		if (state)
		{
			m_scsibus->scsi_data_w(0);
		}
		break;
	}
}

/* 8031/8051 Peripheral controler 80186 side */

static void pc8031_reset(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	device_t *er59256 = machine.device(ER59256_TAG);

	logerror("peripheral controler reset\n");

	memset(&state->m_ipc_interface,0,sizeof(state->m_ipc_interface));

	if(!er59256_data_loaded(er59256))
		er59256_preload_rom(er59256,def_config,ARRAY_LENGTH(def_config));
}


#if 0
static void ipc_dumpregs()
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	logerror("in_data=%02X, in_status=%02X, out_data=%02X, out_status=%02X\n",
				state->m_ipc_interface.ipc_in, state->m_ipc_interface.status_in,
				state->m_ipc_interface.ipc_out, state->m_ipc_interface.status_out);

}
#endif

READ8_MEMBER(rmnimbus_state::nimbus_pc8031_r)
{
	int pc=space.device().safe_pc();
	UINT8   result;

	switch(offset*2)
	{
		case 0x00   : result=m_ipc_interface.ipc_out;
						m_ipc_interface.status_in   &= ~IPC_IN_READ_PEND;
						m_ipc_interface.status_out  &= ~IPC_OUT_BYTE_AVAIL;
						break;

		case 0x02   : result=m_ipc_interface.status_out;
						break;

		default : result=0; break;
	}

	if(LOG_PC8031_186)
		logerror("Nimbus PCIOR %08X read of %04X returns %02X\n",pc,(offset*2)+0xC0,result);

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_pc8031_w)
{
	int pc=space.device().safe_pc();

	switch(offset*2)
	{
		case 0x00   : m_ipc_interface.ipc_in=data;
						m_ipc_interface.status_in   |= IPC_IN_BYTE_AVAIL;
						m_ipc_interface.status_in   &= ~IPC_IN_ADDR;
						m_ipc_interface.status_out  |= IPC_OUT_READ_PEND;
						break;

		case 0x02   : m_ipc_interface.ipc_in=data;
						m_ipc_interface.status_in   |= IPC_IN_BYTE_AVAIL;
						m_ipc_interface.status_in   |= IPC_IN_ADDR;
						m_ipc_interface.status_out  |= IPC_OUT_READ_PEND;
						break;
	}

	if(LOG_PC8031_186)
		logerror("Nimbus PCIOW %08X write of %02X to %04X\n",pc,data,(offset*2)+0xC0);

}

/* 8031/8051 Peripheral controler 8031/8051 side */

READ8_MEMBER(rmnimbus_state::nimbus_pc8031_iou_r)
{
	int pc=space.device().safe_pc();
	UINT8   result = 0;

	switch (offset & 0x01)
	{
		case 0x00   : result=m_ipc_interface.ipc_in;
						m_ipc_interface.status_out  &= ~IPC_OUT_READ_PEND;
						m_ipc_interface.status_in   &= ~IPC_IN_BYTE_AVAIL;
						break;

		case 0x01   : result=m_ipc_interface.status_in;
						break;
	}

	if(((offset==2) || (offset==3)) && (m_iou_reg092 & PC8031_INT_ENABLE))
		external_int(machine(),0,EXTERNAL_INT_PC8031_8C);

	if(LOG_PC8031)
		logerror("8031: PCIOR %04X read of %04X returns %02X\n",pc,offset,result);

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_pc8031_iou_w)
{
	int pc=space.device().safe_pc();

	if(LOG_PC8031)
		logerror("8031 PCIOW %04X write of %02X to %04X\n",pc,data,offset);

	switch(offset & 0x03)
	{
		case 0x00   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out  |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out  &= ~IPC_OUT_ADDR;
						m_ipc_interface.status_in   |= IPC_IN_READ_PEND;
						break;

		case 0x01   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out   |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out   |= IPC_OUT_ADDR;
						m_ipc_interface.status_in    |= IPC_IN_READ_PEND;
						break;

		case 0x02   : m_ipc_interface.ipc_out=data;
						m_ipc_interface.status_out  |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out  &= ~IPC_OUT_ADDR;
						m_ipc_interface.status_in   |= IPC_IN_READ_PEND;
						if(m_iou_reg092 & PC8031_INT_ENABLE)
						external_int(machine(),0,EXTERNAL_INT_PC8031_8F);
						break;

		case 0x03   : m_ipc_interface.ipc_out=data;
						//m_ipc_interface.status_out   |= IPC_OUT_BYTE_AVAIL;
						m_ipc_interface.status_out   |= IPC_OUT_ADDR;
						m_ipc_interface.status_in    |= IPC_IN_READ_PEND;
						if(m_iou_reg092 & PC8031_INT_ENABLE)
						external_int(machine(),0,EXTERNAL_INT_PC8031_8E);
						break;
	}
}

READ8_MEMBER(rmnimbus_state::nimbus_pc8031_port_r)
{
	device_t *er59256 = machine().device(ER59256_TAG);
	int pc=space.device().safe_pc();
	UINT8   result = 0;

	if(LOG_PC8031_PORT)
		logerror("8031: PCPORTR %04X read of %04X returns %02X\n",pc,offset,result);

	switch(offset)
	{
		case 0x01   : result=er59256_get_iobits(er59256);
	}

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_pc8031_port_w)
{
	device_t *er59256 = machine().device(ER59256_TAG);
	int pc=space.device().safe_pc();

	switch (offset)
	{
		case 0x01   : er59256_set_iobits(er59256,(data&0x0F));
	}

	if(LOG_PC8031_PORT)
		logerror("8031 PCPORTW %04X write of %02X to %04X\n",pc,data,offset);
}



/* IO Unit */
READ8_MEMBER(rmnimbus_state::nimbus_iou_r)
{
	int pc=space.device().safe_pc();
	UINT8   result=0;

	if(offset==0)
	{
		result=m_iou_reg092;
	}

	if(LOG_IOU)
		logerror("Nimbus IOUR %08X read of %04X returns %02X\n",pc,(offset*2)+0x92,result);

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_iou_w)
{
	int pc=space.device().safe_pc();
	device_t *msm5205 = machine().device(MSM5205_TAG);

	if(LOG_IOU)
		logerror("Nimbus IOUW %08X write of %02X to %04X\n",pc,data,(offset*2)+0x92);

	if(offset==0)
	{
		m_iou_reg092=data;
		msm5205_reset_w(msm5205, (data & MSM5205_INT_ENABLE) ? 0 : 1);
	}
}

static void iou_reset(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	state->m_iou_reg092=0x00;
}

/*
    Sound hardware : AY8910

    I believe that the IO ports of the 8910 are used to control the ROMPack ports, however
    this is currently un-implemented (and may never be as I don't have any rompacks!).

    The registers are mapped as so :

    Address     0xE0                0xE2
    Read        Data                ????
    Write       Register Address    Data

*/

static void rmni_sound_reset(running_machine &machine)
{
	rmnimbus_state *state = machine.driver_data<rmnimbus_state>();
	//device_t *ay8910 = machine.device(AY8910_TAG);
	device_t *msm5205 = machine.device(MSM5205_TAG);

	//ay8910_reset_ym(ay8910);
	msm5205_reset_w(msm5205, 1);

	state->m_last_playmode=MSM5205_S48_4B;
	msm5205_playmode_w(msm5205,state->m_last_playmode);

	state->m_ay8910_a=0;
}

READ8_MEMBER(rmnimbus_state::nimbus_sound_ay8910_r)
{
	device_t *ay8910 = machine().device(AY8910_TAG);
	UINT8   result=0;

	if ((offset*2)==0)
		result=ay8910_r(ay8910,space, 0);

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_sound_ay8910_w)
{
	int pc=space.device().safe_pc();
	device_t *ay8910 = machine().device(AY8910_TAG);

	if(LOG_SOUND)
		logerror("Nimbus SoundW %05X write of %02X to %04X\n",pc,data,(offset*2)+0xE0);

	switch (offset*2)
	{
		case 0x00   : ay8910_data_address_w(ay8910, space, 1, data); break;
		case 0x02   : ay8910_data_address_w(ay8910, space, 0, data); break;
	}

}

WRITE8_MEMBER(rmnimbus_state::nimbus_sound_ay8910_porta_w)
{
	device_t *msm5205 = machine().device(MSM5205_TAG);

	msm5205_data_w(msm5205, data);

	// Mouse code needs a copy of this.
	m_ay8910_a=data;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_sound_ay8910_portb_w)
{
	device_t *msm5205 = machine().device(MSM5205_TAG);

	if((data & 0x07)!=m_last_playmode)
	{
		m_last_playmode=(data & 0x07);
		msm5205_playmode_w(msm5205, m_last_playmode);
	}
}

void nimbus_msm5205_vck(device_t *device)
{
	rmnimbus_state *state = device->machine().driver_data<rmnimbus_state>();
	if(state->m_iou_reg092 & MSM5205_INT_ENABLE)
		external_int(device->machine(),0,EXTERNAL_INT_MSM5205);
}

static const int MOUSE_XYA[3][4] = { { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 0, 1, 1, 0 } };
static const int MOUSE_XYB[3][4] = { { 0, 0, 0, 0 }, { 0, 1, 1, 0 }, { 1, 1, 0, 0 } };
//static const int MOUSE_XYA[4] = { 1, 1, 0, 0 };
//static const int MOUSE_XYB[4] = { 0, 1, 1, 0 };

static void mouse_js_reset(running_machine &machine)
{
	rmnimbus_state *drvstate = machine.driver_data<rmnimbus_state>();
	mouse_joy_state *state = &drvstate->m_nimbus_mouse;

	state->m_mouse_px=0;
	state->m_mouse_py=0;
	state->m_mouse_x=128;
	state->m_mouse_y=128;
	state->m_mouse_pc=0;
	state->m_mouse_pcx=0;
	state->m_mouse_pcy=0;
	state->m_intstate_x=0;
	state->m_intstate_y=0;
	state->m_reg0a4=0xC0;

	// Setup timer to poll the mouse
	state->m_mouse_timer->adjust(attotime::zero, 0, attotime::from_hz(1000));
}

TIMER_CALLBACK_MEMBER(rmnimbus_state::mouse_callback)
{
	UINT8   x = 0;
	UINT8   y = 0;
//  int     pc=machine().device(MAINCPU_TAG)->safe_pc();

	UINT8   intstate_x;
	UINT8   intstate_y;
	int     xint;
	int     yint;

	mouse_joy_state *state = &m_nimbus_mouse;


	state->m_reg0a4 = machine().root_device().ioport(MOUSE_BUTTON_TAG)->read() | 0xC0;
	x = machine().root_device().ioport(MOUSEX_TAG)->read();
	y = machine().root_device().ioport(MOUSEY_TAG)->read();

	UINT8   mxa;
	UINT8   mxb;
	UINT8   mya;
	UINT8   myb;

	//logerror("poll_mouse()\n");

	if (x == state->m_mouse_x)
	{
		state->m_mouse_px = MOUSE_PHASE_STATIC;
	}
	else if (x > state->m_mouse_x)
	{
		state->m_mouse_px = MOUSE_PHASE_POSITIVE;
	}
	else if (x < state->m_mouse_x)
	{
		state->m_mouse_px = MOUSE_PHASE_NEGATIVE;
	}

	if (y == state->m_mouse_y)
	{
		state->m_mouse_py = MOUSE_PHASE_STATIC;
	}
	else if (y > state->m_mouse_y)
	{
		state->m_mouse_py = MOUSE_PHASE_POSITIVE;
	}
	else if (y < state->m_mouse_y)
	{
		state->m_mouse_py = MOUSE_PHASE_NEGATIVE;
	}

	switch (state->m_mouse_px)
	{
		case MOUSE_PHASE_STATIC     : break;
		case MOUSE_PHASE_POSITIVE   : state->m_mouse_pcx++; break;
		case MOUSE_PHASE_NEGATIVE   : state->m_mouse_pcx--; break;
	}
	state->m_mouse_pcx &= 0x03;

	switch (state->m_mouse_py)
	{
		case MOUSE_PHASE_STATIC     : break;
		case MOUSE_PHASE_POSITIVE   : state->m_mouse_pcy++; break;
		case MOUSE_PHASE_NEGATIVE   : state->m_mouse_pcy--; break;
	}
	state->m_mouse_pcy &= 0x03;

//  mxb = MOUSE_XYB[state->m_mouse_px][state->m_mouse_pcx]; // XB
//  mxa = MOUSE_XYA[state->m_mouse_px][state->m_mouse_pcx]; // XA
//  mya = MOUSE_XYA[state->m_mouse_py][state->m_mouse_pcy]; // YA
//  myb = MOUSE_XYB[state->m_mouse_py][state->m_mouse_pcy]; // YB

	mxb = MOUSE_XYB[1][state->m_mouse_pcx]; // XB
	mxa = MOUSE_XYA[1][state->m_mouse_pcx]; // XA
	mya = MOUSE_XYA[1][state->m_mouse_pcy]; // YA
	myb = MOUSE_XYB[1][state->m_mouse_pcy]; // YB

	if ((state->m_mouse_py!=MOUSE_PHASE_STATIC) || (state->m_mouse_px!=MOUSE_PHASE_STATIC))
	{
//        logerror("mouse_px=%02X, mouse_py=%02X, mouse_pcx=%02X, mouse_pcy=%02X\n",
//              state->m_mouse_px,state->m_mouse_py,state->m_mouse_pcx,state->m_mouse_pcy);

//        logerror("mxb=%02x, mxa=%02X (mxb ^ mxa)=%02X, (ay8910_a & 0xC0)=%02X, (mxb ^ mxa) ^ ((ay8910_a & 0x80) >> 7)=%02X\n",
//              mxb,mxa, (mxb ^ mxa) , (state->m_ay8910_a & 0xC0), (mxb ^ mxa) ^ ((state->m_ay8910_a & 0x40) >> 6));
	}

	intstate_x = (mxb ^ mxa) ^ ((m_ay8910_a & 0x40) >> 6);
	intstate_y = (myb ^ mya) ^ ((m_ay8910_a & 0x80) >> 7);

	if (MOUSE_INT_ENABLED(this))
	{
		if ((intstate_x==1) && (state->m_intstate_x==0))
//        if (intstate_x!=state->m_intstate_x)
		{

			xint=mxa ? EXTERNAL_INT_MOUSE_XR : EXTERNAL_INT_MOUSE_XL;

			external_int(machine(),0,xint);

//            logerror("Xint:%02X, mxb=%02X\n",xint,mxb);
		}

		if ((intstate_y==1) && (state->m_intstate_y==0))
//        if (intstate_y!=state->m_intstate_y)
		{
			yint=myb ? EXTERNAL_INT_MOUSE_YU : EXTERNAL_INT_MOUSE_YD;

			external_int(machine(),0,yint);
//            logerror("Yint:%02X, myb=%02X\n",yint,myb);
		}
	}
	else
	{
		state->m_reg0a4 &= 0xF0;
		state->m_reg0a4 |= ( mxb & 0x01) << 3; // XB
		state->m_reg0a4 |= (~mxb & 0x01) << 2; // XA
		state->m_reg0a4 |= (~myb & 0x01) << 1; // YA
		state->m_reg0a4 |= ( myb & 0x01) << 0; // YB
	}

	state->m_mouse_x = x;
	state->m_mouse_y = y;

	if ((state->m_mouse_py!=MOUSE_PHASE_STATIC) || (state->m_mouse_px!=MOUSE_PHASE_STATIC))
	{
//        logerror("pc=%05X, reg0a4=%02X, reg092=%02X, ay_a=%02X, x=%02X, y=%02X, px=%02X, py=%02X, intstate_x=%02X, intstate_y=%02X\n",
//                 pc,state->m_reg0a4,state->m_iou_reg092,state->m_ay8910_a,state->m_mouse_x,state->m_mouse_y,state->m_mouse_px,state->m_mouse_py,intstate_x,intstate_y);
	}

	state->m_intstate_x=intstate_x;
	state->m_intstate_y=intstate_y;
}

READ8_MEMBER(rmnimbus_state::nimbus_mouse_js_r)
{
	/*

        bit     description

        0       JOY 0-Up    or mouse XB
        1       JOY 0-Down  or mouse XA
        2       JOY 0-Left  or mouse YA
        3       JOY 0-Right or mouse YB
        4       JOY 0-b0    or mouse rbutton
        5       JOY 0-b1    or mouse lbutton
        6       ?? always reads 1
        7       ?? always reads 1

    */
	UINT8 result;
	//int pc=machine().device(MAINCPU_TAG)->safe_pc();
	mouse_joy_state *state = &m_nimbus_mouse;

	if (ioport("config")->read() & 0x01)
	{
		result=state->m_reg0a4;
		//logerror("mouse_js_r: pc=%05X, result=%02X\n",pc,result);
	}
	else
	{
		result=ioport(JOYSTICK0_TAG)->read_safe(0xff);
	}

	return result;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_mouse_js_w)
{
}

/**********************************************************************
Paralell printer / User port.
The Nimbus paralell printer port card is almost identical to the circuit
in the BBC micro, so I have borrowed the driver code from the BBC :)

Port A output is buffered before being connected to the printer connector.
This means that they can only be operated as output lines.
CA1 is pulled high by a 4K7 resistor. CA1 normally acts as an acknowledge
line when a printer is used. CA2 is buffered so that it has become an open
collector output only. It usially acts as the printer strobe line.
***********************************************************************/

/* USER VIA 6522 port B is connected to the BBC user port */
READ8_MEMBER(rmnimbus_state::nimbus_via_read_portb)
{
	return 0xff;
}

WRITE8_MEMBER(rmnimbus_state::nimbus_via_write_portb)
{
}

WRITE_LINE_MEMBER(rmnimbus_state::nimbus_via_irq_w)
{
	if(state)
		external_int(machine(),VIA_INT,0x00);
}

const via6522_interface nimbus_via =
{
	DEVCB_NULL, //via_user_read_porta,
	DEVCB_DRIVER_MEMBER(rmnimbus_state,nimbus_via_read_portb),
	DEVCB_NULL, //via_user_read_ca1,
	DEVCB_NULL, //via_user_read_cb1,
	DEVCB_NULL, //via_user_read_ca2,
	DEVCB_NULL, //via_user_read_cb2,
	DEVCB_DEVICE_MEMBER(CENTRONICS_TAG, centronics_device, write),
	DEVCB_DRIVER_MEMBER(rmnimbus_state,nimbus_via_write_portb),
	DEVCB_NULL, //via_user_write_ca1
	DEVCB_NULL, //via_user_write_cb1
	DEVCB_DEVICE_LINE_MEMBER(CENTRONICS_TAG, centronics_device, strobe_w),
	DEVCB_NULL, //via_user_write_cb2,
	DEVCB_DRIVER_LINE_MEMBER(rmnimbus_state,nimbus_via_irq_w)
};

WRITE_LINE_MEMBER(rmnimbus_state::nimbus_ack_w)
{
	via6522_device *via_1 = machine().device<via6522_device>(VIA_TAG);
	via_1->write_ca1(!state); /* ack seems to be inverted? */
}
