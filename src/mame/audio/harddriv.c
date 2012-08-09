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
	state->m_rombase = (UINT8 *)state->memregion("serialroms")->base();
	state->m_romsize = state->memregion("serialroms")->bytes();
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

READ16_MEMBER(harddriv_state::hd68k_snd_data_r)
{
	m_soundflag = 0;
	logerror("%06X:main read from sound=%04X\n", cpu_get_previouspc(&space.device()), m_sounddata);
	return m_sounddata;
}


READ16_MEMBER(harddriv_state::hd68k_snd_status_r)
{
	return (m_mainflag << 15) | (m_soundflag << 14) | 0x1fff;
}


static TIMER_CALLBACK( delayed_68k_w )
{
	harddriv_state *state = machine.driver_data<harddriv_state>();
	state->m_maindata = param;
	state->m_mainflag = 1;
	update_68k_interrupts(machine);
}


WRITE16_MEMBER(harddriv_state::hd68k_snd_data_w)
{
	machine().scheduler().synchronize(FUNC(delayed_68k_w), data);
	logerror("%06X:main write to sound=%04X\n", cpu_get_previouspc(&space.device()), data);
}


WRITE16_MEMBER(harddriv_state::hd68k_snd_reset_w)
{
	device_set_input_line(m_soundcpu, INPUT_LINE_RESET, ASSERT_LINE);
	device_set_input_line(m_soundcpu, INPUT_LINE_RESET, CLEAR_LINE);
	m_mainflag = m_soundflag = 0;
	update_68k_interrupts(machine());
	logerror("%06X:Reset sound\n", cpu_get_previouspc(&space.device()));
}



/*************************************
 *
 *  I/O from sound CPU side
 *
 *************************************/

READ16_MEMBER(harddriv_state::hdsnd68k_data_r)
{
	m_mainflag = 0;
	update_68k_interrupts(machine());
	logerror("%06X:sound read from main=%04X\n", cpu_get_previouspc(&space.device()), m_maindata);
	return m_maindata;
}


WRITE16_MEMBER(harddriv_state::hdsnd68k_data_w)
{
	COMBINE_DATA(&m_sounddata);
	m_soundflag = 1;
	logerror("%06X:sound write to main=%04X\n", cpu_get_previouspc(&space.device()), data);
}



/*************************************
 *
 *  Misc. 68k inputs
 *
 *************************************/

READ16_MEMBER(harddriv_state::hdsnd68k_switches_r)
{
	logerror("%06X:hdsnd68k_switches_r(%04X)\n", cpu_get_previouspc(&space.device()), offset);
	return 0;
}


READ16_MEMBER(harddriv_state::hdsnd68k_320port_r)
{
	logerror("%06X:hdsnd68k_320port_r(%04X)\n", cpu_get_previouspc(&space.device()), offset);
	return 0;
}


READ16_MEMBER(harddriv_state::hdsnd68k_status_r)
{
//FFFF 3000 R   READSTAT    Read Status
//            D15 = 'Main Flag'
//            D14 = 'Sound Flag'
//            D13 = Test Switch
//            D12 = 5220 Ready Flag (0=Ready)
	logerror("%06X:hdsnd68k_status_r(%04X)\n", cpu_get_previouspc(&space.device()), offset);
	return (m_mainflag << 15) | (m_soundflag << 14) | 0x2000 | 0;//((ioport("IN0")->read() & 0x0020) << 8) | 0;
}



/*************************************
 *
 *  Misc. 68k outputs
 *
 *************************************/

WRITE16_MEMBER(harddriv_state::hdsnd68k_latches_w)
{

	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0:	/* SPWR - 5220 write strobe */
			/* data == 0 means high, 1 means low */
			logerror("%06X:SPWR=%d\n", cpu_get_previouspc(&space.device()), data);
			break;

		case 1:	/* SPRES - 5220 hard reset */
			/* data == 0 means low, 1 means high */
			logerror("%06X:SPRES=%d\n", cpu_get_previouspc(&space.device()), data);
			break;

		case 2:	/* SPRATE */
			/* data == 0 means 8kHz, 1 means 10kHz */
			logerror("%06X:SPRATE=%d\n", cpu_get_previouspc(&space.device()), data);
			break;

		case 3:	/* CRAMEN */
			/* data == 0 means disable 68k access to COM320, 1 means enable */
			m_cramen = data;
			break;

		case 4:	/* RES320 */
			logerror("%06X:RES320=%d\n", cpu_get_previouspc(&space.device()), data);
			if (m_sounddsp != NULL)
				device_set_input_line(m_sounddsp, INPUT_LINE_HALT, data ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 7:	/* LED */
			break;
	}
}


