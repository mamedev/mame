/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"


UINT8 *slapfight_dpram;
size_t slapfight_dpram_size;

static int slapfight_status;
static int getstar_sequence_index;
static int getstar_sh_intenabled;

static int slapfight_status_state;
extern UINT8 *getstar_e803;

static UINT8 mcu_val;

/* Perform basic machine initialisation */
MACHINE_RESET( slapfight )
{
	/* MAIN CPU */

	slapfight_status_state=0;
	slapfight_status = 0xc7;

	getstar_sequence_index = 0;
	getstar_sh_intenabled = 0;	/* disable sound cpu interrupts */

	/* SOUND CPU */
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, ASSERT_LINE);

	/* MCU */
	mcu_val = 0;
}

/* Interrupt handlers cpu & sound */

WRITE8_HANDLER( slapfight_dpram_w )
{
    slapfight_dpram[offset]=data;
}

READ8_HANDLER( slapfight_dpram_r )
{
    return slapfight_dpram[offset];
}



/* Slapfight CPU input/output ports

  These ports seem to control memory access

*/

/* Reset and hold sound CPU */
WRITE8_HANDLER( slapfight_port_00_w )
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, ASSERT_LINE);
	getstar_sh_intenabled = 0;
}

/* Release reset on sound CPU */
WRITE8_HANDLER( slapfight_port_01_w )
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, CLEAR_LINE);
}

/* Disable and clear hardware interrupt */
WRITE8_HANDLER( slapfight_port_06_w )
{
	interrupt_enable_w(machine,0,0);
}

/* Enable hardware interrupt */
WRITE8_HANDLER( slapfight_port_07_w )
{
	interrupt_enable_w(machine,0,1);
}

WRITE8_HANDLER( slapfight_port_08_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	memory_set_bankptr(1,&RAM[0x10000]);
}

WRITE8_HANDLER( slapfight_port_09_w )
{
	UINT8 *RAM = memory_region(REGION_CPU1);

	memory_set_bankptr(1,&RAM[0x14000]);
}


/* Status register */
READ8_HANDLER( slapfight_port_00_r )
{
	static const int states[3]={ 0xc7, 0x55, 0x00 };

	slapfight_status = states[slapfight_status_state];

	slapfight_status_state++;
	if (slapfight_status_state > 2) slapfight_status_state = 0;

	return slapfight_status;
}



/*
 Reads at e803 expect a sequence of values such that:
 - first value is different from successive
 - third value is (first+5)^0x56
 I don't know what writes to this address do (connected to port 0 reads?).
*/
READ8_HANDLER( getstar_e803_r )
{
	static const UINT8 seq[] = { 0, 1, ((0+5)^0x56) };
UINT8 val;

	val = seq[getstar_sequence_index];
	getstar_sequence_index = (getstar_sequence_index+1)%3;
	return val;
}

/* Enable hardware interrupt of sound cpu */
WRITE8_HANDLER( getstar_sh_intenable_w )
{
	getstar_sh_intenabled = 1;
	logerror("cpu #1 PC=%d: %d written to a0e0\n",activecpu_get_pc(),data);
}



/* Generate interrups only if they have been enabled */
INTERRUPT_GEN( getstar_interrupt )
{
	if (getstar_sh_intenabled)
		cpunum_set_input_line(machine, 1, INPUT_LINE_NMI, PULSE_LINE);
}

#ifdef UNUSED_FUNCTION
WRITE8_HANDLER( getstar_port_04_w )
{
//  cpu_halt(0,0);
}
#endif


/* Tiger Heli MCU */

static UINT8 from_main,from_mcu;
static int mcu_sent = 0,main_sent = 0;
static UINT8 portA_in,portA_out,ddrA;
static UINT8 portB_in,portB_out,ddrB;
static UINT8 portC_in,portC_out,ddrC;

READ8_HANDLER( tigerh_68705_portA_r )
{
	return (portA_out & ddrA) | (portA_in & ~ddrA);
}

WRITE8_HANDLER( tigerh_68705_portA_w )
{
	portA_out = data;//?
	from_mcu = portA_out;
	mcu_sent = 1;
}

WRITE8_HANDLER( tigerh_68705_ddrA_w )
{
	ddrA = data;
}

READ8_HANDLER( tigerh_68705_portB_r )
{
	return (portB_out & ddrB) | (portB_in & ~ddrB);
}

WRITE8_HANDLER( tigerh_68705_portB_w )
{

	if ((ddrB & 0x02) && (~data & 0x02) && (portB_out & 0x02))
	{
		portA_in = from_main;
		if (main_sent) cpunum_set_input_line(machine, 2,0,CLEAR_LINE);
		main_sent = 0;
	}
	if ((ddrB & 0x04) && (data & 0x04) && (~portB_out & 0x04))
	{
		from_mcu = portA_out;
		mcu_sent = 1;
	}

	portB_out = data;
}

WRITE8_HANDLER( tigerh_68705_ddrB_w )
{
	ddrB = data;
}


READ8_HANDLER( tigerh_68705_portC_r )
{
	portC_in = 0;
	if (!main_sent) portC_in |= 0x01;
	if (mcu_sent) portC_in |= 0x02;
	return (portC_out & ddrC) | (portC_in & ~ddrC);
}

WRITE8_HANDLER( tigerh_68705_portC_w )
{
	portC_out = data;
}

WRITE8_HANDLER( tigerh_68705_ddrC_w )
{
	ddrC = data;
}

WRITE8_HANDLER( tigerh_mcu_w )
{
	from_main = data;
	main_sent = 1;
	mcu_sent=0;
	cpunum_set_input_line(machine, 2,0,ASSERT_LINE);
}

READ8_HANDLER( tigerh_mcu_r )
{
	mcu_sent = 0;
	return from_mcu;
}

READ8_HANDLER( tigerh_mcu_status_r )
{
	int res = 0;
	if (!main_sent) res |= 0x02;
	if (!mcu_sent) res |= 0x04;
	return res;
}

