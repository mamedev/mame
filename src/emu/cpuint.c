/***************************************************************************

    cpuint.c

    Core multi-CPU interrupt engine.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "deprecat.h"
#include "debug/debugcpu.h"



/*************************************
 *
 *  Debug logging
 *
 *************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)




/*************************************
 *
 *  CPU interrupt variables
 *
 *************************************/

/* current states for each CPU */
static INT32 interrupt_vector[MAX_CPU][MAX_INPUT_LINES];

/* deferred states written in callbacks */
static UINT8 input_line_state[MAX_CPU][MAX_INPUT_LINES];
static INT32 input_line_vector[MAX_CPU][MAX_INPUT_LINES];

/* ick, interrupt event queues */
#define MAX_INPUT_EVENTS		32
static INT32 input_event_queue[MAX_CPU][MAX_INPUT_LINES][MAX_INPUT_EVENTS];
static int input_event_index[MAX_CPU][MAX_INPUT_LINES];



/*************************************
 *
 *  IRQ acknowledge callbacks
 *
 *************************************/

static int cpu_0_irq_callback(int line);
static int cpu_1_irq_callback(int line);
static int cpu_2_irq_callback(int line);
static int cpu_3_irq_callback(int line);
static int cpu_4_irq_callback(int line);
static int cpu_5_irq_callback(int line);
static int cpu_6_irq_callback(int line);
static int cpu_7_irq_callback(int line);

int (*const cpu_irq_callbacks[MAX_CPU])(int) =
{
	cpu_0_irq_callback,
	cpu_1_irq_callback,
	cpu_2_irq_callback,
	cpu_3_irq_callback,
	cpu_4_irq_callback,
	cpu_5_irq_callback,
	cpu_6_irq_callback,
	cpu_7_irq_callback
};

static int (*drv_irq_callbacks[MAX_CPU])(running_machine *, int);



#if 0
#pragma mark CORE CPU
#endif

/*************************************
 *
 *  Initialize a CPU's interrupt states
 *
 *************************************/

void cpuint_init(running_machine *machine)
{
	int cpunum;
	int line;

	/* loop over all CPUs and input lines */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
	{
		/* reset any driver hooks into the IRQ acknowledge callbacks */
		drv_irq_callbacks[cpunum] = NULL;

		/* clear out all the CPU states */
		for (line = 0; line < MAX_INPUT_LINES; line++)
		{
			input_line_state[cpunum][line] = CLEAR_LINE;
			interrupt_vector[cpunum][line] =
			input_line_vector[cpunum][line] = cputype_default_irq_vector(machine->config->cpu[cpunum].type);
			input_event_index[cpunum][line] = 0;
		}
	}

	/* set up some stuff to save */
	state_save_push_tag(0);
	state_save_register_item_2d_array("cpu", 0, interrupt_vector);
	state_save_register_item_2d_array("cpu", 0, input_line_state);
	state_save_register_item_2d_array("cpu", 0, input_line_vector);
	state_save_pop_tag();
}



/*************************************
 *
 *  Reset a CPU's interrupt states
 *
 *************************************/

void cpuint_reset(void)
{
	int cpunum, line;

	/* loop over CPUs */
	for (cpunum = 0; cpunum < cpu_gettotalcpu(); cpunum++)
		for (line = 0; line < MAX_INPUT_LINES; line++)
		{
			interrupt_vector[cpunum][line] = cpunum_default_irq_vector(cpunum);
			input_event_index[cpunum][line] = 0;
		}
}



#if 0
#pragma mark -
#pragma mark LINE STATES
#endif


/*************************************
 *
 *  Empty a CPU's event queue for
 *  a specific input line
 *
 *************************************/

