/***************************************************************************

    Hard Drivin' sound hardware

****************************************************************************/

#include "driver.h"
#include "cpu/tms32010/tms32010.h"
#include "sound/dac.h"
#include "machine/atarigen.h"
#include "harddriv.h"


#define BIO_FREQUENCY		(1000000 / 50)
#define CYCLES_PER_BIO		(5000000 / BIO_FREQUENCY)


UINT16 *hdsnddsp_ram;


/*************************************
 *
 *  Static globals
 *
 *************************************/

static UINT8 soundflag;
static UINT8 mainflag;
static UINT16 sounddata;
static UINT16 maindata;

static UINT8 dacmute;
static UINT8 cramen;
static UINT8 irq68k;

static offs_t sound_rom_offs;

static UINT8 *rombase;
static UINT32 romsize;
static UINT16 *comram;

static UINT64 last_bio_cycles;



/*************************************
 *
 *  Driver init
 *
 *************************************/

void hdsnd_init(running_machine *machine)
{
	rombase = (UINT8 *)memory_region(machine, "serialroms");
	romsize = memory_region_length(machine, "serialroms");
	comram = (UINT16 *)auto_malloc(0x400);
	last_bio_cycles = 0;
}



/*************************************
 *
 *  Update flags
 *
 *************************************/

