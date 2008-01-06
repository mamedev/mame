/***************************************************************************

    timer.c

    Functions needed to generate timing and synchronization between several
    CPUs.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "driver.h"
#include "profiler.h"
#include "pool.h"
#include <math.h>


/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define MAX_TIMERS		256



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* in timer.h: typedef struct _emu_timer emu_timer; */
struct _emu_timer
{
	emu_timer *		next;
	emu_timer *		prev;
	timer_callback	callback;
	INT32 			param;
	void *			ptr;
	const char *	file;
	int 			line;
	const char *	func;
	UINT8 			enabled;
	UINT8 			temporary;
	attotime 		period;
	attotime 		start;
	attotime 		expire;
};



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

/* conversion constants */
attoseconds_t attoseconds_per_cycle[MAX_CPU];
UINT32 cycles_per_second[MAX_CPU];

/* list of active timers */
static emu_timer timers[MAX_TIMERS];
static emu_timer *timer_head;
static emu_timer *timer_free_head;
static emu_timer *timer_free_tail;

/* other internal states */
static attotime global_basetime;
static emu_timer *callback_timer;
static int callback_timer_modified;
static attotime callback_timer_expire_time;



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void timer_postload(void);
static void timer_logtimers(void);
static void timer_remove(emu_timer *which);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_current_time - return the current time
-------------------------------------------------*/

INLINE attotime get_current_time(void)
{
	int activecpu;

	/* if we're currently in a callback, use the timer's expiration time as a base */
	if (callback_timer != NULL)
		return callback_timer_expire_time;

	/* if we're executing as a particular CPU, use its local time as a base */
	activecpu = cpu_getactivecpu();
	if (activecpu >= 0)
		return cpunum_get_localtime(activecpu);

	/* otherwise, return the current global base time */
	return global_basetime;
}


/*-------------------------------------------------
    timer_new - allocate a new timer
-------------------------------------------------*/

INLINE emu_timer *timer_new(void)
{
	emu_timer *timer;

	/* remove an empty entry */
	if (!timer_free_head)
	{
		timer_logtimers();
		fatalerror("Out of timers!");
	}
	timer = timer_free_head;
	timer_free_head = timer->next;
	if (!timer_free_head)
		timer_free_tail = NULL;

	return timer;
}


/*-------------------------------------------------
    timer_list_insert - insert a new timer into
    the list at the appropriate location
-------------------------------------------------*/

INLINE void timer_list_insert(emu_timer *timer)
{
	attotime expire = timer->enabled ? timer->expire : attotime_never;
	emu_timer *t, *lt = NULL;

	/* sanity checks for the debug build */
	#ifdef MAME_DEBUG
	{
		int tnum = 0;

		/* loop over the timer list */
		for (t = timer_head; t; t = t->next, tnum++)
		{
			if (t == timer)
				fatalerror("This timer is already inserted in the list!");
			if (tnum == MAX_TIMERS-1)
				fatalerror("Timer list is full!");
		}
	}
	#endif

	/* loop over the timer list */
	for (t = timer_head; t; lt = t, t = t->next)
	{
		/* if the current list entry expires after us, we should be inserted before it */
		if (attotime_compare(t->expire, expire) > 0)
		{
			/* link the new guy in before the current list entry */
			timer->prev = t->prev;
			timer->next = t;

			if (t->prev)
				t->prev->next = timer;
			else
				timer_head = timer;
			t->prev = timer;
			return;
		}
	}

	/* need to insert after the last one */
	if (lt)
		lt->next = timer;
	else
		timer_head = timer;
	timer->prev = lt;
	timer->next = NULL;
}


/*-------------------------------------------------
    timer_list_remove - remove a timer from the
    linked list
-------------------------------------------------*/

INLINE void timer_list_remove(emu_timer *timer)
{
	/* sanity checks for the debug build */
	#ifdef MAME_DEBUG
	{
		emu_timer *t;

		/* loop over the timer list */
		for (t = timer_head; t && t != timer; t = t->next) ;
		if (t == NULL)
			fatalerror("timer (%s from %s:%d) not found in list", timer->func, timer->file, timer->line);
	}
	#endif

	/* remove it from the list */
	if (timer->prev)
		timer->prev->next = timer->next;
	else
		timer_head = timer->next;
	if (timer->next)
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
	int i;

	/* we need to wait until the first call to timer_cyclestorun before using real CPU times */
	global_basetime = attotime_zero;
	callback_timer = NULL;
	callback_timer_modified = FALSE;

	/* register with the save state system */
	state_save_push_tag(0);
	state_save_register_item("timer", 0, global_basetime.seconds);
	state_save_register_item("timer", 0, global_basetime.attoseconds);
	state_save_register_func_postload(timer_postload);
	state_save_pop_tag();

	/* reset the timers */
	memset(timers, 0, sizeof(timers));

	/* initialize the lists */
	timer_head = NULL;
	timer_free_head = &timers[0];
	for (i = 0; i < MAX_TIMERS-1; i++)
		timers[i].next = &timers[i+1];
	timers[MAX_TIMERS-1].next = NULL;
	timer_free_tail = &timers[MAX_TIMERS-1];
}