WRITE16_MEMBER(harddriv_state::hdsnd68k_speech_w)
{
	logerror("%06X:hdsnd68k_speech_w(%04X)=%04X\n", cpu_get_previouspc(&space.device()), offset, data);
}


WRITE16_MEMBER(harddriv_state::hdsnd68k_irqclr_w)
{
	m_irq68k = 0;
	update_68k_interrupts(machine());
}



/*************************************
 *
 *  TMS32010 access
 *
 *************************************/

READ16_MEMBER(harddriv_state::hdsnd68k_320ram_r)
{
	return m_sounddsp_ram[offset & 0xfff];
}


WRITE16_MEMBER(harddriv_state::hdsnd68k_320ram_w)
{
	COMBINE_DATA(&m_sounddsp_ram[offset & 0xfff]);
}


READ16_MEMBER(harddriv_state::hdsnd68k_320ports_r)
{
	address_space *iospace = m_sounddsp->memory().space(AS_IO);
	return iospace->read_word((offset & 7) << 1);
}


WRITE16_MEMBER(harddriv_state::hdsnd68k_320ports_w)
{
	address_space *iospace = m_sounddsp->memory().space(AS_IO);
	iospace->write_word((offset & 7) << 1, data);
}


READ16_MEMBER(harddriv_state::hdsnd68k_320com_r)
{

	if (m_cramen)
		return m_comram[offset & 0x1ff];

	logerror("%06X:hdsnd68k_320com_r(%04X) -- not allowed\n", cpu_get_previouspc(&space.device()), offset);
	return 0xffff;
}


WRITE16_MEMBER(harddriv_state::hdsnd68k_320com_w)
{

	if (m_cramen)
		COMBINE_DATA(&m_comram[offset & 0x1ff]);
	else
		logerror("%06X:hdsnd68k_320com_w(%04X)=%04X -- not allowed\n", cpu_get_previouspc(&space.device()), offset, data);
}



/*************************************
 *
 *  TMS32010 interrupts
 *
 *************************************/

READ16_MEMBER(harddriv_state::hdsnddsp_get_bio)
{
	UINT64 cycles_since_last_bio = m_sounddsp->total_cycles() - m_last_bio_cycles;
	INT32 cycles_until_bio = CYCLES_PER_BIO - cycles_since_last_bio;

	/* if we're not at the next BIO yet, advance us there */
	if (cycles_until_bio > 0)
	{
		device_adjust_icount(&space.device(), -cycles_until_bio);
		m_last_bio_cycles += CYCLES_PER_BIO;
	}
	else
		m_last_bio_cycles = m_sounddsp->total_cycles();
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
		downcast<dac_device *>(device)->write_signed16(data ^ 0x8000);
}


WRITE16_MEMBER(harddriv_state::hdsnddsp_comport_w)
{
	/* COM port TD0-7 */
	logerror("%06X:hdsnddsp_comport_w=%d\n", cpu_get_previouspc(&space.device()), data);
}


WRITE16_MEMBER(harddriv_state::hdsnddsp_mute_w)
{
	/* mute DAC audio, D0=1 */
/*  m_dacmute = data & 1;     -- NOT STUFFED */
	logerror("%06X:mute DAC=%d\n", cpu_get_previouspc(&space.device()), data);
}


WRITE16_MEMBER(harddriv_state::hdsnddsp_gen68kirq_w)
{
	/* generate 68k IRQ */
	m_irq68k = 1;
	update_68k_interrupts(machine());
}


WRITE16_MEMBER(harddriv_state::hdsnddsp_soundaddr_w)
{
	if (offset == 0)
	{
		/* select sound ROM block */
		m_sound_rom_offs = (m_sound_rom_offs & 0xffff) | ((data & 15) << 16);
	}
	else
	{
		/* sound memory address */
		m_sound_rom_offs = (m_sound_rom_offs & ~0xffff) | (data & 0xffff);
	}
}


READ16_MEMBER(harddriv_state::hdsnddsp_rom_r)
{
	if (m_sound_rom_offs < m_romsize)
		return m_rombase[m_sound_rom_offs++] << 7;
	m_sound_rom_offs++;
	return 0;
}


READ16_MEMBER(harddriv_state::hdsnddsp_comram_r)
{
	return m_comram[m_sound_rom_offs++ & 0x1ff];
}


READ16_MEMBER(harddriv_state::hdsnddsp_compare_r)
{
	logerror("%06X:hdsnddsp_compare_r(%04X)\n", cpu_get_previouspc(&space.device()), offset);
	return 0;
}