static TIMER_CALLBACK( cpunum_empty_event_queue )
{
	int cpunum = param & 0xff;
	int line = param >> 8;
	int i;

	/* swap to the CPU's context */
	cpuintrf_push_context(cpunum);

	/* loop over all events */
	for (i = 0; i < input_event_index[cpunum][line]; i++)
	{
		INT32 input_event = input_event_queue[cpunum][line][i];
		int state = input_event & 0xff;
		int vector = input_event >> 8;

		LOG(("cpunum_empty_event_queue %d,%d,%d\n",cpunum,line,state));

		/* set the input line state and vector */
		input_line_state[cpunum][line] = state;
		input_line_vector[cpunum][line] = vector;

		/* special case: RESET */
		if (line == INPUT_LINE_RESET)
		{
			/* if we're asserting the line, just halt the CPU */
			if (state == ASSERT_LINE)
				cpunum_suspend(cpunum, SUSPEND_REASON_RESET, 1);
			else
			{
				/* if we're clearing the line that was previously asserted, or if we're just */
				/* pulsing the line, reset the CPU */
				if ((state == CLEAR_LINE && cpunum_is_suspended(cpunum, SUSPEND_REASON_RESET)) || state == PULSE_LINE)
					cpunum_reset(cpunum);

				/* if we're clearing the line, make sure the CPU is not halted */
				cpunum_resume(cpunum, SUSPEND_REASON_RESET);
			}
		}

		/* special case: HALT */
		else if (line == INPUT_LINE_HALT)
		{
			/* if asserting, halt the CPU */
			if (state == ASSERT_LINE)
				cpunum_suspend(cpunum, SUSPEND_REASON_HALT, 1);

			/* if clearing, unhalt the CPU */
			else if (state == CLEAR_LINE)
				cpunum_resume(cpunum, SUSPEND_REASON_HALT);
		}

		/* all other cases */
		else
		{
			/* switch off the requested state */
			switch (state)
			{
				case PULSE_LINE:
					/* temporary: PULSE_LINE only makes sense for NMI lines on Z80 */
					assert(machine->config->cpu[cpunum].type != CPU_Z80 || line == INPUT_LINE_NMI);
					activecpu_set_input_line(line, INTERNAL_ASSERT_LINE);
					activecpu_set_input_line(line, INTERNAL_CLEAR_LINE);
					break;

				case HOLD_LINE:
				case ASSERT_LINE:
					activecpu_set_input_line(line, INTERNAL_ASSERT_LINE);
					break;

				case CLEAR_LINE:
					activecpu_set_input_line(line, INTERNAL_CLEAR_LINE);
					break;

				default:
					logerror("cpunum_empty_event_queue cpu #%d, line %d, unknown state %d\n", cpunum, line, state);
			}

			/* generate a trigger to unsuspend any CPUs waiting on the interrupt */
			if (state != CLEAR_LINE)
				cpu_triggerint(machine, cpunum);
		}
	}

	/* swap back */
	cpuintrf_pop_context();

	/* reset counter */
	input_event_index[cpunum][line] = 0;
}



/*************************************
 *
 *  Set the state of a CPU's input
 *  line
 *
 *************************************/

void cpunum_set_input_line(running_machine *machine, int cpunum, int line, int state)
{
	int vector = (line >= 0 && line < MAX_INPUT_LINES) ? interrupt_vector[cpunum][line] : 0xff;
	cpunum_set_input_line_and_vector(machine, cpunum, line, state, vector);
}


void cputag_set_input_line(running_machine *machine, const char *tag, int line, int state)
{
	int cpunum = mame_find_cpu_index(machine, tag);
	assert(cpunum != -1);
	cpunum_set_input_line(machine, cpunum, line, state);
}


void cpunum_set_input_line_vector(int cpunum, int line, int vector)
{
	if (cpunum < cpu_gettotalcpu() && line >= 0 && line < MAX_INPUT_LINES)
	{
		LOG(("cpunum_set_input_line_vector(%d,%d,$%04x)\n",cpunum,line,vector));
		interrupt_vector[cpunum][line] = vector;
		return;
	}
	LOG(("cpunum_set_input_line_vector CPU#%d line %d > max input lines\n", cpunum, line));
}


