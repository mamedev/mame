/*************************************************************************

	re900.c

	Ruleta RE-900 - Entretenimientos GEMINI (1990)

	Driver by Grull Osgo.

	Games running on this hardware:

	* Ruleta RE-900,    1990, Entretenimientos GEMINI.
	* Buena Suerte '94  1990, Entretenimientos GEMINI.


    Preliminary Version:

	Video: OK.
	Sound: OK.
	Inputs: Incomplete.
	Outputs: Incomplete.
	Layout: Incomplete.

***************************************************************************/

#define MASTER_CLOCK	11059200
#define VDP_CLOCK		10730000
#define TMS_CLOCK		VDP_CLOCK / 24

#include "driver.h"
#include "cpu/i8051/i8051.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
//#include "re900.lh"

static UINT8 *re900_rom;


/**********************
* Read/Write Handlers *
**********************/

static READ8_HANDLER (in_port_0_r)
{
	if ((input_port_read(machine, "IN0") & 0x01) == 0) output_set_lamp_value(1,1); else output_set_lamp_value(1,0);
	if ((input_port_read(machine, "IN0") & 0x02) == 0) output_set_lamp_value(2,1); else output_set_lamp_value(2,0);
	return input_port_read(machine, "IN0");
}

static READ8_HANDLER (in_port_1_r)
{
	if ((input_port_read(machine, "IN1") & 0x01) == 0) output_set_lamp_value(3,1); else output_set_lamp_value(3,0);
	if ((input_port_read(machine, "IN1") & 0x02) == 0) output_set_lamp_value(4,1); else output_set_lamp_value(4,0);
	if ((input_port_read(machine, "IN1") & 0x04) == 0) output_set_lamp_value(5,1); else output_set_lamp_value(5,0);
	if ((input_port_read(machine, "IN1") & 0x08) == 0) output_set_lamp_value(6,1); else output_set_lamp_value(6,0);
	if ((input_port_read(machine, "IN1") & 0x10) == 0) output_set_lamp_value(7,1); else output_set_lamp_value(7,0);
	return input_port_read(machine, "IN1") | 0x80;
}

static WRITE8_HANDLER (output_port_3_w)
{
	
	output_set_lamp_value(1,1 ^ ( (data >> 4) & 1)); /* Cont. Sal */
	output_set_lamp_value(2,1 ^ ( (data >> 5) & 1)); /* Cont. Ent */
	//output_set_lamp_value(20,1);

}

static UINT8 mux_data = 0, ledant = 0;
static UINT8 psg_pb = 0;
static UINT8 psg_pa;


static WRITE8_HANDLER (mux_port_A_w)
{
	psg_pa = data;
	mux_data = ((data >> 2) & 0x3f) ^ 0x3f;
}
		
static READ8_HANDLER (mux_port_B_r)
{
	UINT8 retval = 0xff;

	switch( mux_data )
	{
		case 0x20: retval = in_port_1_r(machine, 1); break;			// Player 1
		case 0x10: retval = input_port_read(machine, "IN2"); break;	// Player 2
		case 0x01: retval = input_port_read(machine, "IN3"); break;	// Player 6
	}

	return retval;
}

static WRITE8_HANDLER (mux_port_B_w)
{
	UINT8 led;
	psg_pb = data;
	//stb = (data >> 7) & 1;
	led = (psg_pa >> 2) & 0x3f;

	if (data == 0x7f)
	{
		logerror("LED Ruleta %x \n", led);
		output_set_lamp_value(20 + led,1);
		
		if (led != ledant) 
		{
			output_set_lamp_value(20 + ledant,0);
			ledant = led;
		}

	}

	//popmessage("PSG_B=%x", psg_pb);
	//output_set_lamp_value(0, 1 ^ ((data >> 7) & 1)); /* Strobe Led's Ruleta */
}

static READ8_HANDLER (rom_r)
{
	return re900_rom[offset];
}

/*
External RAM Callback for I8052
READ32_HANDLER(re900_external_ram_iaddr)
{
	UINT8 p2 = i8051_get_intram(P2);
	if (offset < 0x100) 
	{
		return ((p2 << 8) | offset);
	}
	else
		return offset;
}
*/

static WRITE8_HANDLER(re900_watchdog_reset_w)
{
	//watchdog_reset_w(machine,0,0); /* To do ! */
}