/*-------------------------------------------------
    timer_destructor - destruct a timer from a
    pool callback
-------------------------------------------------*/

void timer_destructor(void *ptr, size_t size)
{
	timer_remove(ptr);
}



/***************************************************************************
    SCHEDULING HELPERS
***************************************************************************/

/*-------------------------------------------------
    timer_next_fire_time - return the
    time when the next timer will fire
-------------------------------------------------*/

attotime timer_next_fire_time(void)
{
	return timer_head->expire;
}


/*-------------------------------------------------
    timer_adjust_global_time - adjust the global
    time; this is also where we fire the timers
-------------------------------------------------*/

void timer_set_global_time(attotime newbase)
{
	emu_timer *timer;

	/* set the new global offset */
	global_basetime = newbase;

	LOG(("timer_set_global_time: new=%s head->expire=%s\n", attotime_string(newbase, 9), attotime_string(timer_head->expire, 9)));

	/* now process any timers that are overdue */
	while (attotime_compare(timer_head->expire, global_basetime) <= 0)
	{
		int was_enabled = timer_head->enabled;

		/* if this is a one-shot timer, disable it now */
		timer = timer_head;
		if (attotime_compare(timer->period, attotime_zero) == 0 || attotime_compare(timer->period, attotime_never) == 0)
			timer->enabled = FALSE;

		/* set the global state of which callback we're in */
		callback_timer_modified = FALSE;
		callback_timer = timer;
		callback_timer_expire_time = timer->expire;

		/* call the callback */
		if (was_enabled && timer->callback != NULL)
		{
			LOG(("Timer %s:%d[%s] fired (expire=%s)\n", timer->file, timer->line, timer->func, attotime_string(timer->expire, 9)));
			profiler_mark(PROFILER_TIMER_CALLBACK);
			(*timer->callback)(Machine, timer->ptr, timer->param);
			profiler_mark(PROFILER_END);
		}

		/* clear the callback timer global */
		callback_timer = NULL;

		/* reset or remove the timer, but only if it wasn't modified during the callback */
		if (!callback_timer_modified)
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



/***************************************************************************
    SAVE/RESTORE HELPERS
***************************************************************************/

/*-------------------------------------------------
    timer_register_save - register ourself with
    the save state system
-------------------------------------------------*/

static void timer_register_save(emu_timer *timer)
{
	char buf[256];
	int count = 0;
	emu_timer *t;

	/* find other timers that match our func name */
	for (t = timer_head; t; t = t->next)
		if (!strcmp(t->func, timer->func))
			count++;

	/* make up a name */
	sprintf(buf, "timer.%s", timer->func);

	/* use different instances to differentiate the bits */
	state_save_push_tag(0);
	state_save_register_item(buf, count, timer->param);
	state_save_register_item(buf, count, timer->enabled);
	state_save_register_item(buf, count, timer->period.seconds);
	state_save_register_item(buf, count, timer->period.attoseconds);
	state_save_register_item(buf, count, timer->start.seconds);
	state_save_register_item(buf, count, timer->start.attoseconds);
	state_save_register_item(buf, count, timer->expire.seconds);
	state_save_register_item(buf, count, timer->expire.attoseconds);
	state_save_pop_tag();
}


/*-------------------------------------------------
    timer_postload - after loading a save state
-------------------------------------------------*/

static void timer_postload(void)
{
	emu_timer *privlist = NULL;
	emu_timer *t;

	/* remove all timers and make a private list */
	while (timer_head != NULL)
	{
		t = timer_head;

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

int timer_count_anonymous(void)
{
	emu_timer *t;
	int count = 0;

	logerror("timer_count_anonymous:\n");
	for (t = timer_head; t; t = t->next)
		if (t->temporary && t != callback_timer)
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

INLINE emu_timer *_timer_alloc_common(timer_callback callback, void *ptr, const char *file, int line, const char *func, int temp)
{
	attotime time = get_current_time();
	emu_timer *timer = timer_new();

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

	/* if we're not temporary, register ourselve with the save state system */
	if (!temp)
	{
		timer_register_save(timer);
		restrack_register_object(OBJTYPE_TIMER, timer, 0, file, line);
	}

	/* return a handle */
	return timer;
}

emu_timer *_timer_alloc_internal(timer_callback callback, void *ptr, const char *file, int line, const char *func)
{
	return _timer_alloc_common(callback, ptr, file, line, func, FALSE);
}


/*-------------------------------------------------
    timer_remove - remove a timer from the
    system
-------------------------------------------------*/

static void timer_remove(emu_timer *which)
{
	/* if this is a callback timer, note that */
	if (which == callback_timer)
		callback_timer_modified = TRUE;

	/* remove it from the list */
	timer_list_remove(which);

	/* free it up by adding it back to the free list */
	if (timer_free_tail)
		timer_free_tail->next = which;
	else
		timer_free_head = which;
	which->next = NULL;
	timer_free_tail = which;
}



/***************************************************************************
    CORE TIMER ADJUSTMENT
***************************************************************************/

/*-------------------------------------------------
    timer_adjust - adjust the time when this
    timer will fire, and whether or not it will
    fire periodically
-------------------------------------------------*/

void timer_adjust(emu_timer *which, attotime duration, INT32 param, attotime period)
{
	attotime time = get_current_time();

	/* if this is the callback timer, mark it modified */
	if (which == callback_timer)
		callback_timer_modified = TRUE;

	/* compute the time of the next firing and insert into the list */
	which->param = param;
	which->enabled = TRUE;

	/* clamp negative times to 0 */
	if (duration.seconds < 0)
		duration = attotime_zero;

	/* set the start and expire times */
	which->start = time;
	which->expire = attotime_add(time, duration);
	which->period = period;

	/* remove and re-insert the timer in its new order */
	timer_list_remove(which);
	timer_list_insert(which);

	/* if this was inserted as the head, abort the current timeslice and resync */
	LOG(("timer_adjust %s.%s:%d to expire @ %s\n", which->file, which->func, which->line, attotime_string(which->expire, 9)));
	if (which == timer_head && cpu_getexecutingcpu() >= 0)
		activecpu_abort_timeslice();
}



/***************************************************************************
    SIMPLIFIED ANONYMOUS TIMER MANAGEMENT
***************************************************************************/

/*-------------------------------------------------
    timer_pulse - allocate a pulse timer, which
    repeatedly calls the callback using the given
    period
-------------------------------------------------*/

void _timer_pulse_internal(attotime period, void *ptr, INT32 param, timer_callback callback, const char *file, int line, const char *func)
{
	emu_timer *timer = _timer_alloc_common(callback, ptr, file, line, func, FALSE);
	timer_adjust(timer, period, param, period);
}


/*-------------------------------------------------
    timer_set - allocate a one-shot timer, which
    calls the callback after the given duration
-------------------------------------------------*/

void _timer_set_internal(attotime duration, void *ptr, INT32 param, timer_callback callback, const char *file, int line, const char *func)
{
	emu_timer *timer = _timer_alloc_common(callback, ptr, file, line, func, TRUE);
	timer_adjust(timer, duration, param, attotime_zero);
}



/***************************************************************************
    MISCELLANEOUS CONTROLS
***************************************************************************/

/*-------------------------------------------------
    timer_reset - reset the timing on a timer
-------------------------------------------------*/

void timer_reset(emu_timer *which, attotime duration)
{
	timer_adjust(which, duration, which->param, which->period);
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
    timer_get_param
    timer_get_param_ptr - returns the callback
    parameter of a timer
-------------------------------------------------*/

int timer_get_param(emu_timer *which)
{
	return which->param;
}


void *timer_get_param_ptr(emu_timer *which)
{
	return which->ptr;
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
	return attotime_sub(get_current_time(), which->start);
}


/*-------------------------------------------------
    timer_timeleft - return the time until the
    next trigger
-------------------------------------------------*/

attotime timer_timeleft(emu_timer *which)
{
	return attotime_sub(which->expire, get_current_time());
}


/*-------------------------------------------------
    timer_get_time - return the current time
-------------------------------------------------*/

attotime timer_get_time(void)
{
	return get_current_time();
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

static void timer_logtimers(void)
{
	emu_timer *t;

	logerror("===============\n");
	logerror("TIMER LOG START\n");
	logerror("===============\n");

	logerror("Enqueued timers:\n");
	for (t = timer_head; t; t = t->next)
		logerror("  Start=%15.6f Exp=%15.6f Per=%15.6f Ena=%d Tmp=%d (%s:%d[%s])\n",
			attotime_to_double(t->start), attotime_to_double(t->expire), attotime_to_double(t->period), t->enabled, t->temporary, t->file, t->line, t->func);

	logerror("Free timers:\n");
	for (t = timer_free_head; t; t = t->next)
		logerror("  Start=%15.6f Exp=%15.6f Per=%15.6f Ena=%d Tmp=%d (%s:%d[%s])\n",
			attotime_to_double(t->start), attotime_to_double(t->expire), attotime_to_double(t->period), t->enabled, t->temporary, t->file, t->line, t->func);

	logerror("==============\n");
	logerror("TIMER LOG STOP\n");
	logerror("==============\n");
}
