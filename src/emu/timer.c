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

/* in timer.h: typedef struct _emu_timer emu_timer; */
struct _emu_timer
{
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


/* configuration of a single timer device */
typedef struct _timer_state timer_state;
struct _timer_state
{
	emu_timer				*timer;			/* the backing timer */
	void					*ptr;			/* the pointer parameter passed to the timer callback */

	/* periodic timers only */
	attotime				start_delay;	/* delay before the timer fires for the first time */
	attotime				period;			/* period of repeated timer firings */
	INT32					param;			/* the integer parameter passed to the timer callback */

	/* scanline timers only */
	UINT32					first_time;		/* indicates that the system is starting */
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
	extern attotime cpuexec_override_local_time(running_machine *machine, attotime default_time);
	timer_private *global = machine->timer_data;

	/* if we're currently in a callback, use the timer's expiration time as a base */
	if (global->callback_timer != NULL)
		return global->callback_timer_expire_time;

	/* if we're executing as a particular CPU, use its local time as a base */
	/* otherwise, return the global base time */
	return cpuexec_override_local_time(machine, global->exec.basetime);
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
    get_safe_token - makes sure that the passed
    in device is, in fact, a timer
-------------------------------------------------*/

INLINE timer_state *get_safe_token(running_device *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == TIMER);

