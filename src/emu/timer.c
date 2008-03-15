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
#include "deprecat.h"


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
	timer_fired_func	callback;
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


/*-------------------------------------------------
    timer_state - configuration of a single
    timer device
-------------------------------------------------*/

typedef struct _timer_state timer_state;
struct _timer_state
{
	emu_timer				*timer;			/* the backing timer */
	void 					*ptr;			/* the pointer parameter passed to the timer callback */

	/* periodic timers only */
	attotime				start_delay;	/* delay before the timer fires for the first time */
	attotime				period;			/* period of repeated timer firings */
	INT32					param;			/* the integer parameter passed to the timer callback */

	/* scanline timers only */
	UINT32 					first_time;		/* indicates that the system is starting */
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
    get_safe_token - makes sure that the passed
    in device is, in fact, a timer
-------------------------------------------------*/

INLINE timer_state *get_safe_token(const device_config *device)
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

INLINE emu_timer *_timer_alloc_common(timer_fired_func callback, void *ptr, const char *file, int line, const char *func, int temp)
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

	/* if we're not temporary, register ourselves with the save state system */
	if (!temp)
	{
		timer_register_save(timer);
		restrack_register_object(OBJTYPE_TIMER, timer, 0, file, line);
	}

	/* return a handle */
	return timer;
}

emu_timer *_timer_alloc_internal(timer_fired_func callback, void *ptr, const char *file, int line, const char *func)
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
    timer_adjust_oneshot - adjust the time when this timer
    will fire and disable any periodic firings
-------------------------------------------------*/

void timer_adjust_oneshot(emu_timer *which, attotime duration, INT32 param)
{
	timer_adjust_periodic(which, duration, param, attotime_never);
}


void timer_device_adjust_oneshot(const device_config *timer, attotime duration, INT32 param)
{
#ifndef NDEBUG
	timer_config *config = timer->inline_config;

	/* only makes sense for periodic timers */
	assert(config->type == TIMER_TYPE_PERIODIC);
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
	attotime time = get_current_time();

	/* if this is the callback timer, mark it modified */
	if (which == callback_timer)
		callback_timer_modified = TRUE;

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
	if (which == timer_head && cpu_getexecutingcpu() >= 0)
		activecpu_abort_timeslice();
}


