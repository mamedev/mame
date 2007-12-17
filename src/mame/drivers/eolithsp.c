/* Eolith Speedup Handling */

/*
  This uses triggers and a scanline counter to speed up the eolith games a bit
  in some cases this results in a 100% speedup
  e.g hidden catch 25% -> 50% speed ingame

  this could probably be done a bit better using timers
*/

#include "driver.h"
#include "includes/eolithsp.h"

static int eolith_speedup_address;
static int eolith_speedup_resume_scanline;
static int eolith_vblank = 0;
static int eolith_scanline = 0;

void eolith_speedup_read(void)
{
	/* for debug */
//  if ((activecpu_get_pc()!=eolith_speedup_address) && (eolith_vblank!=1) )
//      printf("%06x eolith speedup_read data %02x\n",activecpu_get_pc(), eolith_vblank);

	if (activecpu_get_pc()==eolith_speedup_address && eolith_vblank==0 && eolith_scanline < eolith_speedup_resume_scanline)
	{
		cpu_spinuntil_trigger(1000);
	}

}

static struct
{
	const char *s_name;
	UINT32 speedup_address;
	int speedup_resume_scanline;

} eolith_speedup_table[] =
{
	/* eolith.c */
	{ "hidnctch", 0x4000bba0, 239 },
	{ "raccoon",  0x40008204, 239 },
	{ "puzzlekg", 0x40029458, 239 },
	{ "hidctch2", 0x40009524, 239 },
	{ "landbrk",  0x40023574, 239 },
	{ "landbrka", 0x4002446c, 239 },
	{ "nhidctch", 0x40012778, 239 },
	{ "hidctch3", 0x4001f6a0, 239 },
	{ "fort2b",   0x000081e0, 239 },
	{ "fort2ba",  0x000081e0, 239 },
	/* eolith16.c */
	{ "klondkp",  0x0001a046, 239 },
	/* vegaeo.c */
	{ "crazywar",  0x00008cf8, 239 },
	{ NULL, 0, 0 }
};


void init_eolith_speedup(running_machine *machine)
{
	int n_game = 0;
	eolith_speedup_address = 0;
	eolith_speedup_resume_scanline = 0;

	while( eolith_speedup_table[ n_game ].s_name != NULL )
	{
		if( strcmp( machine->gamedrv->name, eolith_speedup_table[ n_game ].s_name ) == 0 )
		{
			eolith_speedup_address = eolith_speedup_table[ n_game ].speedup_address;
			eolith_speedup_resume_scanline = eolith_speedup_table[ n_game ].speedup_resume_scanline;

		}
		n_game++;
	}
}

/* todo, use timers instead! */
INTERRUPT_GEN( eolith_speedup )
{
	eolith_scanline = 261 -  cpu_getiloops();

	if (eolith_scanline==0)
	{
		eolith_vblank = 0;
	}

	if (eolith_scanline==eolith_speedup_resume_scanline)
	{
		cpu_trigger(1000);
	}

	if (eolith_scanline==240)
	{
		eolith_vblank = 1;
	}
}

UINT32 eolith_speedup_getvblank(void *param)
{
	return eolith_vblank&1;
}
