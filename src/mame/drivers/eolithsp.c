/* Eolith Speedup Handling */

/*
  This uses triggers and a scanline counter to speed up the eolith games a bit
  in some cases this results in a 100% speedup
  e.g hidden catch 25% -> 50% speed ingame

  this could probably be done a bit better using timers
*/

#include "emu.h"
#include "includes/eolithsp.h"

static int eolith_speedup_address;
static int eolith_speedup_resume_scanline;
static int eolith_vblank = 0;
static int eolith_scanline = 0;

void eolith_speedup_read(address_space *space)
{
	/* for debug */
  //if ((cpu_get_pc(&space->device())!=eolith_speedup_address) && (eolith_vblank!=1) )
  //    printf("%s:eolith speedup_read data %02x\n",space->machine().describe_context(), eolith_vblank);

	if (cpu_get_pc(&space->device())==eolith_speedup_address && eolith_vblank==0 && eolith_scanline < eolith_speedup_resume_scanline)
	{
		device_spin_until_trigger(&space->device(), 1000);
	}

}

static const struct
{
	const char *s_name;
	UINT32 speedup_address;
	int speedup_resume_scanline;

} eolith_speedup_table[] =
{
	/* eolith.c */
	{ "ironfort", 0x40020854, 240 },
	{ "hidnctch", 0x4000bba0, 240 },
	{ "raccoon",  0x40008204, 240 },
	{ "puzzlekg", 0x40029458, 240 },
	{ "hidctch2", 0x40009524, 240 },
	{ "hidctch2a",0x40029B58, 240 },
	{ "landbrk",  0x40023574, 240 },
	{ "landbrka", 0x4002446c, 240 },
	{ "nhidctch", 0x40012778, 240 },
	{ "hidctch3", 0x4001f6a0, 240 },
	{ "fort2b",   0x000081e0, 240 },
	{ "fort2ba",  0x000081e0, 240 },
	{ "penfan",   0x4001FA66, 240 },
	{ "candy",	  0x4001990C, 240 },
	/* eolith16.c */
	{ "klondkp",  0x0001a046, 240 },
	/* vegaeo.c */
	{ "crazywar",  0x00008cf8, 240 },
	{ NULL, 0, 0 }
};


void init_eolith_speedup(running_machine &machine)
{
	int n_game = 0;
	eolith_speedup_address = 0;
	eolith_speedup_resume_scanline = 0;

	while( eolith_speedup_table[ n_game ].s_name != NULL )
	{
		if( strcmp( machine.system().name, eolith_speedup_table[ n_game ].s_name ) == 0 )
		{
			eolith_speedup_address = eolith_speedup_table[ n_game ].speedup_address;
			eolith_speedup_resume_scanline = eolith_speedup_table[ n_game ].speedup_resume_scanline;

		}
		n_game++;
	}
}

/* todo, use timers instead! */
TIMER_DEVICE_CALLBACK( eolith_speedup )
{
	if (param==0)
	{
		eolith_vblank = 0;
	}

	if (param==eolith_speedup_resume_scanline)
	{
		timer.machine().scheduler().trigger(1000);
	}

	if (param==240)
	{
		eolith_vblank = 1;
	}
}

CUSTOM_INPUT( eolith_speedup_getvblank )
{
	return (field.machine().primary_screen->vpos() >= 240);
}