/*****************************
*   Memory Map Information   *
*****************************/

static ADDRESS_MAP_START( mem_prg, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xffff) AM_ROM AM_BASE(&re900_rom)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mem_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_READ (rom_r) 
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0xe000, 0xe000) AM_WRITE(TMS9928A_vram_w)
	AM_RANGE(0xe001, 0xe001) AM_WRITE(TMS9928A_register_w)
	AM_RANGE(0xe800, 0xe800) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0xe801, 0xe801) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0xe802, 0xe802) AM_READ(ay8910_read_port_0_r)
	AM_RANGE(0xe000, 0xefff) AM_WRITE(re900_watchdog_reset_w) 
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_WRITE(output_port_3_w)
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_RAM
ADDRESS_MAP_END
/*
static ADDRESS_MAP_START( mem_io, ADDRESS_SPACE_IO, 8 )
	AM_RANGE(0x00, 0x00) AM_WRITE(output_port_3_w)
	AM_RANGE(0x02, 0x02) AM_RAM
ADDRESS_MAP_END
*/

static INTERRUPT_GEN( re900_video_interrupt )
{
	TMS9928A_interrupt(machine);
}

static void vdp_interrupt (running_machine *machine, int state)
{
	cpunum_set_input_line(machine, 0, INPUT_LINE_NMI, state ? ASSERT_LINE : CLEAR_LINE );
}


/************************
*      Input ports      *
************************/

INPUT_PORTS_START( re900 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Operator Key") PORT_TOGGLE PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Auditor Key")  PORT_TOGGLE PORT_CODE(KEYCODE_9)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1-Apuesta")   PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1-Izquierda") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1-Derecha")   PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1-Arriba")    PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P1-Abajo")     PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P1-Carga")     PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P1-Descarga")  PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) 

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P2-Apuesta")   PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P2-Izquierda") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P2-Derecha")   PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P2-Arriba")    PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P2-Abajo")     PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P2-Carga")     PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P2-Descarga")  PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) 

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P3-Apuesta")   PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P3-Izquierda") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P3-Derecha")   PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P3-Arriba")    PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("P3-Abajo")     PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("P3-Carga")     PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("P3-Descarga")  PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) 
INPUT_PORTS_END


/**********************
*  TMS9928a Interfase *
**********************/

static const TMS9928a_interface tms9928a_interface =
{
	TMS99x8A,		/* TMS9128NL on pcb */
	0x4000,
	0,0,
	vdp_interrupt
};


/*********************
*  AY8910 Interfase  *
*********************/

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	in_port_0_r, mux_port_B_r, mux_port_A_w, mux_port_B_w	
};


/*********************
*   Machine driver   *
*********************/

static MACHINE_DRIVER_START( re900 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", I8051, MASTER_CLOCK)
	MDRV_CPU_PROGRAM_MAP(mem_prg, 0)
	//MDRV_CPU_DATA_MAP(mem_data, 0)
	MDRV_CPU_IO_MAP(mem_io, 0)
	MDRV_CPU_VBLANK_INT("main", re900_video_interrupt) 

	/* video hardware */
	MDRV_IMPORT_FROM(tms9928a)
	MDRV_SCREEN_MODIFY("main")
	MDRV_SCREEN_REFRESH_RATE(60)

	MDRV_NVRAM_HANDLER(generic_0fill)

	/* sound hardware	*/
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910, TMS_CLOCK) /* From TMS9128NL - Pin 37 (GROMCLK) */
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)

MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( re900 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "re900.bin", 0x0000, 0x10000, CRC(967ae944) SHA1(104bab79fd50a8e38ae15058dbe47a59f1ec4b05) )
ROM_END


/************************
*      Driver Init      *
************************/

static DRIVER_INIT( re900 )
{
	/* External RAM callback */
	//i8051_set_eram_iaddr_callback(re900_external_ram_iaddr);
	TMS9928A_configure(&tms9928a_interface);
}

/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME    PARENT  MACHINE  INPUT    INIT    ROT     COMPANY                    FULLNAME        FLAGS   */
GAME( 1990, re900,  0,      re900,   re900,   re900,  ROT90, "Entretenimientos GEMINI", "Ruleta RE-900", 0 )
//GAMEL( 1990, re900, 0, re900, re900, re900, ROT90, "GEMINI", "Ruleta RE-900", 0, layout_re900 )
