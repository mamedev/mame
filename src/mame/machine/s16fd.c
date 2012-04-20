/* System 16 and friends FD1094 handling */

/*
todo:

support multiple FD1094s (does anything /use/ multiple FD1094s?)
make more configurable (select caches per game?)

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/fd1094.h"
#include "machine/fddebug.h"
#include "includes/segas16.h"

#define KEY_DEBUGGING 0
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

static char fd1094_cputag[64];

static void (*fd1094_set_decrypted)(running_machine &, UINT8 *);

void *fd1094_get_decrypted_base(void)
{
	if (!fd1094_key)
		return NULL;
	return fd1094_userregion;
}

static void set_decrypted_region(running_machine &machine)
{
	if (fd1094_set_decrypted != NULL)
		(*fd1094_set_decrypted)(machine, (UINT8 *)fd1094_userregion);
	else
		machine.device<cpu_device>(fd1094_cputag)->space(AS_PROGRAM)->set_decrypted_region(0, fd1094_cpuregionsize - 1, fd1094_userregion);
}

/* this function checks the cache to see if the current state is cached,
   if it is then it copies the cached data to the user region where code is
   executed from, if its not cached then it gets decrypted to the current
   cache position using the functions in fd1094.c */
static void fd1094_setstate_and_decrypt(running_machine &machine, int state)
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

	cpu_set_reg(machine.device(fd1094_cputag), M68K_PREF_ADDR, 0x0010);	// force a flush of the prefetch cache

	/* set the FD1094 state ready to decrypt.. */
	state = fd1094_set_state(fd1094_key, state) & 0xff;

	/* first check the cache, if its cached we don't need to decrypt it, just copy */
	for (i = 0; i < CACHE_ENTRIES; i++)
	{
		if (fd1094_cached_states[i] == state)
		{
			/* copy cached state */
			fd1094_userregion = fd1094_cacheregion[i];
			set_decrypted_region(machine);
			m68k_set_encrypted_opcode_range(machine.device(fd1094_cputag), 0, fd1094_cpuregionsize);

			return;
		}
	}

	/* mark it as cached (because it will be once we decrypt it) */
	fd1094_cached_states[fd1094_current_cacheposition] = state;

	for (addr = 0; addr < fd1094_cpuregionsize / 2; addr++)
	{
		UINT16 dat;
		dat = fd1094_decode(addr,fd1094_cpuregion[addr],fd1094_key,0);
		fd1094_cacheregion[fd1094_current_cacheposition][addr]=dat;
	}

	/* copy newly decrypted data to user region */
	fd1094_userregion = fd1094_cacheregion[fd1094_current_cacheposition];
	set_decrypted_region(machine);
	m68k_set_encrypted_opcode_range(machine.device(fd1094_cputag), 0, fd1094_cpuregionsize);

	fd1094_current_cacheposition++;

	if (fd1094_current_cacheposition >= CACHE_ENTRIES)
	{
		mame_printf_debug("out of cache, performance may suffer, incrase CACHE_ENTRIES!\n");
		fd1094_current_cacheposition = 0;
	}
}

/* Callback for CMP.L instructions (state change) */
static void fd1094_cmp_callback(device_t *device, UINT32 val, UINT8 reg)
{
	if (reg == 0 && (val & 0x0000ffff) == 0x0000ffff) // ?
	{
		fd1094_setstate_and_decrypt(device->machine(), (val & 0xffff0000) >> 16);
	}
}

/* Callback when the FD1094 enters interrupt code */
static IRQ_CALLBACK(fd1094_int_callback)
{
	fd1094_setstate_and_decrypt(device->machine(), FD1094_STATE_IRQ);
	return (0x60+irqline*4)/4; // vector address
}

static void fd1094_rte_callback (device_t *device)
{
	fd1094_setstate_and_decrypt(device->machine(), FD1094_STATE_RTE);
}


/* KLUDGE, set the initial PC / SP based on table as we can't decrypt them yet */
static void fd1094_kludge_reset_values(void)
{
	int i;

	for (i = 0;i < 4;i++)
		fd1094_userregion[i] = fd1094_decode(i,fd1094_cpuregion[i],fd1094_key,1);
}


/* function, to be called from MACHINE_RESET (every reset) */
void fd1094_machine_init(device_t *device)
{
	/* punt if no key; this allows us to be called even for non-FD1094 games */
	if (!fd1094_key)
		return;

	fd1094_setstate_and_decrypt(device->machine(), FD1094_STATE_RESET);
	fd1094_kludge_reset_values();

	m68k_set_cmpild_callback(device, fd1094_cmp_callback);
	m68k_set_rte_callback(device, fd1094_rte_callback);
	device_set_irq_callback(device, fd1094_int_callback);

	device->reset();
}

static void fd1094_postload(running_machine &machine)
{
	if (fd1094_state != -1)
	{
		int selected_state = fd1094_selected_state;
		int state = fd1094_state;

		fd1094_machine_init(machine.device(fd1094_cputag));

		fd1094_setstate_and_decrypt(machine, selected_state);
		fd1094_setstate_and_decrypt(machine, state);
	}
}

#if KEY_DEBUGGING
static void key_changed(running_machine &machine)
{
	int addr;

	/* re-decode the against the current parameter into cache entry 0 */
	for (addr = 0; addr < fd1094_cpuregionsize / 2; addr++)
	{
		UINT16 dat;
		dat = fd1094_decode(addr, fd1094_cpuregion[addr], fd1094_key, 0);
		fd1094_cacheregion[0][addr]=dat;
	}

	/* set cache entry 0 to be the active one, and reset the cache position to 1 */
	fd1094_userregion = fd1094_cacheregion[0];
	set_decrypted_region(machine);
	fd1094_current_cacheposition = 1;

	/* flush the prefetch queue */
	cpu_set_reg(machine.device(fd1094_cputag), M68K_PREF_ADDR, 0x0010);
}
#endif

/* startup function, to be called from DRIVER_INIT (once on startup) */
void fd1094_driver_init(running_machine &machine, const char* tag, void (*set_decrypted)(running_machine &, UINT8 *))
{
	int i;

	strcpy(fd1094_cputag, tag);

	fd1094_cpuregion = (UINT16*)machine.root_device().memregion(fd1094_cputag)->base();
	fd1094_cpuregionsize = machine.root_device().memregion(fd1094_cputag)->bytes();
	fd1094_key = machine.root_device().memregion("user1")->base();
	fd1094_set_decrypted = set_decrypted;

	/* punt if no key; this allows us to be called even for non-FD1094 games */
	if (fd1094_key == NULL)
		return;

	for (i = 0; i < CACHE_ENTRIES; i++)
	{
		fd1094_cacheregion[i] = auto_alloc_array(machine, UINT16, fd1094_cpuregionsize / 2);
		fd1094_cached_states[i] = -1;
	}
	fd1094_current_cacheposition = 0;
	fd1094_state = -1;

	/* key debugging */
#if KEY_DEBUGGING
	if ((machine.debug_flags & DEBUG_FLAG_ENABLED) != 0 && machine.root_device().memregion("user2")->base() != NULL)
	{
		fd1094_init_debugging(machine, fd1094_cputag, "user1", "user2", key_changed);
	}
#endif

	state_save_register_global(machine, fd1094_selected_state);
	state_save_register_global(machine, fd1094_state);
	machine.save().register_postload(save_prepost_delegate(FUNC(fd1094_postload), &machine));
}
