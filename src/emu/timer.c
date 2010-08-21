/***************************************************************************

    timer.c

    Functions needed to generate timing and synchronization between several
    CPUs.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "emu.h"
#include "profiler.h"


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_TIMERS				256
#define MAX_QUANTA				16

#define DEFAULT_MINIMUM_QUANTUM	ATTOSECONDS_IN_MSEC(100)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class emu_timer
{
public:
	running_machine *		machine;		/* pointer to the owning machine */
	emu_timer *				next;			/* next timer in order in the list */
	emu_timer *				prev;			/* previous timer in order in the list */
	timer_fired_func		callback;		/* callback function */
	INT32					param;			/* integer parameter */
	void *					ptr;			/* pointer parameter */
	const char *			file;			/* file that created the timer */
	int 					line;			/* line number that created the timer */
	const char *			func;			/* string name of the callback function */
	UINT8					enabled;		/* is the timer enabled? */
	UINT8					temporary;		/* is the timer temporary? */
	attotime				period;			/* the repeat frequency of the timer */
	attotime				start;			/* time when the timer was started */
	attotime				expire;			/* time when the timer will expire */
};


/* a single minimum quantum */
typedef struct _quantum_slot quantum_slot;
struct _quantum_slot
{
	attoseconds_t			actual;			/* actual duration of the quantum */
	attoseconds_t			requested;		/* duration of the requested quantum */
	attotime				expire;			/* absolute expiration time of this quantum */
};


/* global private data */
/* In mame.h: typedef struct _timer_private timer_private; */
struct _timer_private
{
	/* list of active timers */
	emu_timer				timers[MAX_TIMERS]; /* actual timers */
	emu_timer *				activelist;			/* head of the active list */
	emu_timer *				freelist;			/* head of the free list */
	emu_timer *				freelist_tail;		/* tail of the free list */

	/* execution state */
	timer_execution_state	exec;				/* current global execution state */

	/* other internal states */
	emu_timer *				callback_timer;		/* pointer to the current callback timer */
	UINT8					callback_timer_modified; /* TRUE if the current callback timer was modified */
	attotime				callback_timer_expire_time; /* the original expiration time */