void timer_device_adjust_periodic(const device_config *timer, attotime start_delay, INT32 param, attotime period)
{
	timer_state *state = get_safe_token(timer);
#ifndef NDEBUG
	timer_config *config = timer->inline_config;

	/* only makes sense for periodic timers */
	assert(config->type == TIMER_TYPE_PERIODIC);
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

void _timer_pulse_internal(attotime period, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func)
{
	emu_timer *timer = _timer_alloc_common(callback, ptr, file, line, func, FALSE);
	timer_adjust_periodic(timer, period, param, period);
}


/*-------------------------------------------------
    timer_set - allocate a one-shot timer, which
    calls the callback after the given duration
-------------------------------------------------*/

void _timer_set_internal(attotime duration, void *ptr, INT32 param, timer_fired_func callback, const char *file, int line, const char *func)
{
	emu_timer *timer = _timer_alloc_common(callback, ptr, file, line, func, TRUE);
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


void timer_device_reset(const device_config *timer)
{
	timer_state *state = get_safe_token(timer);
#ifndef NDEBUG
	timer_config *config = timer->inline_config;

	/* only makes sense for periodic timers */
	assert(config->type == TIMER_TYPE_PERIODIC);
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


int timer_device_enable(const device_config *timer, int enable)
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


int timer_device_enabled(const device_config *timer)
{
	timer_state *state = get_safe_token(timer);
	return timer_enabled(state->timer);
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


int timer_device_get_param(const device_config *timer)
{
	timer_state *state = get_safe_token(timer);
#ifndef NDEBUG
	timer_config *config = timer->inline_config;

	/* only makes sense for periodic timers */
	assert(config->type == TIMER_TYPE_PERIODIC);
#endif

	return state->param;
}


void *timer_get_param_ptr(emu_timer *which)
{
	return which->ptr;
}


void *timer_device_get_param_ptr(const device_config *timer)
{
	timer_state *state = get_safe_token(timer);
	return state->ptr;
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


attotime timer_device_timeelapsed(const device_config *timer)
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
	return attotime_sub(which->expire, get_current_time());
}


attotime timer_device_timeleft(const device_config *timer)
{
	timer_state *state = get_safe_token(timer);
	return timer_timeleft(state->timer);
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


attotime timer_device_starttime(const device_config *timer)
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


attotime timer_device_firetime(const device_config *timer)
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
	const device_config *timer = ptr;
	timer_state *state = get_safe_token(timer);
	timer_config *config = timer->inline_config;

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
	const device_config *timer = ptr;
	timer_state *state = get_safe_token(timer);
	timer_config *config = timer->inline_config;

	/* get the screen device and verify it */
	const device_config *screen = device_list_find_by_tag(timer->machine->config->devicelist, VIDEO_SCREEN, config->screen);
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



/***************************************************************************
    TIMER DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    timer_start - device start callback
    for a timer device
-------------------------------------------------*/

static DEVICE_START( timer )
{
	char unique_tag[50];
	timer_state *state;
	timer_config *config;
	void *param;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->static_config == NULL);
	assert(device->inline_config != NULL);
	assert(device->machine != NULL);
	assert(device->machine->config != NULL);

	/* get and validate the configuration */
	config = device->inline_config;
	assert((config->type == TIMER_TYPE_PERIODIC) || (config->type == TIMER_TYPE_SCANLINE));
	assert(config->callback != NULL);

	/* everything checks out so far, allocate the state object */
	state = auto_malloc(sizeof(*state));
	memset(state, 0, sizeof(*state));

	/* copy the pointer parameter */
	state->ptr = config->ptr;

	/* create the name for save states */
	assert(strlen(device->tag) < 30);
	state_save_combine_module_and_tag(unique_tag, "timer_device", device->tag);

	/* type based configuration */
	switch (config->type)
	{
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
		state_save_register_item(unique_tag, 0, state->start_delay.seconds);
		state_save_register_item(unique_tag, 0, state->start_delay.attoseconds);
		state_save_register_item(unique_tag, 0, state->period.seconds);
		state_save_register_item(unique_tag, 0, state->period.attoseconds);
		state_save_register_item(unique_tag, 0, state->param);

		/* allocate the backing timer */
		param = (void *)device;
		state->timer = timer_alloc(periodic_timer_device_timer_callback, param);

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
		state->timer = timer_alloc(scanline_timer_device_timer_callback, param);

		/* indicate that this will be the first call */
		state->first_time = TRUE;

		/* register for state saves */
		state_save_register_item(unique_tag, 0, state->first_time);

		/* fire it as soon as the emulation starts */
		timer_adjust_oneshot(state->timer, attotime_zero, 0);
		break;

	default:
		/* we will never get here */
		fatalerror("Unknown timer device type");
		break;
	}

	return state;
}


/*-------------------------------------------------
    timer_set_info - device set info callback
-------------------------------------------------*/

static DEVICE_SET_INFO( timer )
{
	switch (state)
	{
		/* no parameters to set */
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
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(timer_config);			break;
		case DEVINFO_INT_CLASS:					info->i = DEVICE_CLASS_TIMER;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_SET_INFO:				info->set_info = DEVICE_SET_INFO_NAME(timer); break;
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(timer); break;
		case DEVINFO_FCT_STOP:					/* Nothing */							break;
		case DEVINFO_FCT_RESET:					/* Nothing */							break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					info->s = "Generic";					break;
		case DEVINFO_STR_FAMILY:				info->s = "Timer";						break;
		case DEVINFO_STR_VERSION:				info->s = "1.0";						break;
		case DEVINFO_STR_SOURCE_FILE:			info->s = __FILE__;						break;
		case DEVINFO_STR_CREDITS:				info->s = "Copyright Nicola Salmoria and the MAME Team"; break;
	}
}