void cpunum_set_input_line_and_vector(running_machine *machine, int cpunum, int line, int state, int vector)
{
#ifdef MAME_DEBUG
	/* catch errors where people use PULSE_LINE for CPUs that don't support it */
	if (state == PULSE_LINE && line != INPUT_LINE_NMI && line != INPUT_LINE_RESET)
	{
		switch (machine->config->cpu[cpunum].type)
		{
			case CPU_Z80:
			case CPU_Z180:
			case CPU_M68000:
			case CPU_M68008:
			case CPU_M68010:
			case CPU_M68EC020:
			case CPU_M68020:
			case CPU_M68040:
			case CPU_R4600BE:
			case CPU_R4600LE:
			case CPU_R4650BE:
			case CPU_R4650LE:
			case CPU_R4700BE:
			case CPU_R4700LE:
			case CPU_R5000BE:
			case CPU_R5000LE:
			case CPU_QED5271BE:
			case CPU_QED5271LE:
			case CPU_RM7000BE:
			case CPU_RM7000LE:
			case CPU_PPC403GA:
			case CPU_PPC403GCX:
			case CPU_PPC601:
			case CPU_PPC602:
			case CPU_PPC603:
			case CPU_PPC603E:
			case CPU_PPC603R:
			case CPU_PPC604:
			case CPU_I8035:
			case CPU_I8041:
			case CPU_I8048:
			case CPU_I8648:
			case CPU_I8748:
			case CPU_MB8884:
			case CPU_N7751:
			case CPU_TMS34010:
			case CPU_TMS34020:
			case CPU_TMS32010:
			case CPU_TMS32025:
			case CPU_TMS32026:
				fatalerror("CPU %s: PULSE_LINE used with level-detected IRQ %d\n", machine->config->cpu[cpunum].tag, line);
				break;
			
			default:
				break;
		}
	}
#endif

	if (line >= 0 && line < MAX_INPUT_LINES)
	{
		INT32 input_event = (state & 0xff) | (vector << 8);
		int event_index = input_event_index[cpunum][line]++;

		LOG(("cpunum_set_input_line_and_vector(%d,%d,%d,%02x)\n", cpunum, line, state, vector));

		/* if we're full of events, flush the queue and log a message */
		if (event_index >= MAX_INPUT_EVENTS)
		{
			input_event_index[cpunum][line]--;
			cpunum_empty_event_queue(machine, NULL, cpunum | (line << 8));
			event_index = input_event_index[cpunum][line]++;
			logerror("Exceeded pending input line event queue on CPU %d!\n", cpunum);
		}

		/* enqueue the event */
		if (event_index < MAX_INPUT_EVENTS)
		{
			input_event_queue[cpunum][line][event_index] = input_event;

			/* if this is the first one, set the timer */
			if (event_index == 0)
				timer_call_after_resynch(NULL, cpunum | (line << 8), cpunum_empty_event_queue);
		}
	}
}


void cputag_set_input_line_and_vector(running_machine *machine, const char *tag, int line, int state, int vector)
{
	int cpunum = mame_find_cpu_index(machine, tag);
	assert(cpunum != -1);
	cpunum_set_input_line_and_vector(machine, cpunum, line, state, vector);
}


#if 0
#pragma mark -
#pragma mark INTERRUPT HANDLING
#endif

/*************************************
 *
 *  Set IRQ callback for drivers
 *
 *************************************/

void cpunum_set_irq_callback(int cpunum, int (*callback)(running_machine *, int))
{
	drv_irq_callbacks[cpunum] = callback;
}



/*************************************
 *
 *  Internal IRQ callbacks
 *
 *************************************/

INLINE int cpu_irq_callback(running_machine *machine, int cpunum, int line)
{
	int vector = input_line_vector[cpunum][line];

	LOG(("cpu_%d_irq_callback(%d) $%04x\n", cpunum, line, vector));

	/* if the IRQ state is HOLD_LINE, clear it */
	if (input_line_state[cpunum][line] == HOLD_LINE)
	{
		LOG(("->set_irq_line(%d,%d,%d)\n", cpunum, line, CLEAR_LINE));
		activecpu_set_input_line(line, INTERNAL_CLEAR_LINE);
		input_line_state[cpunum][line] = CLEAR_LINE;
	}

	/* if there's a driver callback, run it */
	if (drv_irq_callbacks[cpunum])
		vector = (*drv_irq_callbacks[cpunum])(machine, line);

	/* notify the debugger */
	debug_cpu_interrupt_hook(machine, cpunum, line);

	/* otherwise, just return the current vector */
	return vector;
}

static int cpu_0_irq_callback(int line) { return cpu_irq_callback(Machine, 0, line); }
static int cpu_1_irq_callback(int line) { return cpu_irq_callback(Machine, 1, line); }
static int cpu_2_irq_callback(int line) { return cpu_irq_callback(Machine, 2, line); }
static int cpu_3_irq_callback(int line) { return cpu_irq_callback(Machine, 3, line); }
static int cpu_4_irq_callback(int line) { return cpu_irq_callback(Machine, 4, line); }
static int cpu_5_irq_callback(int line) { return cpu_irq_callback(Machine, 5, line); }
static int cpu_6_irq_callback(int line) { return cpu_irq_callback(Machine, 6, line); }
static int cpu_7_irq_callback(int line) { return cpu_irq_callback(Machine, 7, line); }



