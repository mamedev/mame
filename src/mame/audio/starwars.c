/***************************************************************************

    Atari Star Wars hardware

    This file is Copyright Steve Baines.
    Modified by Frank Palazzolo for sound support

***************************************************************************/

#include "driver.h"
#include "cpu/m6809/m6809.h"
#include "sound/5220intf.h"
#include "includes/starwars.h"


static UINT8 sound_data;	/* data for the sound cpu */
static UINT8 main_data;		/* data for the main  cpu */

static const device_config *riot;


SOUND_START( starwars )
{
	riot = device_list_find_by_tag(machine->config->devicelist, RIOT6532, "riot");
}


/*************************************
 *
 *  RIOT interfaces
 *
 *************************************/

static UINT8 r6532_porta_r(const device_config *device, UINT8 olddata)
{
	/* Configured as follows:           */
	/* d7 (in)  Main Ready Flag         */
	/* d6 (in)  Sound Ready Flag        */
	/* d5 (out) Mute Speech             */
	/* d4 (in)  Not Sound Self Test     */
	/* d3 (out) Hold Main CPU in Reset? */
	/*          + enable delay circuit? */
	/* d2 (in)  TMS5220 Not Ready       */
	/* d1 (out) TMS5220 Not Read        */
	/* d0 (out) TMS5220 Not Write       */
	/* Note: bit 4 is always set to avoid sound self test */

	return (olddata & 0xc0) | 0x10 | (!tms5220_ready_r() << 2);
}


static void r6532_porta_w(const device_config *device, UINT8 newdata, UINT8 olddata)
{
	const address_space *space = cpu_get_address_space(device->machine->cpu[0], ADDRESS_SPACE_PROGRAM);

	/* handle 5220 read */
	if ((olddata & 2) != 0 && (newdata & 2) == 0)
		riot6532_portb_in_set(riot, tms5220_status_r(space, 0), 0xff);

	/* handle 5220 write */
	if ((olddata & 1) != 0 && (newdata & 1) == 0)
		tms5220_data_w(space, 0, riot6532_portb_out_get(riot));
}


static void snd_interrupt(const device_config *device, int state)
{
	cpu_set_input_line(device->machine->cpu[1], M6809_IRQ_LINE, state);
}


const riot6532_interface starwars_riot6532_intf =
{
	r6532_porta_r,
	NULL,
	r6532_porta_w,
	NULL,
	snd_interrupt
};




/*************************************
 *
 *  Sound CPU to/from main CPU
 *
 *************************************/

static TIMER_CALLBACK( sound_callback )
{
	riot6532_porta_in_set(riot, 0x40, 0x40);
	main_data = param;
	cpuexec_boost_interleave(machine, attotime_zero, ATTOTIME_IN_USEC(100));
}


READ8_HANDLER( starwars_sin_r )
{
	riot6532_porta_in_set(riot, 0x00, 0x80);
	return sound_data;
}


WRITE8_HANDLER( starwars_sout_w )
{
	timer_call_after_resynch(space->machine, NULL, data, sound_callback);
}



/*************************************
 *
 *  Main CPU to/from source CPU
 *
 *************************************/

READ8_HANDLER( starwars_main_read_r )
{
	riot6532_porta_in_set(riot, 0x00, 0x40);
	return main_data;
}


READ8_HANDLER( starwars_main_ready_flag_r )
{
	return riot6532_porta_in_get(riot) & 0xc0;	/* only upper two flag bits mapped */
}

static TIMER_CALLBACK( main_callback )
{
	if (riot6532_porta_in_get(riot) & 0x80)
		logerror("Sound data not read %x\n",sound_data);

	riot6532_porta_in_set(riot, 0x80, 0x80);
	sound_data = param;
	cpuexec_boost_interleave(machine, attotime_zero, ATTOTIME_IN_USEC(100));
}

WRITE8_HANDLER( starwars_main_wr_w )
{
	timer_call_after_resynch(space->machine, NULL, data, main_callback);
}


WRITE8_HANDLER( starwars_soundrst_w )
{
	riot6532_porta_in_set(riot, 0x00, 0xc0);

	/* reset sound CPU here  */
	cpu_set_input_line(space->machine->cpu[1], INPUT_LINE_RESET, PULSE_LINE);
}
