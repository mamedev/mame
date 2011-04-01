/***************************************************************************

    Hard Drivin' sound hardware

****************************************************************************/

#include "emu.h"
#include "cpu/tms32010/tms32010.h"
#include "sound/dac.h"
#include "machine/atarigen.h"
#include "includes/harddriv.h"


#define BIO_FREQUENCY		(1000000 / 50)
#define CYCLES_PER_BIO		(5000000 / BIO_FREQUENCY)


/*************************************
 *
 *  Driver init
 *
 *************************************/

void hdsnd_init(running_machine &machine)
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	state->m_rombase = (UINT8 *)machine.region("serialroms")->base();
	state->m_romsize = machine.region("serialroms")->bytes();
}



/*************************************
 *
 *  Update flags
 *
 *************************************/

static void update_68k_interrupts(running_machine &machine)
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	device_set_input_line(state->m_soundcpu, 1, state->m_mainflag ? ASSERT_LINE : CLEAR_LINE);
	device_set_input_line(state->m_soundcpu, 3, state->m_irq68k   ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  I/O from main CPU side
 *
 *************************************/

READ16_HANDLER( hd68k_snd_data_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	state->m_soundflag = 0;
	logerror("%06X:main read from sound=%04X\n", cpu_get_previouspc(&space->device()), state->m_sounddata);
	return state->m_sounddata;
}


READ16_HANDLER( hd68k_snd_status_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return (state->m_mainflag << 15) | (state->m_soundflag << 14) | 0x1fff;
}


static TIMER_CALLBACK( delayed_68k_w )
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	state->m_maindata = param;
	state->m_mainflag = 1;
	update_68k_interrupts(machine);
}


WRITE16_HANDLER( hd68k_snd_data_w )
{
	space->machine().scheduler().synchronize(FUNC(delayed_68k_w), data);
	logerror("%06X:main write to sound=%04X\n", cpu_get_previouspc(&space->device()), data);
}


WRITE16_HANDLER( hd68k_snd_reset_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	device_set_input_line(state->m_soundcpu, INPUT_LINE_RESET, ASSERT_LINE);
	device_set_input_line(state->m_soundcpu, INPUT_LINE_RESET, CLEAR_LINE);
	state->m_mainflag = state->m_soundflag = 0;
	update_68k_interrupts(space->machine());
	logerror("%06X:Reset sound\n", cpu_get_previouspc(&space->device()));
}



/*************************************
 *
 *  I/O from sound CPU side
 *
 *************************************/

READ16_HANDLER( hdsnd68k_data_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	state->m_mainflag = 0;
	update_68k_interrupts(space->machine());
	logerror("%06X:sound read from main=%04X\n", cpu_get_previouspc(&space->device()), state->m_maindata);
	return state->m_maindata;
}


WRITE16_HANDLER( hdsnd68k_data_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	COMBINE_DATA(&state->m_sounddata);
	state->m_soundflag = 1;
	logerror("%06X:sound write to main=%04X\n", cpu_get_previouspc(&space->device()), data);
}



/*************************************
 *
 *  Misc. 68k inputs
 *
 *************************************/

READ16_HANDLER( hdsnd68k_switches_r )
{
	logerror("%06X:hdsnd68k_switches_r(%04X)\n", cpu_get_previouspc(&space->device()), offset);
	return 0;
}


READ16_HANDLER( hdsnd68k_320port_r )
{
	logerror("%06X:hdsnd68k_320port_r(%04X)\n", cpu_get_previouspc(&space->device()), offset);
	return 0;
}


READ16_HANDLER( hdsnd68k_status_r )
{
//FFFF 3000 R   READSTAT    Read Status
//            D15 = 'Main Flag'
//            D14 = 'Sound Flag'
//            D13 = Test Switch
//            D12 = 5220 Ready Flag (0=Ready)
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	logerror("%06X:hdsnd68k_status_r(%04X)\n", cpu_get_previouspc(&space->device()), offset);
	return (state->m_mainflag << 15) | (state->m_soundflag << 14) | 0x2000 | 0;//((input_port_read(space->machine(), "IN0") & 0x0020) << 8) | 0;
}



/*************************************
 *
 *  Misc. 68k outputs
 *
 *************************************/

WRITE16_HANDLER( hdsnd68k_latches_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0:	/* SPWR - 5220 write strobe */
			/* data == 0 means high, 1 means low */
			logerror("%06X:SPWR=%d\n", cpu_get_previouspc(&space->device()), data);
			break;

		case 1:	/* SPRES - 5220 hard reset */
			/* data == 0 means low, 1 means high */
			logerror("%06X:SPRES=%d\n", cpu_get_previouspc(&space->device()), data);
			break;

		case 2:	/* SPRATE */
			/* data == 0 means 8kHz, 1 means 10kHz */
			logerror("%06X:SPRATE=%d\n", cpu_get_previouspc(&space->device()), data);
			break;

		case 3:	/* CRAMEN */
			/* data == 0 means disable 68k access to COM320, 1 means enable */
			state->m_cramen = data;
			break;

		case 4:	/* RES320 */
			logerror("%06X:RES320=%d\n", cpu_get_previouspc(&space->device()), data);
			if (state->m_sounddsp != NULL)
				device_set_input_line(state->m_sounddsp, INPUT_LINE_HALT, data ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 7:	/* LED */
			break;
	}
}