	/* scheduling quanta */
	quantum_slot			quantum_list[MAX_QUANTA]; /* list of scheduling quanta */
	quantum_slot *			quantum_current;	/* current minimum quantum */
	attoseconds_t			quantum_minimum;	/* duration of minimum quantum */
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static STATE_POSTLOAD( timer_postload );
static void timer_logtimers(running_machine *machine);
static void timer_remove(emu_timer *which);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_current_time - return the current time
-------------------------------------------------*/

INLINE attotime get_current_time(running_machine *machine)
{
	timer_private *global = machine->timer_data;

	/* if we're currently in a callback, use the timer's expiration time as a base */
	if (global->callback_timer != NULL)
		return global->callback_timer_expire_time;

	/* if we're executing as a particular CPU, use its local time as a base */
	/* otherwise, return the global base time */
	device_execute_interface *execdevice = machine->scheduler().currently_executing();
	return (execdevice != NULL) ? execdevice->local_time() : global->exec.basetime;
}


/*-------------------------------------------------
    timer_new - allocate a new timer
-------------------------------------------------*/

INLINE emu_timer *timer_new(running_machine *machine)
{
	timer_private *global = machine->timer_data;
	emu_timer *timer;

	/* if nothing remains available, fatal error -- we should never hit this */
	if (global->freelist == NULL)
	{
		timer_logtimers(machine);
		fatalerror("Out of timers!");
	}

	/* pull an entry from the free list */
	timer = global->freelist;
	global->freelist = timer->next;
	if (global->freelist == NULL)
		global->freelist_tail = NULL;

	/* set up the machine */
	timer->machine = machine;
	return timer;
}


/*-------------------------------------------------
    timer_list_insert - insert a new timer into
    the list at the appropriate location
-------------------------------------------------*/

INLINE void timer_list_insert(emu_timer *timer)
{
	attotime expire = timer->enabled ? timer->expire : attotime_never;
	timer_private *global = timer->machine->timer_data;
	emu_timer *t, *lt = NULL;

	/* sanity checks for the debug build */
	#ifdef MAME_DEBUG
	{
		int tnum = 0;

		/* loop over the timer list */
		for (t = global->activelist; t; t = t->next, tnum++)
		{
			if (t == timer)
				fatalerror("This timer is already inserted in the list!");
			if (tnum == MAX_TIMERS-1)
				fatalerror("Timer list is full!");
		}
	}
	#endif

	/* loop over the timer list */
	for (t = global->activelist; t != NULL; lt = t, t = t->next)
	{
		/* if the current list entry expires after us, we should be inserted before it */
		if (attotime_compare(t->expire, expire) > 0)
		{
			/* link the new guy in before the current list entry */
			timer->prev = t->prev;
			timer->next = t;

			if (t->prev != NULL)
				t->prev->next = timer;
			else
			{
				global->activelist = timer;
				global->exec.nextfire = timer->expire;
			}
			t->prev = timer;
			return;
		}
	}

	/* need to insert after the last one */
	if (lt != NULL)
		lt->next = timer;
	else
	{
		global->activelist = timer;
		global->exec.nextfire = timer->expire;
	}
	timer->prev = lt;
	timer->next = NULL;
}


/*-------------------------------------------------
    timer_list_remove - remove a timer from the
    linked list
-------------------------------------------------*/

INLINE void timer_list_remove(emu_timer *timer)
{
	timer_private *global = timer->machine->timer_data;

	/* sanity checks for the debug build */
	#ifdef MAME_DEBUG
	{
		emu_timer *t;

		/* loop over the timer list */
		for (t = global->activelist; t && t != timer; t = t->next) ;
		if (t == NULL)
			fatalerror("timer (%s from %s:%d) not found in list", timer->func, timer->file, timer->line);
	}
	#endif

	/* remove it from the list */
	if (timer->prev != NULL)
		timer->prev->next = timer->next;
	else
	{
		global->activelist = timer->next;
		if (global->activelist != NULL)
			global->exec.nextfire = global->activelist->expire;
	}
	if (timer->next != NULL)
		timer->next->prev = timer->prev;
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    timer_init - initialize the timer system
-------------------------------------------------*/

void timer_init(running_machine *machine)
{
	timer_private *global;
	int i;

	/* allocate global data */
	global = machine->timer_data = auto_alloc_clear(machine, timer_private);

	/* we need to wait until the first call to timer_cyclestorun before using real CPU times */
	global->exec.basetime = attotime_zero;
	global->exec.nextfire = attotime_never;
	global->exec.curquantum = DEFAULT_MINIMUM_QUANTUM;
	global->callback_timer = NULL;
	global->callback_timer_modified = FALSE;

	/* register with the save state system */
	state_save_register_item(machine, "timer", NULL, 0, global->exec.basetime.seconds);
	state_save_register_item(machine, "timer", NULL, 0, global->exec.basetime.attoseconds);
	state_save_register_postload(machine, timer_postload, NULL);

	/* initialize the lists */
	global->activelist = NULL;
	global->freelist = &global->timers[0];
	for (i = 0; i < MAX_TIMERS-1; i++)
		global->timers[i].next = &global->timers[i+1];
	global->timers[MAX_TIMERS-1].next = NULL;
	global->freelist_tail = &global->timers[MAX_TIMERS-1];

	/* reset the quanta */
	global->quantum_list[0].requested = DEFAULT_MINIMUM_QUANTUM;
	global->quantum_list[0].actual = DEFAULT_MINIMUM_QUANTUM;
	global->quantum_list[0].expire = attotime_never;
	global->quantum_current = &global->quantum_list[0];
	global->quantum_minimum = ATTOSECONDS_IN_NSEC(1) / 1000;
}


/*-------------------------------------------------
    timer_destructor - destruct a timer from a
    pool callback
-------------------------------------------------*/

void timer_destructor(void *ptr, size_t size)
{
	timer_remove((emu_timer *)ptr);
}



/***************************************************************************
    SCHEDULING HELPERS
***************************************************************************/

/*-------------------------------------------------
    timer_get_execution_state - return a pointer
    to the execution state
-------------------------------------------------*/

timer_execution_state *timer_get_execution_state(running_machine *machine)
{
	timer_private *global = machine->timer_data;
	return &global->exec;
}


/*-------------------------------------------------
    timer_execute_timers - execute timers and
    update scheduling quanta
-------------------------------------------------*/

void timer_execute_timers(running_machine *machine)
{
	timer_private *global = machine->timer_data;
	emu_timer *timer;

	/* if the current quantum has expired, find a new one */
	if (attotime_compare(global->exec.basetime, global->quantum_current->expire) >= 0)
	{
		int curr;

		global->quantum_current->requested = 0;
		global->quantum_current = &global->quantum_list[0];
		for (curr = 1; curr < ARRAY_LENGTH(global->quantum_list); curr++)
			if (global->quantum_list[curr].requested != 0 && global->quantum_list[curr].requested < global->quantum_current->requested)
				global->quantum_current = &global->quantum_list[curr];
		global->exec.curquantum = global->quantum_current->actual;
	}

	LOG(("timer_set_global_time: new=%s head->expire=%s\n", attotime_string(global->exec.basetime, 9), attotime_string(global->activelist->expire, 9)));

	/* now process any timers that are overdue */
	while (attotime_compare(global->activelist->expire, global->exec.basetime) <= 0)
	{
		int was_enabled = global->activelist->enabled;

		/* if this is a one-shot timer, disable it now */
		timer = global->activelist;
		if (attotime_compare(timer->period, attotime_zero) == 0 || attotime_compare(timer->period, attotime_never) == 0)
			timer->enabled = FALSE;

		/* set the global state of which callback we're in */
		global->callback_timer_modified = FALSE;
		global->callback_timer = timer;
		global->callback_timer_expire_time = timer->expire;

		/* call the callback */
		if (was_enabled && timer->callback != NULL)
		{
			LOG(("Timer %s:%d[%s] fired (expire=%s)\n", timer->file, timer->line, timer->func, attotime_string(timer->expire, 9)));
			g_profiler.start(PROFILER_TIMER_CALLBACK);
			(*timer->callback)(machine, timer->ptr, timer->param);
			g_profiler.stop();
		}

		/* clear the callback timer global */
		global->callback_timer = NULL;

		/* reset or remove the timer, but only if it wasn't modified during the callback */
		if (!global->callback_timer_modified)
		{
			/* if the timer is temporary, remove it now */
			if (timer->temporary)
				timer_remove(timer);

			/* otherwise, reschedule it */
			else
			{
				timer->start = timer->expire;
				timer->expire = attotime_add(timer->expire, timer->period);

				timer_list_remove(timer);
				timer_list_insert(timer);
			}
		}
	}
}


/*-------------------------------------------------
    timer_add_scheduling_quantum - add a
    scheduling quantum; the smallest active one
    is the one that is in use
-------------------------------------------------*/

void timer_add_scheduling_quantum(running_machine *machine, attoseconds_t quantum, attotime duration)
{
	timer_private *global = machine->timer_data;
	attotime curtime = timer_get_time(machine);
	attotime expire = attotime_add(curtime, duration);
	int curr, blank = -1;

	/* a 0 request (minimum) needs to be non-zero to occupy a slot */
	if (quantum == 0)
		quantum = 1;

	/* find an equal-duration slot or an empty slot */
	for (curr = 1; curr < ARRAY_LENGTH(global->quantum_list); curr++)
	{
		quantum_slot *slot = &global->quantum_list[curr];

		/* look for a matching quantum and extend it */
		if (slot->requested == quantum)
		{
			slot->expire = attotime_max(slot->expire, expire);
			return;
		}

		/* remember any empty slots in case of no match */
		if (slot->requested == 0)
		{
			if (blank == -1)
				blank = curr;
		}

		/* otherwise, expire any expired slots */
		else if (attotime_compare(curtime, slot->expire) >= 0)
			slot->requested = 0;
	}

	/* fatal error if no slots left */
	assert_always(blank != -1, "Out of scheduling quantum slots!");

	/* fill in the item */
	global->quantum_list[blank].requested = quantum;
	global->quantum_list[blank].actual = MAX(global->quantum_list[blank].requested, global->quantum_minimum);
	global->quantum_list[blank].expire = expire;

	/* update the minimum */
	if (quantum < global->quantum_current->requested)
	{
		global->quantum_current = &global->quantum_list[blank];
		global->exec.curquantum = global->quantum_current->actual;
	}
}


/*-------------------------------------------------
    timer_set_minimum_quantum - control the
    minimum useful quantum (used by cpuexec only)
-------------------------------------------------*/

void timer_set_minimum_quantum(running_machine *machine, attoseconds_t quantum)
{
	timer_private *global = machine->timer_data;
	int curr;

	/* do nothing if nothing changed */
	if (global->quantum_minimum == quantum)
		return;
	global->quantum_minimum = quantum;

	/* adjust all the actuals; this doesn't affect the current */
	for (curr = 0; curr < ARRAY_LENGTH(global->quantum_list); curr++)
		if (global->quantum_list[curr].requested != 0)
			global->quantum_list[curr].actual = MAX(global->quantum_list[curr].requested, global->quantum_minimum);

	/* ensure that the live current quantum is up to date */
	global->exec.curquantum = global->quantum_current->actual;
}



/***************************************************************************
    SAVE/RESTORE HELPERS
***************************************************************************/

/*-------------------------------------------------
    timer_register_save - register ourself with
    the save state system
-------------------------------------------------*/

static void timer_register_save(emu_timer *timer)
{
	timer_private *global = timer->machine->timer_data;
	int count = 0;
	emu_timer *t;

	/* find other timers that match our func name */
	for (t = global->activelist; t; t = t->next)
		if (!strcmp(t->func, timer->func))
			count++;

	/* use different instances to differentiate the bits */
	state_save_register_item(timer->machine, "timer", timer->func, count, timer->param);
	state_save_register_item(timer->machine, "timer", timer->func, count, timer->enabled);
	state_save_register_item(timer->machine, "timer", timer->func, count, timer->period.seconds);
	state_save_register_item(timer->machine, "timer", timer->func, count, timer->period.attoseconds);
	state_save_register_item(timer->machine, "timer", timer->func, count, timer->start.seconds);
	state_save_register_item(timer->machine, "timer", timer->func, count, timer->start.attoseconds);
	state_save_register_item(timer->machine, "timer", timer->func, count, timer->expire.seconds);
	state_save_register_item(timer->machine, "timer", timer->func, count, timer->expire.attoseconds);
}


/*-------------------------------------------------
    timer_postload - after loading a save state
-------------------------------------------------*/

static STATE_POSTLOAD( timer_postload )
{
	timer_private *global = machine->timer_data;
	emu_timer *privlist = NULL;
	emu_timer *t;

	/* remove all timers and make a private list */
	while (global->activelist != NULL)
	{
		t = global->activelist;

		/* temporary timers go away entirely */
		if (t->temporary)
			timer_remove(t);

		/* permanent ones get added to our private list */
		else
		{
			timer_list_remove(t);
			t->next = privlist;
			privlist = t;
		}
	}

	/* now add them all back in; this effectively re-sorts them by time */
	while (privlist != NULL)
	{
		t = privlist;
		privlist = t->next;
		timer_list_insert(t);
	}
}


/*-------------------------------------------------
    timer_count_anonymous - count the number of
    anonymous (non-saveable) timers
-------------------------------------------------*/

int timer_count_anonymous(running_machine *machine)
{
	timer_private *global = machine->timer_data;
	emu_timer *t;
	int count = 0;

	logerror("timer_count_anonymous:\n");
	for (t = global->activelist; t; t = t->next)
		if (t->temporary && t != global->callback_timer)
		{
			count++;
			logerror("  Temp. timer %p, file %s:%d[%s]\n", (void *) t, t->file, t->line, t->func);
		}
	logerror("%d temporary timers found\n", count);

	return count;
}



/***************************************************************************
    CORE TIMER ALLOCATION
***************************************************************************/

/*-------------------------------------------------
    timer_alloc - allocate a permament timer that
    isn't primed yet
-------------------------------------------------*/

INLINE emu_timer *_timer_alloc_common(running_machine *machine, timer_fired_func callback, void *ptr, const char *file, int line, const char *func, int temp)
{
	attotime time = get_current_time(machine);
	emu_timer *timer = timer_new(machine);

	/* fill in the record */
	timer->callback = callback;
	timer->ptr = ptr;
	timer->param = 0;
	timer->enabled = FALSE;
	timer->temporary = temp;
	timer->period = attotime_zero;
	timer->file = file;
	timer->line = line;
	timer->func = func;

	/* compute the time of the next firing and insert into the list */
	timer->start = time;
	timer->expire = attotime_never;
	timer_list_insert(timer);

	/* if we're not temporary, register ourselves with the save state system */
	if (!temp)
	{
		if (!state_save_registration_allowed(machine))
			fatalerror("timer_alloc() called after save state registration closed! (file %s, line %d)\n", file, line);
		timer_register_save(timer);
	}

	/* return a handle */
	return timer;
}

emu_timer *_timer_alloc_internal(running_machine *machine, timer_fired_func callback, void *ptr, const char *file, int line, const char *func)
{
	return _timer_alloc_common(machine, callback, ptr, file, line, func, FALSE);
}


/*-------------------------------------------------
    timer_remove - remove a timer from the
    system
-------------------------------------------------*/

static void timer_remove(emu_timer *which)
{
	timer_private *global = which->machine->timer_data;

	/* if this is a callback timer, note that */
	if (which == global->callback_timer)
		global->callback_timer_modified = TRUE;

	/* remove it from the list */
	timer_list_remove(which);

	/* free it up by adding it back to the free list */
	if (global->freelist_tail)
		global->freelist_tail->next = which;
	else
		global->freelist = which;
	which->next = NULL;
	global->freelist_tail = which;
}



/***************************************************************************
    CORE TIMER ADJUSTMENT
***************************************************************************/

/*-------------------------------------------------
    timer_adjust_oneshot - adjust the time when this timer
    will fire and disable any periodic firings
-------------------------------------------------*/

void timer_adjust_oneshot(emu_timer *which, attotime duration, INT32 param)
{
	timer_adjust_periodic(which, duration, param, attotime_never);
}


/*-------------------------------------------------
    timer_adjust_periodic - adjust the time when
    this timer will fire and specify a period for
    subsequent firings
-------------------------------------------------*/

void timer_adjust_periodic(emu_timer *which, attotime start_delay, INT32 param, attotime period)
{
	timer_private *global = which->machine->timer_data;
	attotime time = get_current_time(which->machine);

	/* if this is the callback timer, mark it modified */
	if (which == global->callback_timer)
		global->callback_timer_modified = TRUE;

	/* compute the time of the next firing and insert into the list */
	which->param = param;
	which->enabled = TRUE;

	/* clamp negative times to 0 */
	if (start_delay.seconds < 0)
		start_delay = attotime_zero;

	/* set the start and expire times */
	which->start = time;
	which->expire = attotime_add(time, start_delay);
	which->period = period;

	/* remove and re-insert the timer in its new order */
	timer_list_remove(which);
	timer_list_insert(which);

	/* if this was inserted as the head, abort the current timeslice and resync */
	LOG(("timer_adjust_oneshot %s.%s:%d to expire @ %s\n", which->file, which->func, which->line, attotime_string(which->expire, 9)));
	if (which == global->activelist)
		which->machine->scheduler().abort_timeslice();
}



/***************************************************************************
    SIMPLIFIED ANONYMOUS TIMER MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    timer_pulse - allocate a pulse timer, which
    repeatedly calls the callback using the given
    period
-------------------------------------------------*/

void _timer_pulse_internal(running_machine *machine, attotime period, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func)
{
	emu_timer *timer = _timer_alloc_common(machine, callback, ptr, file, line, func, FALSE);
	timer_adjust_periodic(timer, period, param, period);
}


/*-------------------------------------------------
    timer_set - allocate a one-shot timer, which
    calls the callback after the given duration
-------------------------------------------------*/

void _timer_set_internal(running_machine *machine, attotime duration, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func)
{
	emu_timer *timer = _timer_alloc_common(machine, callback, ptr, file, line, func, TRUE);
	timer_adjust_oneshot(timer, duration, param);
}



/***************************************************************************
    MISCELLANEOUS CONTROLS
***************************************************************************/

/*-------------------------------------------------
    timer_reset - reset the timing on a timer
-------------------------------------------------*/

void timer_reset(emu_timer *which, attotime duration)
{
	timer_adjust_periodic(which, duration, which->param, which->period);
}


/*-------------------------------------------------
    timer_enable - enable/disable a timer
-------------------------------------------------*/

int timer_enable(emu_timer *which, int enable)
{
	int old;

	/* set the enable flag */
	old = which->enabled;
	which->enabled = enable;

	/* remove the timer and insert back into the list */
	timer_list_remove(which);
	timer_list_insert(which);

	return old;
}


/*-------------------------------------------------
    timer_enabled - determine if a timer is
    enabled
-------------------------------------------------*/

int timer_enabled(emu_timer *which)
{
	return which->enabled;
}


/*-------------------------------------------------
    timer_get_param - returns the callback
    parameter of a timer
-------------------------------------------------*/

int timer_get_param(emu_timer *which)
{
	return which->param;
}


/*-------------------------------------------------
    timer_set_param - changes the callback
    parameter of a timer
-------------------------------------------------*/

void timer_set_param(emu_timer *which, int param)
{
	which->param = param;
}


/*-------------------------------------------------
    timer_get_ptr - returns the callback pointer
    of a timer
-------------------------------------------------*/

void *timer_get_ptr(emu_timer *which)
{
	return which->ptr;
}


/*-------------------------------------------------
    timer_set_ptr - changes the callback pointer
    of a timer
-------------------------------------------------*/

void timer_set_ptr(emu_timer *which, void *ptr)
{
	which->ptr = ptr;
}



/***************************************************************************
    TIMING FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    timer_timeelapsed - return the time since the
    last trigger
-------------------------------------------------*/

attotime timer_timeelapsed(emu_timer *which)
{
	return attotime_sub(get_current_time(which->machine), which->start);
}


/*-------------------------------------------------
    timer_timeleft - return the time until the
    next trigger
-------------------------------------------------*/

attotime timer_timeleft(emu_timer *which)
{
	return attotime_sub(which->expire, get_current_time(which->machine));
}


/*-------------------------------------------------
    timer_get_time - return the current time
-------------------------------------------------*/

attotime timer_get_time(running_machine *machine)
{
	return get_current_time(machine);
}


/*-------------------------------------------------
    timer_starttime - return the time when this
    timer started counting
-------------------------------------------------*/

attotime timer_starttime(emu_timer *which)
{
	return which->start;
}


/*-------------------------------------------------
    timer_firetime - return the time when this
    timer will fire next
-------------------------------------------------*/

attotime timer_firetime(emu_timer *which)
{
	return which->expire;
}



/***************************************************************************
    DEBUGGING
***************************************************************************/

/*-------------------------------------------------
    timer_logtimers - log all the timers
-------------------------------------------------*/

static void timer_logtimers(running_machine *machine)
{
	timer_private *global = machine->timer_data;
	emu_timer *t;

	logerror("===============\n");
	logerror("TIMER LOG START\n");
	logerror("===============\n");

	logerror("Enqueued timers:\n");
	for (t = global->activelist; t; t = t->next)
		logerror("  Start=%15.6f Exp=%15.6f Per=%15.6f Ena=%d Tmp=%d (%s:%d[%s])\n",
			attotime_to_double(t->start), attotime_to_double(t->expire), attotime_to_double(t->period), t->enabled, t->temporary, t->file, t->line, t->func);

	logerror("Free timers:\n");
	for (t = global->freelist; t; t = t->next)
		logerror("  Start=%15.6f Exp=%15.6f Per=%15.6f Ena=%d Tmp=%d (%s:%d[%s])\n",
			attotime_to_double(t->start), attotime_to_double(t->expire), attotime_to_double(t->period), t->enabled, t->temporary, t->file, t->line, t->func);

	logerror("==============\n");
	logerror("TIMER LOG STOP\n");
	logerror("==============\n");
}


void timer_print_first_timer(running_machine *machine)
{
	timer_private *global = machine->timer_data;
	emu_timer *t = global->activelist;
	printf("  Start=%15.6f Exp=%15.6f Per=%15.6f Ena=%d Tmp=%d (%s)\n",
		attotime_to_double(t->start), attotime_to_double(t->expire), attotime_to_double(t->period), t->enabled, t->temporary, t->func);
}


//**************************************************************************
//  TIMER DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  timer_device_config - constructor
//-------------------------------------------------

timer_device_config::timer_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	: device_config(mconfig, static_alloc_device_config, "Timer", tag, owner, clock),
	  m_type(TIMER_TYPE_GENERIC),
	  m_callback(NULL),
	  m_ptr(NULL),
	  m_start_delay(0),
	  m_period(0),
	  m_param(0),
	  m_screen(NULL),
	  m_first_vpos(0),
	  m_increment(0)
{
}


//-------------------------------------------------
//  static_alloc_device_config - allocate a new
//  configuration object
//-------------------------------------------------

device_config *timer_device_config::static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
{
	return global_alloc(timer_device_config(mconfig, tag, owner, clock));
}


//-------------------------------------------------
//  alloc_device - allocate a new device object
//-------------------------------------------------

device_t *timer_device_config::alloc_device(running_machine &machine) const
{
	return auto_alloc(&machine, timer_device(machine, *this));
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void timer_device_config::device_config_complete()
{
	// move inline data into its final home
	m_type = static_cast<timer_type>(m_inline_data[INLINE_TYPE]);
	m_callback = reinterpret_cast<timer_device_fired_func>(m_inline_data[INLINE_CALLBACK]);
	m_ptr = reinterpret_cast<void *>(m_inline_data[INLINE_PTR]);
	m_start_delay = static_cast<UINT64>(m_inline_data[INLINE_DELAY]);
	m_period = static_cast<UINT64>(m_inline_data[INLINE_PERIOD]);
	m_param = static_cast<UINT32>(m_inline_data[INLINE_PARAM]);
	m_screen = reinterpret_cast<const char *>(m_inline_data[INLINE_SCREEN]);
	m_first_vpos = static_cast<INT16>(m_inline_data[INLINE_FIRST_VPOS]);
	m_increment = static_cast<INT16>(m_inline_data[INLINE_INCREMENT]);
}


//-------------------------------------------------
//  device_validity_check - validate the device
//  configuration
//-------------------------------------------------

bool timer_device_config::device_validity_check(const game_driver &driver) const
{
	bool error = false;

	// type based configuration
	switch (m_type)
	{
		case TIMER_TYPE_GENERIC:
			if (m_screen != NULL || m_first_vpos != 0 || m_start_delay != 0)
				mame_printf_warning("%s: %s generic timer '%s' specified parameters for a scanline timer\n", driver.source_file, driver.name, tag());
			if (m_period != 0 || m_start_delay != 0)
				mame_printf_warning("%s: %s generic timer '%s' specified parameters for a periodic timer\n", driver.source_file, driver.name, tag());
			break;

		case TIMER_TYPE_PERIODIC:
			if (m_screen != NULL || m_first_vpos != 0)
				mame_printf_warning("%s: %s periodic timer '%s' specified parameters for a scanline timer\n", driver.source_file, driver.name, tag());
			if (m_period <= 0)
			{
				mame_printf_error("%s: %s periodic timer '%s' specified invalid period\n", driver.source_file, driver.name, tag());
				error = true;
			}
			break;

		case TIMER_TYPE_SCANLINE:
			if (m_period != 0 || m_start_delay != 0)
				mame_printf_warning("%s: %s scanline timer '%s' specified parameters for a periodic timer\n", driver.source_file, driver.name, tag());
			if (m_param != 0)
				mame_printf_warning("%s: %s scanline timer '%s' specified parameter which is ignored\n", driver.source_file, driver.name, tag());
			if (m_first_vpos < 0)
			{
				mame_printf_error("%s: %s scanline timer '%s' specified invalid initial position\n", driver.source_file, driver.name, tag());
				error = true;
			}
			if (m_increment < 0)
			{
				mame_printf_error("%s: %s scanline timer '%s' specified invalid increment\n", driver.source_file, driver.name, tag());
				error = true;
			}
			break;

		default:
			mame_printf_error("%s: %s timer '%s' has an invalid type\n", driver.source_file, driver.name, tag());
			error = true;
			break;
	}

	return error;
}



//**************************************************************************
//  LIVE TIMER DEVICE
//**************************************************************************

//-------------------------------------------------
//  timer_device - constructor
//-------------------------------------------------

timer_device::timer_device(running_machine &_machine, const timer_device_config &config)
	: device_t(_machine, config),
	  m_config(config),
	  m_timer(NULL),
	  m_ptr(m_config.m_ptr),
	  m_screen(NULL),
	  m_first_time(true)
{
}


//-------------------------------------------------
//  device_start - perform device-specific
//  startup
//-------------------------------------------------

void timer_device::device_start()
{
	// fetch the screen
	if (m_config.m_screen != NULL)
		m_screen = downcast<screen_device *>(machine->device(m_config.m_screen));

	// allocate the timer
	m_timer = timer_alloc(machine, (m_config.m_type == timer_device_config::TIMER_TYPE_SCANLINE) ? static_scanline_timer_callback : static_periodic_timer_callback, (void *)this);

	// register for save states
	state_save_register_device_item(this, 0, m_first_time);
}


//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void timer_device::device_reset()
{
	// type based configuration
	switch (m_config.m_type)
	{
		case timer_device_config::TIMER_TYPE_GENERIC:
		case timer_device_config::TIMER_TYPE_PERIODIC:
		{
			// convert the period into attotime
			attotime period = attotime_never;
			if (m_config.m_period > 0)
			{
				period = UINT64_ATTOTIME_TO_ATTOTIME(m_config.m_period);

				// convert the start_delay into attotime
				attotime start_delay = attotime_zero;
				if (m_config.m_start_delay > 0)
					start_delay = UINT64_ATTOTIME_TO_ATTOTIME(m_config.m_start_delay);

				// allocate and start the backing timer
				timer_adjust_periodic(m_timer, start_delay, m_config.m_param, period);
			}
			break;
		}

		case timer_device_config::TIMER_TYPE_SCANLINE:
			if (m_screen == NULL)
				fatalerror("timer '%s': unable to find screen '%s'\n", tag(), m_config.m_screen);

			// set the timer to to fire immediately
			m_first_time = true;
			timer_adjust_oneshot(m_timer, attotime_zero, m_config.m_param);
			break;
	}
}


/*-------------------------------------------------
    periodic_timer_callback - calls the timer
    device specific callback
-------------------------------------------------*/

void timer_device::periodic_timer_callback(int param)
{
	if (m_config.m_callback != NULL)
		(*m_config.m_callback)(*this, m_ptr, param);
}


/*-------------------------------------------------
    scanline_timer_device_timer_callback -
    manages the scanline based timer's state
-------------------------------------------------*/

void timer_device::scanline_timer_callback(int scanline)
{
	// by default, we fire at the first position
	int next_vpos = m_config.m_first_vpos;

	// the first time through we just go with the default position
	if (!m_first_time)
	{
		// call the real callback
		int vpos = m_screen->vpos();
		(*m_config.m_callback)(*this, m_ptr, vpos);

		// advance by the increment only if we will still be within the screen bounds
        if (m_config.m_increment != 0 && (vpos + m_config.m_increment) < m_screen->height())
			next_vpos = vpos + m_config.m_increment;
	}
	m_first_time = false;

	// adjust the timer
	timer_adjust_oneshot(m_timer, m_screen->time_until_pos(next_vpos), 0);
}

const device_type TIMER = timer_device_config::static_alloc_device_config;