	return (timer_state *)device->token;
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
			profiler_mark_start(PROFILER_TIMER_CALLBACK);
			(*timer->callback)(machine, timer->ptr, timer->param);
			profiler_mark_end();
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


void timer_device_adjust_oneshot(running_device *timer, attotime duration, INT32 param)
{
#ifdef MAME_DEBUG
	timer_config *config = (timer_config *)timer->baseconfig().inline_config;

	/* doesn't make sense for scanline timers */
	assert(config->type != TIMER_TYPE_SCANLINE);
#endif

	timer_device_adjust_periodic(timer, duration, param, attotime_never);
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
		cpuexec_abort_timeslice(which->machine);
}


void timer_device_adjust_periodic(running_device *timer, attotime start_delay, INT32 param, attotime period)
{
	timer_state *state = get_safe_token(timer);
#ifdef MAME_DEBUG
	timer_config *config = (timer_config *)timer->baseconfig().inline_config;

	/* doesn't make sense for scanline timers */
	assert(config->type != TIMER_TYPE_SCANLINE);
#endif

	state->start_delay = start_delay;
	state->period = period;
	state->param = param;

	/* adjust the timer */
	timer_adjust_periodic(state->timer, state->start_delay, 0, state->period);
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


void timer_device_reset(running_device *timer)
{
	timer_state *state = get_safe_token(timer);
#ifdef MAME_DEBUG
	timer_config *config = (timer_config *)timer->baseconfig().inline_config;

	/* doesn't make sense for scanline timers */
	assert(config->type != TIMER_TYPE_SCANLINE);
#endif

	timer_adjust_periodic(state->timer, state->start_delay, 0, state->period);
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


int timer_device_enable(running_device *timer, int enable)
{
	timer_state *state = get_safe_token(timer);
	return timer_enable(state->timer, enable);
}


/*-------------------------------------------------
    timer_enabled - determine if a timer is
    enabled
-------------------------------------------------*/

int timer_enabled(emu_timer *which)
{
	return which->enabled;
}


int timer_device_enabled(running_device *timer)
{
	timer_state *state = get_safe_token(timer);
	return timer_enabled(state->timer);
}


/*-------------------------------------------------
    timer_get_param - returns the callback
    parameter of a timer
-------------------------------------------------*/

int timer_get_param(emu_timer *which)
{
	return which->param;
}


int timer_device_get_param(running_device *timer)
{
	timer_state *state = get_safe_token(timer);
#ifdef MAME_DEBUG
	timer_config *config = (timer_config *)timer->baseconfig().inline_config;

	/* doesn't make sense for scanline timers */
	assert(config->type != TIMER_TYPE_SCANLINE);
#endif

	return state->param;
}


/*-------------------------------------------------
    timer_set_param - changes the callback
    parameter of a timer
-------------------------------------------------*/

void timer_set_param(emu_timer *which, int param)
{
	which->param = param;
}


void timer_device_set_param(running_device *timer, int param)
{
	timer_state *state = get_safe_token(timer);
#ifdef MAME_DEBUG
	timer_config *config = (timer_config *)timer->baseconfig().inline_config;

	/* doesn't make sense for scanline timers */
	assert(config->type != TIMER_TYPE_SCANLINE);
#endif

	state->param = param;
}


/*-------------------------------------------------
    timer_get_ptr - returns the callback pointer
    of a timer
-------------------------------------------------*/

void *timer_get_ptr(emu_timer *which)
{
	return which->ptr;
}


void *timer_device_get_ptr(running_device *timer)
{
	timer_state *state = get_safe_token(timer);
	return state->ptr;
}


/*-------------------------------------------------
    timer_set_ptr - changes the callback pointer
    of a timer
-------------------------------------------------*/

void timer_set_ptr(emu_timer *which, void *ptr)
{
	which->ptr = ptr;
}


void timer_device_set_ptr(running_device *timer, void *ptr)
{
	timer_state *state = get_safe_token(timer);
	state->ptr = ptr;
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


attotime timer_device_timeelapsed(running_device *timer)
{
	timer_state *state = get_safe_token(timer);
	return timer_timeelapsed(state->timer);
}


/*-------------------------------------------------
    timer_timeleft - return the time until the
    next trigger
-------------------------------------------------*/

attotime timer_timeleft(emu_timer *which)
{
	return attotime_sub(which->expire, get_current_time(which->machine));
}


attotime timer_device_timeleft(running_device *timer)
{
	timer_state *state = get_safe_token(timer);
	return timer_timeleft(state->timer);
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


attotime timer_device_starttime(running_device *timer)
{
	timer_state *state = get_safe_token(timer);
	return timer_starttime(state->timer);
}


/*-------------------------------------------------
    timer_firetime - return the time when this
    timer will fire next
-------------------------------------------------*/

attotime timer_firetime(emu_timer *which)
{
	return which->expire;
}


attotime timer_device_firetime(running_device *timer)
{
	timer_state *state = get_safe_token(timer);
	return timer_firetime(state->timer);
}


/*-------------------------------------------------
    periodic_timer_device_timer_callback - calls
    the timer device specific callback
-------------------------------------------------*/

static TIMER_CALLBACK( periodic_timer_device_timer_callback )
{
	running_device *timer = (running_device *)ptr;
	timer_state *state = get_safe_token(timer);
	timer_config *config = (timer_config *)timer->baseconfig().inline_config;

	/* call the real callback */
	config->callback(timer, state->ptr, state->param);
}



/*-------------------------------------------------
    scanline_timer_device_timer_callback -
    manages the scanline based timer's state
-------------------------------------------------*/

static TIMER_CALLBACK( scanline_timer_device_timer_callback )
{
	int next_vpos;
	running_device *timer = (running_device *)ptr;
	timer_state *state = get_safe_token(timer);
	timer_config *config = (timer_config *)timer->baseconfig().inline_config;

	/* get the screen device and verify it */
	running_device *screen = timer->machine->device(config->screen);
	assert(screen != NULL);

	/* first time, start with the first scanline, but do not call the callback */
	if (state->first_time)
	{
		next_vpos = config->first_vpos;

		/* indicate that we are done with the first call */
		state->first_time = FALSE;
	}

	/* not the first time */
	else
	{
		int vpos = video_screen_get_vpos(screen);

		/* call the real callback */
		config->callback(timer, state->ptr, vpos);

		/* if the increment is 0 or the next scanline is larger than the screen size,
           go back to the first one */
        if ((config->increment == 0) ||
            ((vpos + config->increment) >= video_screen_get_height(screen)))
			next_vpos = config->first_vpos;

		/* otherwise, increment */
		else
			next_vpos = vpos + config->increment;
	}

	/* adjust the timer */
	timer_adjust_oneshot(state->timer, video_screen_get_time_until_pos(screen, next_vpos, 0), 0);
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



/***************************************************************************
    TIMER DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    timer_start - device start callback
    for a timer device
-------------------------------------------------*/

static DEVICE_START( timer )
{
	timer_state *state = get_safe_token(device);
	timer_config *config;
	void *param;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->baseconfig().static_config == NULL);
	assert(device->baseconfig().inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* get and validate the configuration */
	config = (timer_config *)device->baseconfig().inline_config;
	assert(config->type == TIMER_TYPE_PERIODIC || config->type == TIMER_TYPE_SCANLINE || config->type == TIMER_TYPE_GENERIC);

	/* copy the pointer parameter */
	state->ptr = config->ptr;

	/* type based configuration */
	switch (config->type)
	{
		case TIMER_TYPE_GENERIC:
			/* make sure that only the applicable parameters are filled in */
			assert(config->screen == NULL);
			assert(config->first_vpos == 0);
			assert(config->increment == 0);
			assert(config->start_delay == 0);
			assert(config->period == 0);

			/* copy the optional integer parameter */
			state->param = config->param;

			/* convert the start_delay and period into attotime */
			state->period = attotime_never;
			state->start_delay = attotime_zero;

			/* register for state saves */
			state_save_register_device_item(device, 0, state->param);

			/* allocate the backing timer */
			param = (void *)device;
			state->timer = timer_alloc(device->machine, periodic_timer_device_timer_callback, param);
			break;

		case TIMER_TYPE_PERIODIC:
			/* make sure that only the applicable parameters are filled in */
			assert(config->screen == NULL);
			assert(config->first_vpos == 0);
			assert(config->increment == 0);

			/* validate that we have at least a start_delay or period */
			assert(config->period > 0);

			/* copy the optional integer parameter */
			state->param = config->param;

			/* convert the start_delay and period into attotime */
			state->period = UINT64_ATTOTIME_TO_ATTOTIME(config->period);

			if (config->start_delay > 0)
				state->start_delay = UINT64_ATTOTIME_TO_ATTOTIME(config->start_delay);
			else
				state->start_delay = attotime_zero;

			/* register for state saves */
			state_save_register_device_item(device, 0, state->start_delay.seconds);
			state_save_register_device_item(device, 0, state->start_delay.attoseconds);
			state_save_register_device_item(device, 0, state->period.seconds);
			state_save_register_device_item(device, 0, state->period.attoseconds);
			state_save_register_device_item(device, 0, state->param);

			/* allocate the backing timer */
			param = (void *)device;
			state->timer = timer_alloc(device->machine, periodic_timer_device_timer_callback, param);

			/* finally, start the timer */
			timer_adjust_periodic(state->timer, state->start_delay, 0, state->period);
			break;

		case TIMER_TYPE_SCANLINE:
			/* make sure that only the applicable parameters are filled in */
			assert(config->start_delay == 0);
			assert(config->period == 0);
			assert(config->param == 0);

			assert(config->first_vpos >= 0);
			assert(config->increment >= 0);

			/* allocate the backing timer */
			param = (void *)device;
			state->timer = timer_alloc(device->machine, scanline_timer_device_timer_callback, param);

			/* indicate that this will be the first call */
			state->first_time = TRUE;

			/* register for state saves */
			state_save_register_device_item(device, 0, state->first_time);

			/* fire it as soon as the emulation starts */
			timer_adjust_oneshot(state->timer, attotime_zero, 0);
			break;

		default:
			/* we will never get here */
			fatalerror("Unknown timer device type");
			break;
	}
}


/*-------------------------------------------------
    timer_get_info - device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( timer )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(timer_state);			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(timer_config);			break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_TIMER;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(timer); break;
		case DEVINFO_FCT_STOP:					/* Nothing */							break;
		case DEVINFO_FCT_RESET:					/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Generic");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Timer");				break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}