static void update_68k_interrupts(running_machine *machine)
{
	cpu_set_input_line(machine->cpu[hdcpu_sound], 1, mainflag ? ASSERT_LINE : CLEAR_LINE);
	cpu_set_input_line(machine->cpu[hdcpu_sound], 3, irq68k   ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  I/O from main CPU side
 *
 *************************************/

READ16_HANDLER( hd68k_snd_data_r )
{
	soundflag = 0;
	logerror("%06X:main read from sound=%04X\n", cpu_get_previouspc(machine->activecpu), sounddata);
	return sounddata;
}


READ16_HANDLER( hd68k_snd_status_r )
{
	return (mainflag << 15) | (soundflag << 14) | 0x1fff;
}


static TIMER_CALLBACK( delayed_68k_w )
{
	maindata = param;
	mainflag = 1;
	update_68k_interrupts(machine);
}


WRITE16_HANDLER( hd68k_snd_data_w )
{
	timer_call_after_resynch(NULL, data, delayed_68k_w);
	logerror("%06X:main write to sound=%04X\n", cpu_get_previouspc(machine->activecpu), data);
}


WRITE16_HANDLER( hd68k_snd_reset_w )
{
	cpu_set_input_line(machine->cpu[hdcpu_sound], INPUT_LINE_RESET, ASSERT_LINE);
	cpu_set_input_line(machine->cpu[hdcpu_sound], INPUT_LINE_RESET, CLEAR_LINE);
	mainflag = soundflag = 0;
	update_68k_interrupts(machine);
	logerror("%06X:Reset sound\n", cpu_get_previouspc(machine->activecpu));
}



/*************************************
 *
 *  I/O from sound CPU side
 *
 *************************************/

READ16_HANDLER( hdsnd68k_data_r )
{
	mainflag = 0;
	update_68k_interrupts(machine);
	logerror("%06X:sound read from main=%04X\n", cpu_get_previouspc(machine->activecpu), maindata);
	return maindata;
}


WRITE16_HANDLER( hdsnd68k_data_w )
{
	COMBINE_DATA(&sounddata);
	soundflag = 1;
	logerror("%06X:sound write to main=%04X\n", cpu_get_previouspc(machine->activecpu), data);
}



/*************************************
 *
 *  Misc. 68k inputs
 *
 *************************************/

READ16_HANDLER( hdsnd68k_switches_r )
{
	logerror("%06X:hdsnd68k_switches_r(%04X)\n", cpu_get_previouspc(machine->activecpu), offset);
	return 0;
}


READ16_HANDLER( hdsnd68k_320port_r )
{
	logerror("%06X:hdsnd68k_320port_r(%04X)\n", cpu_get_previouspc(machine->activecpu), offset);
	return 0;
}


READ16_HANDLER( hdsnd68k_status_r )
{
//FFFF 3000 R   READSTAT    Read Status
//            D15 = 'Main Flag'
//            D14 = 'Sound Flag'
//            D13 = Test Switch
//            D12 = 5220 Ready Flag (0=Ready)
	logerror("%06X:hdsnd68k_status_r(%04X)\n", cpu_get_previouspc(machine->activecpu), offset);
	return (mainflag << 15) | (soundflag << 14) | 0x2000 | 0;//((input_port_read(machine, "IN0") & 0x0020) << 8) | 0;
}



/*************************************
 *
 *  Misc. 68k outputs
 *
 *************************************/

WRITE16_HANDLER( hdsnd68k_latches_w )
{
	/* bit 3 selects the value; data is ignored */
	data = (offset >> 3) & 1;

	/* low 3 bits select the function */
	offset &= 7;
	switch (offset)
	{
		case 0:	/* SPWR - 5220 write strobe */
			/* data == 0 means high, 1 means low */
			logerror("%06X:SPWR=%d\n", cpu_get_previouspc(machine->activecpu), data);
			break;

		case 1:	/* SPRES - 5220 hard reset */
			/* data == 0 means low, 1 means high */
			logerror("%06X:SPRES=%d\n", cpu_get_previouspc(machine->activecpu), data);
			break;

		case 2:	/* SPRATE */
			/* data == 0 means 8kHz, 1 means 10kHz */
			logerror("%06X:SPRATE=%d\n", cpu_get_previouspc(machine->activecpu), data);
			break;

		case 3:	/* CRAMEN */
			/* data == 0 means disable 68k access to COM320, 1 means enable */
			cramen = data;
			break;

		case 4:	/* RES320 */
			logerror("%06X:RES320=%d\n", cpu_get_previouspc(machine->activecpu), data);
			if (hdcpu_sounddsp != -1)
				cpu_set_input_line(machine->cpu[hdcpu_sounddsp], INPUT_LINE_HALT, data ? CLEAR_LINE : ASSERT_LINE);
			break;

		case 7:	/* LED */
			break;
	}
}


WRITE16_HANDLER( hdsnd68k_speech_w )
{
	logerror("%06X:hdsnd68k_speech_w(%04X)=%04X\n", cpu_get_previouspc(machine->activecpu), offset, data);
}


WRITE16_HANDLER( hdsnd68k_irqclr_w )
{
	irq68k = 0;
	update_68k_interrupts(machine);
}



/*************************************
 *
 *  TMS32010 access
 *
 *************************************/

READ16_HANDLER( hdsnd68k_320ram_r )
{
	return hdsnddsp_ram[offset & 0xfff];
}


WRITE16_HANDLER( hdsnd68k_320ram_w )
{
	COMBINE_DATA(&hdsnddsp_ram[offset & 0xfff]);
}


READ16_HANDLER( hdsnd68k_320ports_r )
{
	UINT16 result;
	cpu_push_context(machine->cpu[hdcpu_sounddsp]);
	result = TMS32010_In(offset & 7);
	cpu_pop_context();
	return result;
}


WRITE16_HANDLER( hdsnd68k_320ports_w )
{
	cpu_push_context(machine->cpu[hdcpu_sounddsp]);
	TMS32010_Out(offset & 7, data);
	cpu_pop_context();
}


READ16_HANDLER( hdsnd68k_320com_r )
{
	if (cramen)
		return comram[offset & 0x1ff];

	logerror("%06X:hdsnd68k_320com_r(%04X) -- not allowed\n", cpu_get_previouspc(machine->activecpu), offset);
	return 0xffff;
}


WRITE16_HANDLER( hdsnd68k_320com_w )
{
	if (cramen)
		COMBINE_DATA(&comram[offset & 0x1ff]);
	else
		logerror("%06X:hdsnd68k_320com_w(%04X)=%04X -- not allowed\n", cpu_get_previouspc(machine->activecpu), offset, data);
}



/*************************************
 *
 *  TMS32010 interrupts
 *
 *************************************/

READ16_HANDLER( hdsnddsp_get_bio )
{
	UINT64 cycles_since_last_bio = cpu_get_total_cycles(machine->activecpu) - last_bio_cycles;
	INT32 cycles_until_bio = CYCLES_PER_BIO - cycles_since_last_bio;

	/* if we're not at the next BIO yet, advance us there */
	if (cycles_until_bio > 0)
	{
		cpu_adjust_icount(machine->activecpu, -cycles_until_bio);
		last_bio_cycles += CYCLES_PER_BIO;
	}
	else
		last_bio_cycles = cpu_get_total_cycles(machine->activecpu);
	return ASSERT_LINE;
}



/*************************************
 *
 *  TMS32010 ports
 *
 *************************************/

WRITE16_HANDLER( hdsnddsp_dac_w )
{
	/* DAC L */
	if (!dacmute)
		dac_signed_data_16_w(offset, data ^ 0x8000);
}


WRITE16_HANDLER( hdsnddsp_comport_w )
{
	/* COM port TD0-7 */
	logerror("%06X:hdsnddsp_comport_w=%d\n", cpu_get_previouspc(machine->activecpu), data);
}


WRITE16_HANDLER( hdsnddsp_mute_w )
{
	/* mute DAC audio, D0=1 */
/*  dacmute = data & 1;     -- NOT STUFFED */
	logerror("%06X:mute DAC=%d\n", cpu_get_previouspc(machine->activecpu), data);
}


WRITE16_HANDLER( hdsnddsp_gen68kirq_w )
{
	/* generate 68k IRQ */
	irq68k = 1;
	update_68k_interrupts(machine);
}


WRITE16_HANDLER( hdsnddsp_soundaddr_w )
{
	if (offset == 0)
	{
		/* select sound ROM block */
		sound_rom_offs = (sound_rom_offs & 0xffff) | ((data & 15) << 16);
	}
	else
	{
		/* sound memory address */
		sound_rom_offs = (sound_rom_offs & ~0xffff) | (data & 0xffff);
	}
}


READ16_HANDLER( hdsnddsp_rom_r )
{
	if (sound_rom_offs < romsize)
		return rombase[sound_rom_offs++] << 7;
	sound_rom_offs++;
	return 0;
}


READ16_HANDLER( hdsnddsp_comram_r )
{
	return comram[sound_rom_offs++ & 0x1ff];
}


READ16_HANDLER( hdsnddsp_compare_r )
{
	logerror("%06X:hdsnddsp_compare_r(%04X)\n", cpu_get_previouspc(machine->activecpu), offset);
	return 0;
}
