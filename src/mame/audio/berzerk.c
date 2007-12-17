/*
 * Berzerk/Frenzy Soundhardware Driver
 * Copyright Alex Judd 1997/98
 * V1.1 for Mame 0.31 13March98
 *
 */

#include "driver.h"
#include "berzerk.h"
#include "exidy.h"
#include "sound/s14001a.h"



#define BERZERK_AUDIO_DEBUG		(0)


static struct S14001A_interface berzerk_s14001a_interface =
{
	REGION_SOUND1	/* voice data region */
};


static struct CustomSound_interface custom_interface =
{
	berzerk_sh_start
};


MACHINE_DRIVER_START( berzerk_audio )
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD(S14001A, BERZERK_S14001A_CLOCK)	/* CPU clock divided by 16 divided by a programmable TTL setup */
	MDRV_SOUND_CONFIG(berzerk_s14001a_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD(CUSTOM, 0)
	MDRV_SOUND_CONFIG(custom_interface)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END



#if BERZERK_AUDIO_DEBUG
static void berzerk_audio_debug(UINT8 data)
{
	mame_printf_debug("not busy, triggering S14001A core with %x\n", data);
	mame_printf_debug("S14001a word play command: ");

	switch (data)
	{
		case 0: mame_printf_debug("help\n"); break;
		case 1: mame_printf_debug("kill\n"); break;
		case 2: mame_printf_debug("attack\n"); break;
		case 3: mame_printf_debug("charge\n"); break;
		case 4: mame_printf_debug("got\n"); break;
		case 5: mame_printf_debug("shoot\n"); break;
		case 6: mame_printf_debug("get\n"); break;
		case 7: mame_printf_debug("is\n"); break;
		case 8: mame_printf_debug("alert\n"); break;
		case 9: mame_printf_debug("detected\n"); break;
		case 10: mame_printf_debug("the\n"); break;
		case 11: mame_printf_debug("in\n"); break;
		case 12: mame_printf_debug("it\n"); break;
		case 13: mame_printf_debug("there\n"); break;
		case 14: mame_printf_debug("where\n"); break;
		case 15: mame_printf_debug("humanoid\n"); break;
		case 16: mame_printf_debug("coins\n"); break;
		case 17: mame_printf_debug("pocket\n"); break;
		case 18: mame_printf_debug("intruder\n"); break;
		case 19: mame_printf_debug("no\n"); break;
		case 20: mame_printf_debug("escape\n"); break;
		case 21: mame_printf_debug("destroy\n"); break;
		case 22: mame_printf_debug("must\n"); break;
		case 23: mame_printf_debug("not\n"); break;
		case 24: mame_printf_debug("chicken\n"); break;
		case 25: mame_printf_debug("fight\n"); break;
		case 26: mame_printf_debug("like\n"); break;
		case 27: mame_printf_debug("a\n"); break;
		case 28: mame_printf_debug("robot\n"); break;
		default: mame_printf_debug("ERROR: data %2x; you should NOT see this!\n", data); break;
	}
}
#endif


WRITE8_HANDLER( berzerk_audio_w )
{
	switch (offset)
	{
	/* offsets 0-3, 5 and 7 write to the 6840 */
	case 0:
	case 1:
	case 2:
	case 3:
	case 5:
	case 7:
		exidy_sh6840_w(offset, data);
		break;

	/* offset 6 writes to the sfxcontrol latch */
	case 6:
		exidy_sfxctrl_w(data >> 6, data);
		break;

	/* offset 4 writes to the S14001A */
	case 4:
		if ((data & 0xc0) == 0x40) /* VSU-1000 control write */
		{
			/* volume and frequency control goes here */
			/* mame_printf_debug("TODO: VSU-1000 Control write (ignored for now)\n");*/
		  S14001A_set_volume(((data&0x38)>>3)+1);
		  S14001A_set_rate((16-(data&0x07))*16); /* second LS161 has load triggered by its own TC(when it equals 16) long before the first ls161 will TC and fire again, so effectively it only divides by 15 and not 16. If the clock, as opposed to the E enable, had been tied to the first LS161's TC instead, it would divide by 16 as expected */
		}
		else if ((data & 0xc0) != 0x00)
		{
			/* vsu-1000 ignores these writes entirely */
			mame_printf_debug("bogus write ignored\n");
		}
		else
		{
			/* select word input */
			if (S14001A_bsy_0_r()) /* skip if busy... */
			{
				mame_printf_debug("S14001A busy, ignoring write\n");
				break;
			}

			/* write to the register */
			S14001A_reg_0_w(data & 0x3f);
			S14001A_rst_0_w(1);
			S14001A_rst_0_w(0);

#if BERZERK_AUDIO_DEBUG
			berzerk_audio_debug(data);
#endif
		}
		break;
	}
}


READ8_HANDLER( berzerk_audio_r )
{
	return ((offset == 4) && !S14001A_bsy_0_r()) ? 0x40 : 0x00;
}
