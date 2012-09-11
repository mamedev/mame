/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "emu.h"
#include "sound/samples.h"
#include "includes/galaga.h"

/***************************************************************************

 BATTLES CPU4(custum I/O Emulation) I/O Handlers

***************************************************************************/

static UINT8 customio[16];
static char battles_customio_command;
static char battles_customio_prev_command;
static char battles_customio_command_count;
static char battles_customio_data;
static char battles_sound_played;


void battles_customio_init(running_machine &machine)
{
	battles_customio_command = 0;
	battles_customio_prev_command = 0;
	battles_customio_command_count = 0;
	battles_customio_data = 0;
	battles_sound_played = 0;
}


TIMER_DEVICE_CALLBACK( battles_nmi_generate )
{

	battles_customio_prev_command = battles_customio_command;

	if( battles_customio_command & 0x10 )
	{
		if( battles_customio_command_count == 0 )
		{
			cputag_set_input_line(timer.machine(), "sub3", INPUT_LINE_NMI, PULSE_LINE);
		}
		else
		{
			cputag_set_input_line(timer.machine(), "maincpu", INPUT_LINE_NMI, PULSE_LINE);
			cputag_set_input_line(timer.machine(), "sub3", INPUT_LINE_NMI, PULSE_LINE);
		}
	}
	else
	{
		cputag_set_input_line(timer.machine(), "maincpu", INPUT_LINE_NMI, PULSE_LINE);
		cputag_set_input_line(timer.machine(), "sub3", INPUT_LINE_NMI, PULSE_LINE);
	}
	battles_customio_command_count++;
}


READ8_HANDLER( battles_customio0_r )
{
	logerror("CPU0 %04x: custom I/O Read = %02x\n",space->device().safe_pc(),battles_customio_command);
	return battles_customio_command;
}

READ8_HANDLER( battles_customio3_r )
{
	int	return_data;

	if( space->device().safe_pc() == 0xAE ){
		/* CPU4 0xAA - 0xB9 : waiting for MB8851 ? */
		return_data =	( (battles_customio_command & 0x10) << 3)
						| 0x00
						| (battles_customio_command & 0x0f);
	}else{
		return_data =	( (battles_customio_prev_command & 0x10) << 3)
						| 0x60
						| (battles_customio_prev_command & 0x0f);
	}
	logerror("CPU3 %04x: custom I/O Read = %02x\n",space->device().safe_pc(),return_data);

	return return_data;
}


WRITE8_HANDLER( battles_customio0_w )
{
	timer_device *timer = space->machine().device<timer_device>("battles_nmi");

	logerror("CPU0 %04x: custom I/O Write = %02x\n",space->device().safe_pc(),data);

	battles_customio_command = data;
	battles_customio_command_count = 0;

	switch (data)
	{
		case 0x10:
			timer->reset();
			return; /* nop */
	}
	timer->adjust(attotime::from_usec(166), 0, attotime::from_usec(166));

}

WRITE8_HANDLER( battles_customio3_w )
{
	logerror("CPU3 %04x: custom I/O Write = %02x\n",space->device().safe_pc(),data);

	battles_customio_command = data;
}



READ8_HANDLER( battles_customio_data0_r )
{
	logerror("CPU0 %04x: custom I/O parameter %02x Read = %02x\n",space->device().safe_pc(),offset,battles_customio_data);

	return battles_customio_data;
}

READ8_HANDLER( battles_customio_data3_r )
{
	logerror("CPU3 %04x: custom I/O parameter %02x Read = %02x\n",space->device().safe_pc(),offset,battles_customio_data);
	return battles_customio_data;
}


WRITE8_HANDLER( battles_customio_data0_w )
{
	logerror("CPU0 %04x: custom I/O parameter %02x Write = %02x\n",space->device().safe_pc(),offset,data);
	battles_customio_data = data;
}

WRITE8_HANDLER( battles_customio_data3_w )
{
	logerror("CPU3 %04x: custom I/O parameter %02x Write = %02x\n",space->device().safe_pc(),offset,data);
	battles_customio_data = data;
}


WRITE8_HANDLER( battles_CPU4_coin_w )
{
	set_led_status(space->machine(), 0,data & 0x02);	// Start 1
	set_led_status(space->machine(), 1,data & 0x01);	// Start 2

	coin_counter_w(space->machine(), 0,data & 0x20);
	coin_counter_w(space->machine(), 1,data & 0x10);
	coin_lockout_global_w(space->machine(), ~data & 0x04);
}


WRITE8_HANDLER( battles_noise_sound_w )
{
	logerror("CPU3 %04x: 50%02x Write = %02x\n",space->device().safe_pc(),offset,data);
	if( (battles_sound_played == 0) && (data == 0xFF) ){
		samples_device *samples = space->machine().device<samples_device>("samples");
		if( customio[0] == 0x40 ){
			samples->start(0, 0);
		}
		else{
			samples->start(0, 1);
		}
	}
	battles_sound_played = data;
}


READ8_HANDLER( battles_input_port_r )
{
	switch ( offset )
	{
		default:
		case 0: return ~BITSWAP8(space->machine().root_device().ioport("IN0H")->read(),7,6,5,4,2,3,1,0);
		case 1: return ~space->machine().root_device().ioport("IN1L")->read();
		case 2: return ~space->machine().root_device().ioport("IN1H")->read();
		case 3: return ~space->machine().root_device().ioport("IN0L")->read();
	}
}


INTERRUPT_GEN( battles_interrupt_4 )
{
	device_set_input_line(device, 0, HOLD_LINE);
}

