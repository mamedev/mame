/* System 16 and friends FD1094 handling */

/*
todo:

support multiple FD1094s (does anything /use/ multiple FD1094s?)
make more configurable (select caches per game?)

*/

#include "driver.h"
#include "deprecat.h"

#include "cpu/m68000/m68000.h"
#include "machine/fd1094.h"


#define CACHE_ENTRIES	8

static UINT8 *fd1094_key; // the memory region containing key
static UINT16 *fd1094_cpuregion; // the CPU region with encrypted code
static UINT32  fd1094_cpuregionsize; // the size of this region in bytes

static UINT16 *fd1094_userregion; // a user region where the current decrypted state is put and executed from
static UINT16 *fd1094_cacheregion[CACHE_ENTRIES]; // a cache region where CACHE_ENTRIES states are stored to improve performance
static int fd1094_cached_states[CACHE_ENTRIES]; // array of cached state numbers
static int fd1094_current_cacheposition; // current position in cache array

static int fd1094_state;
static int fd1094_selected_state;

static void (*fd1094_set_decrypted)(UINT8 *);

void *fd1094_get_decrypted_base(void)
{
	if (!fd1094_key)
		return NULL;
	return fd1094_userregion;
}

static void set_decrypted_region(void)
{
	if (fd1094_set_decrypted != NULL)
		(*fd1094_set_decrypted)((UINT8 *)fd1094_userregion);
	else
		memory_set_decrypted_region(0, 0, fd1094_cpuregionsize - 1, fd1094_userregion);
}

/* this function checks the cache to see if the current state is cached,
   if it is then it copies the cached data to the user region where code is
   executed from, if its not cached then it gets decrypted to the current
   cache position using the functions in fd1094.c */
static void fd1094_setstate_and_decrypt(int state)
{
	int i;
	UINT32 addr;

	switch (state & 0x300)
	{
	case 0x000:
	case FD1094_STATE_RESET:
		fd1094_selected_state = state & 0xff;
		break;
	}

	fd1094_state = state;

	cpunum_set_info_int(0, CPUINFO_INT_REGISTER + M68K_PREF_ADDR, 0x0010);	// force a flush of the prefetch cache

	/* set the FD1094 state ready to decrypt.. */
	state = fd1094_set_state(fd1094_key,state) & 0xff;

	/* first check the cache, if its cached we don't need to decrypt it, just copy */
	for (i=0;i<CACHE_ENTRIES;i++)
	{
		if (fd1094_cached_states[i] == state)
		{
			/* copy cached state */
			fd1094_userregion=fd1094_cacheregion[i];
			set_decrypted_region();
			m68k_set_encrypted_opcode_range(0,0,fd1094_cpuregionsize);

			return;
		}
	}

	/* mark it as cached (because it will be once we decrypt it) */
	fd1094_cached_states[fd1094_current_cacheposition]=state;

	for (addr=0;addr<fd1094_cpuregionsize/2;addr++)
	{
		UINT16 dat;
		dat = fd1094_decode(addr,fd1094_cpuregion[addr],fd1094_key,0);
		fd1094_cacheregion[fd1094_current_cacheposition][addr]=dat;
	}

	/* copy newly decrypted data to user region */
	fd1094_userregion=fd1094_cacheregion[fd1094_current_cacheposition];
	set_decrypted_region();
	m68k_set_encrypted_opcode_range(0,0,fd1094_cpuregionsize);

	fd1094_current_cacheposition++;

	if (fd1094_current_cacheposition>=CACHE_ENTRIES)
	{
		mame_printf_debug("out of cache, performance may suffer, incrase CACHE_ENTRIES!\n");
		fd1094_current_cacheposition=0;
	}
}

/* Callback for CMP.L instructions (state change) */
static void fd1094_cmp_callback(UINT32 val, int reg)
{
	if (reg == 0 && (val & 0x0000ffff) == 0x0000ffff) // ?
	{
		fd1094_setstate_and_decrypt((val & 0xffff0000) >> 16);
	}
}