WRITE16_HANDLER( hdsnd68k_speech_w )
{
	logerror("%06X:hdsnd68k_speech_w(%04X)=%04X\n", cpu_get_previouspc(&space->device()), offset, data);
}


WRITE16_HANDLER( hdsnd68k_irqclr_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	state->m_irq68k = 0;
	update_68k_interrupts(space->machine());
}



/*************************************
 *
 *  TMS32010 access
 *
 *************************************/

READ16_HANDLER( hdsnd68k_320ram_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_sounddsp_ram[offset & 0xfff];
}


WRITE16_HANDLER( hdsnd68k_320ram_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	COMBINE_DATA(&state->m_sounddsp_ram[offset & 0xfff]);
}


READ16_HANDLER( hdsnd68k_320ports_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	address_space *iospace = state->m_sounddsp->memory().space(AS_IO);
	return iospace->read_word((offset & 7) << 1);
}


WRITE16_HANDLER( hdsnd68k_320ports_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	address_space *iospace = state->m_sounddsp->memory().space(AS_IO);
	iospace->write_word((offset & 7) << 1, data);
}


READ16_HANDLER( hdsnd68k_320com_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	if (state->m_cramen)
		return state->m_comram[offset & 0x1ff];

	logerror("%06X:hdsnd68k_320com_r(%04X) -- not allowed\n", cpu_get_previouspc(&space->device()), offset);
	return 0xffff;
}


WRITE16_HANDLER( hdsnd68k_320com_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();

	if (state->m_cramen)
		COMBINE_DATA(&state->m_comram[offset & 0x1ff]);
	else
		logerror("%06X:hdsnd68k_320com_w(%04X)=%04X -- not allowed\n", cpu_get_previouspc(&space->device()), offset, data);
}



/*************************************
 *
 *  TMS32010 interrupts
 *
 *************************************/

READ16_HANDLER( hdsnddsp_get_bio )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	UINT64 cycles_since_last_bio = state->m_sounddsp->total_cycles() - state->m_last_bio_cycles;
	INT32 cycles_until_bio = CYCLES_PER_BIO - cycles_since_last_bio;

	/* if we're not at the next BIO yet, advance us there */
	if (cycles_until_bio > 0)
	{
		device_adjust_icount(&space->device(), -cycles_until_bio);
		state->m_last_bio_cycles += CYCLES_PER_BIO;
	}
	else
		state->m_last_bio_cycles = state->m_sounddsp->total_cycles();
	return ASSERT_LINE;
}



/*************************************
 *
 *  TMS32010 ports
 *
 *************************************/

WRITE16_DEVICE_HANDLER( hdsnddsp_dac_w )
{
	harddriv_state *state = device->machine().driver_data<harddriv_state>();

	/* DAC L */
	if (!state->m_dacmute)
		dac_signed_data_16_w(device, data ^ 0x8000);
}


WRITE16_HANDLER( hdsnddsp_comport_w )
{
	/* COM port TD0-7 */
	logerror("%06X:hdsnddsp_comport_w=%d\n", cpu_get_previouspc(&space->device()), data);
}


WRITE16_HANDLER( hdsnddsp_mute_w )
{
	/* mute DAC audio, D0=1 */
/*  state->m_dacmute = data & 1;     -- NOT STUFFED */
	logerror("%06X:mute DAC=%d\n", cpu_get_previouspc(&space->device()), data);
}


WRITE16_HANDLER( hdsnddsp_gen68kirq_w )
{
	/* generate 68k IRQ */
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	state->m_irq68k = 1;
	update_68k_interrupts(space->machine());
}


WRITE16_HANDLER( hdsnddsp_soundaddr_w )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	if (offset == 0)
	{
		/* select sound ROM block */
		state->m_sound_rom_offs = (state->m_sound_rom_offs & 0xffff) | ((data & 15) << 16);
	}
	else
	{
		/* sound memory address */
		state->m_sound_rom_offs = (state->m_sound_rom_offs & ~0xffff) | (data & 0xffff);
	}
}


READ16_HANDLER( hdsnddsp_rom_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	if (state->m_sound_rom_offs < state->m_romsize)
		return state->m_rombase[state->m_sound_rom_offs++] << 7;
	state->m_sound_rom_offs++;
	return 0;
}


READ16_HANDLER( hdsnddsp_comram_r )
{
	harddriv_state *state = space->machine().driver_data<harddriv_state>();
	return state->m_comram[state->m_sound_rom_offs++ & 0x1ff];
}


READ16_HANDLER( hdsnddsp_compare_r )
{
	logerror("%06X:hdsnddsp_compare_r(%04X)\n", cpu_get_previouspc(&space->device()), offset);
	return 0;
}
