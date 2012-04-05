/* s16fd.c modified to support s24

this could get messy if games change their own code after initial loading as we'll have to invalidate caches etc.


*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/fd1094.h"
#include "includes/segas24.h"

#define S16_NUMCACHE 8

static UINT8 *s24_fd1094_key; // the memory region containing key
static UINT16 *s24_fd1094_cpuregion; // the CPU region with encrypted code
static UINT32  s24_fd1094_cpuregionsize; // the size of this region in bytes

static UINT16* s24_fd1094_userregion; // a user region where the current decrypted state is put and executed from
static UINT16* s24_fd1094_cacheregion[S16_NUMCACHE]; // a cache region where S16_NUMCACHE states are stored to improve performance
static int fd1094_cached_states[S16_NUMCACHE]; // array of cached state numbers
static int fd1094_current_cacheposition; // current position in cache array

static int fd1094_state;
static int fd1094_selected_state;

/* this function checks the cache to see if the current state is cached,
   if it is then it copies the cached data to the user region where code is
   executed from, if its not cached then it gets decrypted to the current
   cache position using the functions in s24_fd1094.c */
static void s24_fd1094_setstate_and_decrypt(running_machine &machine, int state)
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

	cpu_set_reg(machine.device("sub"), M68K_PREF_ADDR, 0x0010);	// force a flush of the prefetch cache

	/* set the s24_fd1094 state ready to decrypt.. */
	state = fd1094_set_state(s24_fd1094_key,state) & 0xff;

	/* first check the cache, if its cached we don't need to decrypt it, just copy */
	for (i = 0; i < S16_NUMCACHE; i++)
	{
		if (fd1094_cached_states[i] == state)
		{
			/* copy cached state */
			s24_fd1094_userregion = s24_fd1094_cacheregion[i];
			machine.device<cpu_device>("sub")->space(AS_PROGRAM)->set_decrypted_region(0, s24_fd1094_cpuregionsize - 1, s24_fd1094_userregion);
			m68k_set_encrypted_opcode_range(machine.device("sub"), 0, s24_fd1094_cpuregionsize);

			return;
		}
	}

// mame_printf_debug("new state %04x\n",state);

	/* mark it as cached (because it will be once we decrypt it) */
	fd1094_cached_states[fd1094_current_cacheposition] = state;

	for (addr = 0; addr < s24_fd1094_cpuregionsize / 2; addr++)
	{
		UINT16 dat;
		dat = fd1094_decode(addr, s24_fd1094_cpuregion[addr], s24_fd1094_key, 0);
		s24_fd1094_cacheregion[fd1094_current_cacheposition][addr] = dat;
	}

	/* copy newly decrypted data to user region */
	s24_fd1094_userregion = s24_fd1094_cacheregion[fd1094_current_cacheposition];
	machine.device<cpu_device>("sub")->space(AS_PROGRAM)->set_decrypted_region(0, s24_fd1094_cpuregionsize - 1, s24_fd1094_userregion);
	m68k_set_encrypted_opcode_range(machine.device("sub"), 0, s24_fd1094_cpuregionsize);

	fd1094_current_cacheposition++;

	if (fd1094_current_cacheposition >= S16_NUMCACHE)
	{
		mame_printf_debug("out of cache, performance may suffer, incrase S16_NUMCACHE!\n");
		fd1094_current_cacheposition = 0;
	}
}

/* Callback for CMP.L instructions (state change) */
static void s24_fd1094_cmp_callback(device_t *device, UINT32 val, UINT8 reg)
{
	if (reg == 0 && (val & 0x0000ffff) == 0x0000ffff) // ?
	{
		s24_fd1094_setstate_and_decrypt(device->machine(), (val & 0xffff0000) >> 16);
	}
}

/* Callback when the s24_fd1094 enters interrupt code */
static IRQ_CALLBACK(s24_fd1094_int_callback)
{
	s24_fd1094_setstate_and_decrypt(device->machine(), FD1094_STATE_IRQ);
	return (0x60+irqline*4)/4; // vector address
}

static void s24_fd1094_rte_callback (device_t *device)
{
	s24_fd1094_setstate_and_decrypt(device->machine(), FD1094_STATE_RTE);
}


/* KLUDGE, set the initial PC / SP based on table as we can't decrypt them yet */
static void s24_fd1094_kludge_reset_values(void)
{
	int i;

	for (i = 0; i < 4; i++)
		s24_fd1094_userregion[i] = fd1094_decode(i, s24_fd1094_cpuregion[i], s24_fd1094_key, 1);
}


/* function, to be called from MACHINE_RESET (every reset) */
void s24_fd1094_machine_init(running_machine &machine)
{
	/* punt if no key; this allows us to be called even for non-s24_fd1094 games */
	if (!s24_fd1094_key)
		return;

	s24_fd1094_setstate_and_decrypt(machine, FD1094_STATE_RESET);
	s24_fd1094_kludge_reset_values();

	m68k_set_cmpild_callback(machine.device("sub"), s24_fd1094_cmp_callback);
	m68k_set_rte_callback(machine.device("sub"), s24_fd1094_rte_callback);
	device_set_irq_callback(machine.device("sub"), s24_fd1094_int_callback);

	machine.device("sub")->reset();
}

static void s24_fd1094_postload(running_machine &machine)
{
	if (fd1094_state != -1)
	{
		int selected_state = fd1094_selected_state;
		int state = fd1094_state;

		s24_fd1094_machine_init(machine);

		s24_fd1094_setstate_and_decrypt(machine, selected_state);
		s24_fd1094_setstate_and_decrypt(machine, state);
	}
}

/* startup function, to be called from DRIVER_INIT (once on startup) */
void s24_fd1094_driver_init(running_machine &machine)
{
	int i;

	s24_fd1094_cpuregion = (UINT16*)machine.memory().shared("share2")->ptr();
	s24_fd1094_cpuregionsize = 0x40000;
	s24_fd1094_key = machine.region("fd1094key")->base();

	/* punt if no key; this allows us to be called even for non-s24_fd1094 games */
	if (!s24_fd1094_key)
		return;

	for (i=0;i<S16_NUMCACHE;i++)
	{
		s24_fd1094_cacheregion[i]=auto_alloc_array(machine, UINT16, s24_fd1094_cpuregionsize/2);
	}

	/* flush the cached state array */
	for (i=0;i<S16_NUMCACHE;i++)
		fd1094_cached_states[i] = -1;

	fd1094_current_cacheposition = 0;

	fd1094_state = -1;

	state_save_register_global(machine, fd1094_selected_state);
	state_save_register_global(machine, fd1094_state);
	machine.save().register_postload(save_prepost_delegate(FUNC(s24_fd1094_postload), &machine));
}