/* Callback when the FD1094 enters interrupt code */
static IRQ_CALLBACK(fd1094_int_callback)
{
	fd1094_setstate_and_decrypt(FD1094_STATE_IRQ);
	return (0x60+irqline*4)/4; // vector address
}

static void fd1094_rte_callback (void)
{
	fd1094_setstate_and_decrypt(FD1094_STATE_RTE);
}


/* KLUDGE, set the initial PC / SP based on table as we can't decrypt them yet */
static void fd1094_kludge_reset_values(void)
{
	int i;

	for (i = 0;i < 4;i++)
		fd1094_userregion[i] = fd1094_decode(i,fd1094_cpuregion[i],fd1094_key,1);
}


/* function, to be called from MACHINE_RESET (every reset) */
void fd1094_machine_init(void)
{
	/* punt if no key; this allows us to be called even for non-FD1094 games */
	if (!fd1094_key)
		return;

	fd1094_setstate_and_decrypt(FD1094_STATE_RESET);
	fd1094_kludge_reset_values();

	cpunum_set_info_fct(0, CPUINFO_PTR_M68K_CMPILD_CALLBACK, (genf *)fd1094_cmp_callback);
	cpunum_set_info_fct(0, CPUINFO_PTR_M68K_RTE_CALLBACK, (genf *)fd1094_rte_callback);
	cpunum_set_irq_callback(0, fd1094_int_callback);
}

static STATE_POSTLOAD( fd1094_postload )
{
	if (fd1094_state != -1)
	{
		int selected_state = fd1094_selected_state;
		int state = fd1094_state;

		fd1094_machine_init();

		fd1094_setstate_and_decrypt(selected_state);
		fd1094_setstate_and_decrypt(state);
	}
}


#ifdef ENABLE_DEBUGGER
static void key_changed(void)
{
	int addr;

	/* re-decode the against the current parameter into cache entry 0 */
	for (addr = 0; addr < fd1094_cpuregionsize/2; addr++)
	{
		UINT16 dat;
		dat = fd1094_decode(addr,fd1094_cpuregion[addr],fd1094_key,0);
		fd1094_cacheregion[0][addr]=dat;
	}

	/* set cache entry 0 to be the active one, and reset the cache position to 1 */
	fd1094_userregion = fd1094_cacheregion[0];
	set_decrypted_region();
	fd1094_current_cacheposition = 1;

	/* flush the prefetch queue */
	cpunum_set_info_int(0, CPUINFO_INT_REGISTER + M68K_PREF_ADDR, 0x0010);
}
#endif


/* startup function, to be called from DRIVER_INIT (once on startup) */
void fd1094_driver_init(void (*set_decrypted)(UINT8 *))
{
	int i;

	fd1094_cpuregion = (UINT16*)memory_region(REGION_CPU1);
	fd1094_cpuregionsize = memory_region_length(REGION_CPU1);
	fd1094_key = memory_region(REGION_USER1);
	fd1094_set_decrypted = set_decrypted;

	/* punt if no key; this allows us to be called even for non-FD1094 games */
	if (fd1094_key == NULL)
		return;

	for (i = 0; i < CACHE_ENTRIES; i++)
	{
		fd1094_cacheregion[i] = auto_malloc(fd1094_cpuregionsize);
		fd1094_cached_states[i] = -1;
	}
  	fd1094_current_cacheposition = 0;
	fd1094_state = -1;

#ifdef ENABLE_DEBUGGER
	/* key debugging */
	if (Machine->debug_mode && memory_region(REGION_USER2) != NULL)
	{
		void fd1094_init_debugging(int, int, int, void (*changed)(void));
		fd1094_init_debugging(REGION_CPU1, REGION_USER1, REGION_USER2, key_changed);
	}
#endif

	state_save_register_global(fd1094_selected_state);
	state_save_register_global(fd1094_state);
	state_save_register_postload(Machine, fd1094_postload, NULL);
}
