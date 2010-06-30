/***************************************************************************

    JPM IMPACT with Video hardware

    driver by Phil Bennett

    Games supported:
        * Coronation Street Quiz Game
        * Cluedo (3 sets)
        * Hangman
        * Scrabble
        * Trivial Pursuit

    ROMS wanted:
        * Snakes and Ladders

    Known issues:
        * I/O documentation for lamps, reels, meters etc is possibly incorrect.
        * DUART emulation is very simplistic, in progress.
        * Digital volume control is not emulated.

        * During the attract mode of Cluedo, just after the camera flash,
        the camera colours go screwy and proceeding text is printed behind
        the camera. Is it a TMS34010 emulation flaw or original game code bug?
        For now, the GAME_IMPERFECT_GRAPHICS flag remains.

****************************************************************************

    Memory map (preliminary)

****************************************************************************

    ========================================================================
    Main CPU (68000)
    ========================================================================
    000000-0FFFFF   R     xxxxxxxx xxxxxxxx   Program ROM bank 1
    100000-1FFFFF   R     xxxxxxxx xxxxxxxx   Program ROM bank 2
    400000-403FFF   R/W   xxxxxxxx xxxxxxxx   Program RAM (battery-backed)
    480000-48001F   R/W   -------- xxxxxxxx   MC68681 DUART 1
    480020-480033   R     -------- xxxxxxxx   Inputs
    480041          R     -xxxxxxx xxxxxxxx   Reel optos
    480060-480067   R/W   -------- xxxxxxxx   D71055C? PPI?
    480080-480081     W   -------- xxxxxxxx   uPD7559 communications
    480082-480083     W   -------- xxxxxxxx   Sound control
                          -------- -------x      (uPD7759 reset)
                          -------- -----xx-      (ROM A18-A17)
                          -------- ---x----      (X9C103 /INC)
                          -------- --x-----      (X9C103 U/#D)
                          -------- -x------      (X9C103 /CS)
    480084-480085   R     -------- xxxxxxxx   uPD7759 communications
    4800A0-4800AF     W   xxxxxxxx xxxxxxxx   Lamps?
    4800E0-4800E1     W   xxxxxxxx xxxxxxxx   Reset and status LEDs?
    4801DC-4801DD   R     -------- xxxxxxxx   Unknown
    4801DE-4801DF   R     -------- xxxxxxxx   Unknown
    4801E0-4801FF   R/W   -------- xxxxxxxx   MC68681 DUART 2 (on ROM PCB)
    800000-800007   R/W   xxxxxxxx xxxxxxxx   TMS34010 interface
    C00000-CFFFFF   R     xxxxxxxx xxxxxxxx   Question ROM bank 1
    D00000-DFFFFF   R     xxxxxxxx xxxxxxxx   Question ROM bank 2
    E00000-EFFFFF   R     xxxxxxxx xxxxxxxx   Question ROM bank 3
    F00000-FFFFFF   R     xxxxxxxx xxxxxxxx   Question ROM bank 4
    ========================================================================
    Interrupts:
        IRQ2 = TMS34010
        IRQ5 = MC68681 1
        IRQ6 = Watchdog?
        IRQ7 = Power failure detect
    ========================================================================

    ========================================================================
    Video CPU (TMS34010, all addresses are in bits)
    ========================================================================
    -----000 00xxxxxx xxxxxxxx xxxxxxxx   Video RAM
    -----000 1xxxxxxx xxxxxxxx xxxxxxxx   ROM
    -----010 0xxxxxxx xxxxxxxx xxxxxxxx   ROM
    -----001 0------- -------- --xxxxxx   Bt477 RAMDAC
    -----111 1-xxxxxx xxxxxxxx xxxxxxxx   RAM

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms34010/tms34010.h"
#include "sound/upd7759.h"
#include "includes/jpmimpct.h"
#include "machine/meters.h"

/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 tms_irq;
static UINT8 duart_1_irq;
static UINT8 touch_cnt;
static UINT8 touch_data[3];

static int lamp_strobe;
static UINT8 Lamps[256];


/*************************************
 *
 *  MC68681 DUART (TODO)
 *
 *************************************/

#define MC68681_1_CLOCK		3686400
#define MC68681_2_CLOCK		3686400

static struct
{
	UINT8 MR1A, MR2A;
	UINT8 SRA, CSRA;
	UINT8 CRA;
	UINT8 RBA, TBA;

	UINT8 IPCR;
	UINT8 ACR;
	UINT8 ISR, IMR;

	union
	{
		UINT8 CUR, CLR;
		UINT16 CR;
	};
	union
	{
		UINT8 CTUR, CTLR;
		UINT16 CT;
	};

	int tc;

	UINT8 MR1B, MR2B;
	UINT8 SRB, CSRB;
	UINT8 CRB;
	UINT8 RBB, TBB;

	UINT8 IVR;
	UINT8 IP;
	UINT8 OP;
	UINT8 OPR;
	UINT8 OPCR;
} duart_1;//, duart_2;


/*************************************
 *
 *  68000 IRQ handling
 *
 *************************************/

static void update_irqs(running_machine *machine)
{
	cputag_set_input_line(machine, "maincpu", 2, tms_irq ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", 5, duart_1_irq ? ASSERT_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static MACHINE_START( jpmimpct )
{
	state_save_register_global(machine, tms_irq);
	state_save_register_global(machine, duart_1_irq);
	state_save_register_global(machine, touch_cnt);
	state_save_register_global_array(machine, touch_data);

	/* TODO! */
	state_save_register_global(machine, duart_1.ISR);
	state_save_register_global(machine, duart_1.IMR);
	state_save_register_global(machine, duart_1.CT);
}


static MACHINE_RESET( jpmimpct )
{
	memset(&duart_1, 0, sizeof(duart_1));

	/* Reset states */
	duart_1_irq = tms_irq = 0;
	touch_cnt = 0;

//  duart_1.IVR=0x0f;
}


/*************************************
 *
 *  TMS34010 host interface
 *
 *************************************/

static WRITE16_HANDLER( m68k_tms_w )
{
	tms34010_host_w(devtag_get_device(space->machine, "dsp"), offset, data);
}

static READ16_HANDLER( m68k_tms_r )
{
	return tms34010_host_r(devtag_get_device(space->machine, "dsp"), offset);
}


/*************************************
 *
 *  MC68681 DUART 1
 *
 *************************************/

/*
 *  IP0: MC1489P U7 pin 8
 *  IP1: MC1489P U12 pin 6
 *  IP2: MC1489P U7 pin 11
 *  IP3: MC1489P U12 pin 3
 *  IP4: LM393N U2 pin 1
 *       - Coin meter sense (0 = meter active)
 *  IP5: TEST/DEMO PCB push switch
 *
 *  OP0: SN75188 U6 pins 9 & 10 -> SERIAL PORT pin 6
 *  OP1:
 *  OP2:
 *  OP3: DM7406N U4 pin 3 -> J7 pin 7 (COIN MECH)
 *  OP4: DM7406N U4 pin 5
 *  OP5: DM7406N U4 pin 9 -> J7 pin 5 (COIN MECH)
 *  OP6: DM7406N U4 pin 12
 *  OP7: DM7406N U4 pin 13 -> J7 pin ? (COIN MECH)
 *
 *  TxDA/RxDA: Auxillary serial port
 *  TxDB/TxDB: Data retrieval unit
 */

static TIMER_DEVICE_CALLBACK( duart_1_timer_event )
{
	duart_1.tc = 0;
	duart_1.ISR |= 0x08;

	duart_1_irq = 1;
	update_irqs(timer.machine);
}

static READ16_HANDLER( duart_1_r )
{
	UINT16 val = 0xffff;
	switch (offset)
	{
		case 0x1:
		{
			/* RxDA ready */
			val = 0x04;
			break;
		}
		case 0x2:
		{
			val = 0x00;
			break;
		}
		case 0x3:
		{
			val = duart_1.RBA;
			duart_1.ISR &= ~0x02;
			duart_1.SRA &= ~0x03;
			break;
		}
		case 0x4:
		{
			val = duart_1.IPCR;
			duart_1.ISR &= ~0x80;
			break;
		}
		case 0x5:
		{
			val = duart_1.ISR;
			break;
		}
		case 0x9:
		{
			/* RxDB ready */
			val = 0x04;
			break;
		}
		case 0xd:
		{
			val = input_port_read(space->machine, "TEST/DEMO");
			break;
		}
		case 0xe:
		{
			attotime rate = attotime_mul(ATTOTIME_IN_HZ(MC68681_1_CLOCK), 16 * duart_1.CT);
			timer_device *duart_timer = space->machine->device<timer_device>("duart_1_timer");
			duart_timer->adjust(rate, 0, rate);
			break;
		}
		case 0xf:
		{
			duart_1_irq = 0;
			update_irqs(space->machine);
			duart_1.ISR |= ~0x8;
			break;
		}
	}

	return val;
}

static WRITE16_HANDLER( duart_1_w )
{
	//int old_val;
	switch (offset)
	{
		case 0x1:
		{
			duart_1.CSRA = data;
			break;
		}
		case 0x3:
		{
			//mame_printf_debug("%c", data);
			break;
		}
		case 0x4:
		{
			duart_1.ACR = data;

			/* Only handle counter mode, XTAL divide by 16 */
			if (((data >> 4) & 7) != 0x7)
			{
				logerror("DUART 1: Unhandled counter mode: %x\n", data);
			}
			break;
		}
		case 0x5:
		{
			duart_1.IMR = data;
			break;
		}
		case 0x6:
		{
			duart_1.CTUR = data;
			break;
		}
		case 0x7:
		{
			duart_1.CTLR = data;
			break;
		}
		case 0xb:
		{
			//mame_printf_debug("%c",data);
			break;
		}
		case 0xc:
		{
			duart_1.IVR = data;
			break;
		}
		case 0xd:
		{
			duart_1.OPCR = data;
			break;
		}
		case 0xe:
		{
		    //old_val = duart_1.OPR;
		    duart_1.OPR = duart_1.OPR | data;
		    duart_1.OP = ~duart_1.OPR;
			/* Output port bit set */
			break;
		}
		case 0xf:
		{
		    //old_val = duart_1.OPR;
		    duart_1.OPR = duart_1.OPR &~data;
		    duart_1.OP = ~duart_1.OPR;
			/* Output port bit reset */
			break;
		}
	}
}

/*************************************
 *
 *  MC68681 DUART 2
 *
 *************************************/

/*
    Communication with a touchscreen interface PCB
    is handled via UART B.
*/
static READ16_HANDLER( duart_2_r )
{
	switch (offset)
	{
		case 0x9:
		{
			if (touch_cnt == 0)
			{
				if ( input_port_read(space->machine, "TOUCH") & 0x1 )
				{
					touch_data[0] = 0x2a;
					touch_data[1] = 0x7 - (input_port_read(space->machine, "TOUCH_Y") >> 5) + 0x30;
					touch_data[2] = (input_port_read(space->machine, "TOUCH_X") >> 5) + 0x30;

					/* Return RXRDY */
					return 0x1;
				}
				return 0;
			}
			else
			{
				return 1;
			}
		}
		case 0xb:
		{
			UINT16 val = touch_data[touch_cnt];

			if (touch_cnt++ == 3)
				touch_cnt = 0;

			return val;
		}
		default:
			return 0;
	}
}

/*
    Nothing important here?
*/
static WRITE16_HANDLER( duart_2_w )
{
}


/*************************************
 *
 *  I/O handlers
 *
 *************************************/

/*
 *  0: DIP switches
 *  1: Percentage key
 *  2: Lamps + switches (J10)
 *  3: Lamps + switches (J10)
 *  4: Lamps + switches (J10)
 *      ---- ---x   Back door
 *      ---- --x-   Cash door
 *      ---- -x--   Refill key
 *  5: Lamps + switches (J9)
 *  6: Lamps + switches (J9)
 *  7: Lamps + switches (J9)
 *  8: Payslides
 *  9: Coin mechanism
 */

static READ16_HANDLER( inputs1_r )
{
	UINT16 val = 0x00ff;

	switch (offset)
	{
		case 0:
		{
			val = input_port_read(space->machine, "DSW");
			break;
		}
		case 2:
		{
			val = input_port_read(space->machine, "SW2");
			break;
		}
		case 4:
		{
			val = input_port_read(space->machine, "SW1");
			break;
		}
		case 9:
		{
			val = input_port_read(space->machine, "COINS");
			break;
		}
	}

	return val;
}


/*************************************
 *
 *  Sound control
 *
 *************************************/
static WRITE16_DEVICE_HANDLER( volume_w )
{
	if (ACCESSING_BITS_0_7)
	{
		upd7759_set_bank_base(device, 0x20000 * ((data >> 1) & 3));
		upd7759_reset_w(device, data & 0x01);
	}
}

static WRITE16_DEVICE_HANDLER( upd7759_w )
{
	if (ACCESSING_BITS_0_7)
	{
		upd7759_port_w(device, 0, data);
		upd7759_start_w(device, 0);
		upd7759_start_w(device, 1);
	}
}

static READ16_DEVICE_HANDLER( upd7759_r )
{
	if (ACCESSING_BITS_0_7)
	{
		return upd7759_busy_r(device);
	}

	return 0xffff;
}

/*************************************
 *
 *  Mysterious stuff
 *
 *************************************/

static READ16_HANDLER( unk_r )
{
	return 0xffff;
}

static WRITE16_HANDLER( unk_w )
{
}

static void jpm_draw_lamps(int data, int lamp_strobe)
{
	int i;
	for (i=0; i<16; i++)
	{
		Lamps[16*(lamp_strobe+i)] = data & 1;
		output_set_lamp_value((16*lamp_strobe)+i, (Lamps[(16*lamp_strobe)+i]));
	    data = data >> 1;
    }
}

static READ16_HANDLER( jpmio_r )
{
	return 0xffff;
}

static WRITE16_HANDLER( jpmio_w )
{
	UINT64 cycles = space->machine->firstcpu->total_cycles();
	switch (offset)
	{
		case 0x02:
		{
			//reel 1
			break;
		}
		case 0x04:
		{
			//reel 2
			break;
		}
		case 0x06:
		{
			if ( data & 0x10 )
			{   // PAYEN ?
				if ( data & 0xf )
				{
			//      slide = 1;
				}
				else
				{
				//  slide = 0;
				}
			}
			else
//          slide = 0;
			Mechmtr_update(0, cycles, data >> 10);
			duart_1.IP &= ~0x10;
			break;
		}

		case 0x08:
		{
			jpm_draw_lamps(data, lamp_strobe);
			break;
		}

		case 0x0b:
		{
			output_set_digit_value(lamp_strobe,data);
			break;
		}
		case 0x0f:
		{
			if (data & 0x10)
			{
				lamp_strobe = (data +1) & 0x0f;
			}
			break;
		}
	}
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/
static ADDRESS_MAP_START( m68k_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM
	AM_RANGE(0x00100000, 0x001fffff) AM_ROM
	AM_RANGE(0x00400000, 0x00403fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x00480000, 0x0048001f) AM_READWRITE(duart_1_r, duart_1_w)
	AM_RANGE(0x00480020, 0x00480033) AM_READ(inputs1_r)
	AM_RANGE(0x00480034, 0x00480035) AM_READ(unk_r)
	AM_RANGE(0x00480060, 0x00480067) AM_READWRITE(unk_r, unk_w)//PPI
	AM_RANGE(0x004800a0, 0x004800af) AM_READWRITE(jpmio_r, jpmio_w)
	AM_RANGE(0x004800e0, 0x004800e1) AM_WRITE(unk_w)
	AM_RANGE(0x004801dc, 0x004801dd) AM_READ(unk_r)
	AM_RANGE(0x004801de, 0x004801df) AM_READ(unk_r)
	AM_RANGE(0x00480080, 0x00480081) AM_DEVWRITE("upd", upd7759_w)
	AM_RANGE(0x00480082, 0x00480083) AM_DEVWRITE("upd", volume_w)
	AM_RANGE(0x00480084, 0x00480085) AM_DEVREAD("upd", upd7759_r)
	AM_RANGE(0x004801e0, 0x004801ff) AM_READWRITE(duart_2_r, duart_2_w)
	AM_RANGE(0x00800000, 0x00800007) AM_READWRITE(m68k_tms_r, m68k_tms_w)
	AM_RANGE(0x00c00000, 0x00cfffff) AM_ROM
	AM_RANGE(0x00d00000, 0x00dfffff) AM_ROM
	AM_RANGE(0x00e00000, 0x00efffff) AM_ROM
	AM_RANGE(0x00f00000, 0x00ffffff) AM_ROM
ADDRESS_MAP_END


/*************************************
 *
 *  Video CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( tms_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0x00000000, 0x003fffff) AM_MIRROR(0xf8000000) AM_RAM AM_BASE(&jpmimpct_vram)
	AM_RANGE(0x00800000, 0x00ffffff) AM_MIRROR(0xf8000000) AM_ROM AM_REGION("user1", 0x100000)
	AM_RANGE(0x02000000, 0x027fffff) AM_MIRROR(0xf8000000) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0x01000000, 0x0100003f) AM_MIRROR(0xf87fffc0) AM_READWRITE(jpmimpct_bt477_r, jpmimpct_bt477_w)
	AM_RANGE(0x07800000, 0x07bfffff) AM_MIRROR(0xf8400000) AM_RAM
ADDRESS_MAP_END


/*************************************
 *
 *  Input definitions
 *
 *************************************/

static INPUT_PORTS_START( common )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW 0 (toggle to stop alarm)")
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 1")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2")
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW 3")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW 4")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 5")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW 6")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW 7")
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_TOGGLE PORT_NAME( "Back Door" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE PORT_NAME( "Cash Door" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_TOGGLE PORT_NAME( "Refill Key" )

	PORT_START("TEST/DEMO")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME( "Test/Demo" )
INPUT_PORTS_END

static INPUT_PORTS_START( touchscreen )
	PORT_START("TOUCH")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_IMPULSE(1) PORT_NAME( "Touch screen" )

	PORT_START("TOUCH_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("TOUCH_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

static INPUT_PORTS_START( hngmnjpm )
	PORT_INCLUDE( common )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 1 pound" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 50p" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 20p" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 10p" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_IMPULSE(1) PORT_NAME( "Token: 20" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 5p" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "'3'" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "'2'" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "'1'" )
INPUT_PORTS_END

static INPUT_PORTS_START( coronatn )
	PORT_INCLUDE( common )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 1 pound" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 50p" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 20p" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 10p" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_IMPULSE(1) PORT_NAME( "Token: 20" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 5p" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Ask Ken" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "'1'" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "'2'" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "'3'" )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( cluedo )
	PORT_INCLUDE( common )

	PORT_INCLUDE( touchscreen )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 1 pound" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 50p" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 20p" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 10p" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( trivialp )
	PORT_INCLUDE( common )

	PORT_INCLUDE( touchscreen )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 1 pound" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 50p" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 20p" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 10p" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Pass" )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( scrabble )
	PORT_INCLUDE( common )

	PORT_INCLUDE( touchscreen )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 1 pound" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 50p" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 20p" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 10p" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 5p" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


/*************************************
 *
 *  TMS34010 configuration
 *
 *************************************/

static void jpmimpct_tms_irq(running_device *device, int state)
{
	tms_irq = state;
	update_irqs(device->machine);
}

static const tms34010_config tms_config =
{
	TRUE,                       /* halt on reset */
	"screen",                   /* the screen operated on */
	40000000/16,                /* pixel clock */
	4,                          /* pixels per clock */
	jpmimpct_scanline_update,   /* scanline updater */
	jpmimpct_tms_irq,           /* generate interrupt */
	jpmimpct_to_shiftreg,       /* write to shiftreg function */
	jpmimpct_from_shiftreg      /* read from shiftreg function */
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( jpmimpct )
	MDRV_CPU_ADD("maincpu", M68000, 8000000)
	MDRV_CPU_PROGRAM_MAP(m68k_program_map)

	MDRV_CPU_ADD("dsp", TMS34010, 40000000)
	MDRV_CPU_CONFIG(tms_config)
	MDRV_CPU_PROGRAM_MAP(tms_program_map)

	MDRV_QUANTUM_TIME(HZ(30000))
	MDRV_MACHINE_START(jpmimpct)
	MDRV_MACHINE_RESET(jpmimpct)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_TIMER_ADD( "duart_1_timer", duart_1_timer_event)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(40000000/4, 156*4, 0, 100*4, 328, 0, 300)
	MDRV_PALETTE_LENGTH(256)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_VIDEO_START(jpmimpct)
	MDRV_VIDEO_UPDATE(tms340x0)
MACHINE_DRIVER_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( cluedo )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7322.bin", 0x000000, 0x080000, CRC(049ad02d) SHA1(10297dd466d0019e8d6c162028a23dd235494fb4) )
	ROM_LOAD16_BYTE( "7323.bin", 0x000001, 0x080000, CRC(47ce9c40) SHA1(596a1628142d3c81f2c4ab11ed421f27d082d5f6) )
	ROM_LOAD16_BYTE( "7324.bin", 0x100000, 0x080000, CRC(5946bd75) SHA1(cc4ffa1e4c3628de6b60027d95df413b6d94e669) )
	ROM_LOAD16_BYTE( "7325.bin", 0x100001, 0x080000, CRC(416843ab) SHA1(0d758f7df96384a04596366b1864d5005ca540ee) )

	ROM_LOAD16_BYTE( "6977.bin", 0xc00000, 0x080000, CRC(6030dfc1) SHA1(8746909b0b7f7eb99cf5388ac85db6addb6deee3) )
	ROM_LOAD16_BYTE( "6978.bin", 0xc00001, 0x080000, CRC(21e30e06) SHA1(4e97baa9e39663b662dd202bbaf34be0e29930de) )
	ROM_LOAD16_BYTE( "6979.bin", 0xd00000, 0x080000, CRC(5575162a) SHA1(27f7b5f4ee7d95319b03e2414a25d5b1a6c54fc7) )
	ROM_LOAD16_BYTE( "6980.bin", 0xd00001, 0x080000, CRC(968224df) SHA1(726c278622681206a7f34bafe1b5bb4421232cc4) )
	ROM_LOAD16_BYTE( "6981.bin", 0xe00000, 0x080000, CRC(2ad3ee20) SHA1(9370dab84a255864f40254772199211884d8557b) )
	ROM_LOAD16_BYTE( "6982.bin", 0xe00001, 0x080000, CRC(7478e91b) SHA1(158b473b46aeccf011669cb58dc3a1596370d8f1) )
	ROM_FILL(                    0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "clugrb1", 0x000000, 0x80000, CRC(176ae2df) SHA1(135fd2640c255e5321b1a6ba35f72fa2ba8f04b8) )
	ROM_LOAD16_BYTE( "clugrb2", 0x000001, 0x80000, CRC(06ab2f78) SHA1(4325fd9096e73956310e97e244c7fe1ee8d27f5c) )
	ROM_COPY( "user1", 0x000000, 0x100000, 0x100000 )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "1214.bin", 0x000000, 0x80000, CRC(fe43aeae) SHA1(017a471af5766ef41fa46982c02941fb4fc35174) )
ROM_END

ROM_START( cluedo2c )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clu2c1.bin", 0x000000, 0x080000, CRC(bf94a3c0) SHA1(e5a0d17136691642aba339f574aec7c27ed90848) )
	ROM_LOAD16_BYTE( "clu2c2.bin", 0x000001, 0x080000, CRC(960cda80) SHA1(6b5946ed1241bc673f42991f57e0c74753085b63) )
	ROM_LOAD16_BYTE( "clu2c3.bin", 0x100000, 0x080000, CRC(9d61b28d) SHA1(41c0e17b3933686a2e6f343cd39f90e5663c7787) )
	ROM_LOAD16_BYTE( "clu2c4.bin", 0x100001, 0x080000, CRC(a427d67b) SHA1(a8944e1d86548911a65b398245a0f8f236491644) )

	ROM_LOAD16_BYTE( "6977.bin", 0xc00000, 0x080000, CRC(6030dfc1) SHA1(8746909b0b7f7eb99cf5388ac85db6addb6deee3) )
	ROM_LOAD16_BYTE( "6978.bin", 0xc00001, 0x080000, CRC(21e30e06) SHA1(4e97baa9e39663b662dd202bbaf34be0e29930de) )
	ROM_LOAD16_BYTE( "6979.bin", 0xd00000, 0x080000, CRC(5575162a) SHA1(27f7b5f4ee7d95319b03e2414a25d5b1a6c54fc7) )
	ROM_LOAD16_BYTE( "6980.bin", 0xd00001, 0x080000, CRC(968224df) SHA1(726c278622681206a7f34bafe1b5bb4421232cc4) )
	ROM_LOAD16_BYTE( "6981.bin", 0xe00000, 0x080000, CRC(2ad3ee20) SHA1(9370dab84a255864f40254772199211884d8557b) )
	ROM_LOAD16_BYTE( "6982.bin", 0xe00001, 0x080000, CRC(7478e91b) SHA1(158b473b46aeccf011669cb58dc3a1596370d8f1) )
	ROM_FILL(                    0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "clugrb1", 0x000000, 0x80000, CRC(176ae2df) SHA1(135fd2640c255e5321b1a6ba35f72fa2ba8f04b8) )
	ROM_LOAD16_BYTE( "clugrb2", 0x000001, 0x80000, CRC(06ab2f78) SHA1(4325fd9096e73956310e97e244c7fe1ee8d27f5c) )
	ROM_COPY( "user1", 0x000000, 0x100000, 0x100000 )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "1214.bin", 0x000000, 0x80000, CRC(fe43aeae) SHA1(017a471af5766ef41fa46982c02941fb4fc35174) )
ROM_END

ROM_START( cluedo2 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clu21.bin", 0x000000, 0x080000, CRC(b1aa0103)SHA1(52d10a428710cd04313a2638fc3c23fb9d0ab6db))
	ROM_LOAD16_BYTE( "clu22.bin", 0x000001, 0x080000, CRC(90d8dd28)SHA1(3124a8313c6b362176283e145c4af27f5deac683))
	ROM_LOAD16_BYTE( "clu23.bin", 0x100000, 0x080000, CRC(196bd993)SHA1(50920441707fc6cae9d36961d92ce213e53c4238))
	ROM_LOAD16_BYTE( "clu24.bin", 0x100001, 0x080000, CRC(3f5c1259)SHA1(dfdbb66a81716a0ced7510e277f6f321516f57af))

	ROM_LOAD16_BYTE( "6977.bin", 0xc00000, 0x080000, CRC(6030dfc1) SHA1(8746909b0b7f7eb99cf5388ac85db6addb6deee3) )
	ROM_LOAD16_BYTE( "6978.bin", 0xc00001, 0x080000, CRC(21e30e06) SHA1(4e97baa9e39663b662dd202bbaf34be0e29930de) )
	ROM_LOAD16_BYTE( "6979.bin", 0xd00000, 0x080000, CRC(5575162a) SHA1(27f7b5f4ee7d95319b03e2414a25d5b1a6c54fc7) )
	ROM_LOAD16_BYTE( "6980.bin", 0xd00001, 0x080000, CRC(968224df) SHA1(726c278622681206a7f34bafe1b5bb4421232cc4) )
	ROM_LOAD16_BYTE( "6981.bin", 0xe00000, 0x080000, CRC(2ad3ee20) SHA1(9370dab84a255864f40254772199211884d8557b) )
	ROM_LOAD16_BYTE( "6982.bin", 0xe00001, 0x080000, CRC(7478e91b) SHA1(158b473b46aeccf011669cb58dc3a1596370d8f1) )
	ROM_FILL(                    0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "clugrb1", 0x000000, 0x80000, CRC(176ae2df) SHA1(135fd2640c255e5321b1a6ba35f72fa2ba8f04b8) )
	ROM_LOAD16_BYTE( "clugrb2", 0x000001, 0x80000, CRC(06ab2f78) SHA1(4325fd9096e73956310e97e244c7fe1ee8d27f5c) )
	ROM_COPY( "user1", 0x000000, 0x100000, 0x100000 )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "clue2as1.bin", 0x000000, 0x80000, CRC(16b2bc45) SHA1(56963f5d63b5a091b89b96f4ca9327010006c024) )
ROM_END

ROM_START( trivialp )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1422.bin", 0x000000, 0x080000, CRC(5e39c946) SHA1(bae7f572a32e90d716813271f03e7868be603086) )
	ROM_LOAD16_BYTE( "1423.bin", 0x000001, 0x080000, CRC(bb48c225) SHA1(b479f0bdb69ad11af17b5457c02a9d9618ede455) )
	ROM_LOAD16_BYTE( "1424.bin", 0x100000, 0x080000, CRC(c37d045b) SHA1(3c127b14e1dc1e453fb08c741847c712d1fea78b) )
	ROM_LOAD16_BYTE( "1425.bin", 0x100001, 0x080000, CRC(8d209f61) SHA1(3e16ee4c43a31da2e6773a938a20c616a5e6179b) )

	ROM_LOAD16_BYTE( "tp-q1.bin", 0xc00000, 0x080000, CRC(98d42cfd) SHA1(67a6745d55493034128f767b518d86dedc9c22a6) )
	ROM_LOAD16_BYTE( "tp-q2.bin", 0xc00001, 0x080000, CRC(8a670ee8) SHA1(33628b34f4a0413f2f39e26520169d0eff9942c5) )
	ROM_LOAD16_BYTE( "tp-q3.bin", 0xd00000, 0x080000, CRC(eb47f94e) SHA1(957812b63de4532b9175214db7947c96264a48f1) )
	ROM_LOAD16_BYTE( "tp-q4.bin", 0xd00001, 0x080000, CRC(23c01c99) SHA1(187c3448ae1cb44ca6a4a829e64b860ee7548ac5) )
	ROM_LOAD16_BYTE( "tp-q5.bin", 0xe00000, 0x080000, CRC(1c9f4f8a) SHA1(7541d518d24e59140d62a869b27bcc15b205054d) )
	ROM_LOAD16_BYTE( "tp-q6.bin", 0xe00001, 0x080000, CRC(df9da57d) SHA1(a3e29cb03bd780de2c5454c86d6dc48e1c6c63bc) )
	ROM_LOAD16_BYTE( "tp-q7.bin", 0xf00000, 0x080000, CRC(e075e5d7) SHA1(3490730c569678d48fb2d810484de063882f71a5) )
	ROM_LOAD16_BYTE( "tp-q8.bin", 0xf00001, 0x080000, CRC(12f90e74) SHA1(a39a1cee6107d1e83954e3cabf191fd5c89777f8) )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "tp-gr1.bin", 0x000000, 0x100000, CRC(7fa955f7) SHA1(9ecae4c8c26bfa1701c39148099bf0f8b5974ac8) )
	ROM_LOAD16_BYTE( "tp-gr2.bin", 0x000001, 0x100000, CRC(2495d785) SHA1(eb89eb299a7000364a0a0f59459d1ec27755fca1) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "tp-snd.bin", 0x000000, 0x80000, CRC(7e2cb00a) SHA1(670ee5dd5c60313676b9271901b4df9e6ebd5955) )
ROM_END

ROM_START( scrabble )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1562.bin", 0x000000, 0x080000, CRC(d7303b98) SHA1(46e8ed04c8fdc092b7d8910d3e3f6cc62f691646) )
	ROM_LOAD16_BYTE( "1563.bin", 0x000001, 0x080000, CRC(77f61ba1) SHA1(276dc8b2c23880740309c456d4e4b2eae249cdde) )
	ROM_FILL(                    0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "scra-q1.bin", 0xc00000, 0x080000, CRC(bcbc6328) SHA1(cbf8901e80e7bc1f82f6f7d4d5f6a658af98a6f9) )
	ROM_LOAD16_BYTE( "scra-q2.bin", 0xc00001, 0x080000, CRC(c2147999) SHA1(f21dc0f3f4ba0d6304801bc492a759534447d747) )
	ROM_LOAD16_BYTE( "scra-q3.bin", 0xd00000, 0x080000, CRC(622cebb9) SHA1(9b7c2204462d4912462bad6c4dcf096abe1381bb) )
	ROM_LOAD16_BYTE( "scra-q4.bin", 0xd00001, 0x080000, CRC(fd4b587b) SHA1(e29512a075fbc511271d6902c8900a9b0261355c) )
	ROM_LOAD16_BYTE( "scra-q5.bin", 0xe00000, 0x080000, CRC(fbc28978) SHA1(ce2549da858888d49677ec982ab3c21cf292939b) )
	ROM_LOAD16_BYTE( "scra-q6.bin", 0xe00001, 0x080000, CRC(8b792c9c) SHA1(9a5cc6c4d7e807cbabd174ab7454cdaa93dc3cec) )
	ROM_FILL(                       0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "scra-g1.bin", 0x000000, 0x100000, CRC(04a17df9) SHA1(c215c90d8add3ff608c24aac242369874f6bf9d7) )
	ROM_LOAD16_BYTE( "scra-g2.bin", 0x000001, 0x100000, CRC(724375e6) SHA1(709211a2d7b86f4e83c94a37010fe61ef9a734de) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "scra-snd.bin", 0x000000, 0x80000, CRC(287759ef) SHA1(bd37500689b7b2fb4fbc65056e92486c0c00ff61) )
ROM_END

ROM_START( hngmnjpm )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20264.bin", 0x000000, 0x080000, CRC(50074528) SHA1(8128b2270518af873df4b94d50c5c9849dda3e42) )
	ROM_LOAD16_BYTE( "20265.bin", 0x000001, 0x080000, CRC(a0a6985c) SHA1(ed960e6e88df111aebf208d7105dc241aa916684) )
	ROM_FILL(                     0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "hang-q1.bin", 0xc00000, 0x080000, CRC(0be99a57) SHA1(49fe7faeccd3f9608927ff333fd5783e3cd7d266) )
	ROM_LOAD16_BYTE( "hang-q2.bin", 0xc00001, 0x080000, CRC(71328f71) SHA1(59481b27dbcad109070cc4fd5c9c93f948991f03) )
	ROM_LOAD16_BYTE( "hang-q3.bin", 0xd00000, 0x080000, CRC(3fabeb81) SHA1(67b4561ec4ac8c00728c86e2bce66f432c5f1e86) )
	ROM_LOAD16_BYTE( "hang-q4.bin", 0xd00001, 0x080000, CRC(64fbf56b) SHA1(c5077f9995b890925ef608742ba77ef995de5a3b) )
	ROM_LOAD16_BYTE( "hang-q5.bin", 0xe00000, 0x080000, CRC(283e0c7f) SHA1(64ed626e181d851d3ffd4a1c0e613cd769e0ae31) )
	ROM_LOAD16_BYTE( "hang-q6.bin", 0xe00001, 0x080000, CRC(9a6d3667) SHA1(b4706d77dcd43e6f75e3e5e8bd1fbeebe84b8f60) )
	ROM_FILL(                       0xf00000, 0x100000, 0xff )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "hang-gr1.bin", 0x000000, 0x100000, CRC(5919344c) SHA1(b5c1f98ebfc65743fa2f6c264179ed7115532a6b) )
	ROM_LOAD16_BYTE( "hang-gr2.bin", 0x000001, 0x100000, CRC(3194c6d4) SHA1(11d5e7bfe60912b0eab2a1d06d1a74853ec23567) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "hang-so1.bin", 0x000000, 0x80000, CRC(5efe1712) SHA1(e4e7a73a1b1897ed6e96306f99d234fb3b47c59b) )

	/* Likely to be the same for the other games */
	ROM_REGION( 0x0a00, "plds", 0 )
	ROM_LOAD( "s60-3.bin", 0x0000, 0x0117, CRC(19e1d28b) SHA1(12dff4bea16b95807f1a9455b6785468ca5de858) )
	ROM_LOAD( "s61-6.bin", 0x0000, 0x0117, CRC(c72cec0e) SHA1(9d6e5510600987f9359af9ecc3e95f5bd8444bcd) )
	ROM_LOAD( "ig1.1.bin", 0x0000, 0x02DD, CRC(4e11fa4e) SHA1(ded2d2086c4360708462024054e5409962ea8589) )
	ROM_LOAD( "ig2.1.bin", 0x0000, 0x0157, CRC(2365878b) SHA1(d91d9906aadcfd8cff7ee6b92449c522f73a29e1) )
	ROM_LOAD( "ig3.2.bin", 0x0000, 0x0117, CRC(4970dad7) SHA1(c5931db3d66c7d1027a762be10f9e3d9e321b70f) )
	ROM_LOAD( "jpms6.bin", 0x0000, 0x0117, CRC(1fba3b6f) SHA1(0e33e49cbf24e836deb1ef16385ff20549ef188e) )
	ROM_LOAD( "mem-2.bin", 0x0000, 0x0157, CRC(92832445) SHA1(b6edcc6d4f721f0e91e9fcf322163db017afaee1) )
ROM_END

ROM_START( coronatn )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20824.bin", 0x000000, 0x080000, CRC(f5cc07cb) SHA1(45b83829ba9bd5f22c2978bbde9c0e25c476e719) )
	ROM_LOAD16_BYTE( "20825.bin", 0x000001, 0x080000, CRC(2e749edf) SHA1(12b24836a71085aef8ca1bc61e6671f8d6e1908c) )
	ROM_FILL(                     0x100000, 0x100000, 0xff )

	ROM_LOAD16_BYTE( "cs-q1.bin", 0xc00000, 0x080000, CRC(beef496a) SHA1(6089ee8b0821d5b8cb8f724748888a0915083622) )
	ROM_LOAD16_BYTE( "cs-q2.bin", 0xc00001, 0x080000, CRC(16f88f36) SHA1(78c829d837cc09fdd1119ba73168d272843f7f50) )
	ROM_LOAD16_BYTE( "cs-q3.bin", 0xd00000, 0x080000, CRC(1d412b03) SHA1(2400fa776effeb2ab21234a6ecf183ed0cffa92e) )
	ROM_LOAD16_BYTE( "cs-q4.bin", 0xd00001, 0x080000, CRC(55c23ab9) SHA1(0eaa8c88315ef4544f1d1ef2fec2c6edc3589db3) )
	ROM_LOAD16_BYTE( "cs-q5.bin", 0xe00000, 0x080000, CRC(289f4db0) SHA1(8eca9df9e278bf77be4b2aad4c80ea6a1880fe96) )
	ROM_LOAD16_BYTE( "cs-q6.bin", 0xe00001, 0x080000, CRC(791d9d39) SHA1(44f3dcbfe8523118d52785844e103a480e8e13b5) )

	ROM_REGION16_LE( 0x200000, "user1", 0 )
	ROM_LOAD16_BYTE( "cs-ag1.bin", 0x000000, 0x100000, CRC(7ce449cc) SHA1(408e1405c80e623ee120cea65760ca9a8554cc29) )
	ROM_LOAD16_BYTE( "cs-ag2.bin", 0x000001, 0x100000, CRC(7026df0c) SHA1(a000d72c06ad37879673324880fb0e715f55788e) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "cs-sound.bin", 0x000000, 0x80000, CRC(96ea4e9f) SHA1(a5443d893f38f3e279f2eb9f4500547e7b8efa37) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1995, cluedo,   0,      jpmimpct, cluedo,   0, ROT0, "JPM", "Cluedo (prod. 2D)",           GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1995, cluedo2c, cluedo, jpmimpct, cluedo,   0, ROT0, "JPM", "Cluedo (prod. 2C)",           GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1995, cluedo2,  cluedo, jpmimpct, cluedo,   0, ROT0, "JPM", "Cluedo (prod. 2)",       	 GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1996, trivialp, 0,      jpmimpct, trivialp, 0, ROT0, "JPM", "Trivial Pursuit (prod. 1D)",  GAME_SUPPORTS_SAVE )
GAME( 1997, scrabble, 0,      jpmimpct, scrabble, 0, ROT0, "JPM", "Scrabble (rev. F)",           GAME_SUPPORTS_SAVE )
GAME( 1998, hngmnjpm, 0,      jpmimpct, hngmnjpm, 0, ROT0, "JPM", "Hangman (JPM)",               GAME_SUPPORTS_SAVE )
GAME( 1999, coronatn, 0,      jpmimpct, coronatn, 0, ROT0, "JPM", "Coronation Street Quiz Game", GAME_SUPPORTS_SAVE )

/**************************************************************************

    Mechanical IMPACT Games
        AGEMAME driver

    This is mostly based on old documentation, and needs a proper look.
***************************************************************************/

#include "video/awpvid.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/8255ppi.h"

/*************************************
 *
 *  Statics
 *
 *************************************/

static int optic_pattern;
static int payen;

/*************************************
 *
 *  Initialisation
 *
 *************************************/

static READ8_DEVICE_HANDLER( hopper_b_r )
{
	//?
	return 0;
}

static READ8_DEVICE_HANDLER( hopper_c_r )
{
	//?
	return 0;
}

static WRITE8_DEVICE_HANDLER( payen_a_w )
{
	if ( data )
	{
		if ( data & 0x10 )
		payen = 1;
		else
		payen = 0;
    }
}

static WRITE8_DEVICE_HANDLER( display_c_w )
{
	//?
}

static const ppi8255_interface ppi8255_intf[1] =
{
	{
		DEVCB_NULL,
		DEVCB_HANDLER(hopper_b_r),
		DEVCB_HANDLER(hopper_c_r),
		DEVCB_HANDLER(payen_a_w),
		DEVCB_NULL,
		DEVCB_HANDLER(display_c_w)
	}
};

static MACHINE_START( impctawp )
{
	state_save_register_global(machine, duart_1_irq);
	state_save_register_global(machine, touch_cnt);
	state_save_register_global_array(machine, touch_data);

	/* TODO! */
	state_save_register_global(machine, duart_1.ISR);
	state_save_register_global(machine, duart_1.IMR);
	state_save_register_global(machine, duart_1.CT);

	stepper_config(machine, 0, &starpoint_interface_48step);
	stepper_config(machine, 1, &starpoint_interface_48step);
	stepper_config(machine, 2, &starpoint_interface_48step);
	stepper_config(machine, 3, &starpoint_interface_48step);
	stepper_config(machine, 4, &starpoint_interface_48step);
	stepper_config(machine, 5, &starpoint_interface_48step);
	stepper_config(machine, 6, &starpoint_interface_48step);
}

static MACHINE_RESET( impctawp )
{
	memset(&duart_1, 0, sizeof(duart_1));

	/* Reset states */
	duart_1_irq = 0;

	ROC10937_init(0, MSC1937,0);
	ROC10937_reset(0);	/* reset display1 */
}
/*************************************
 *
 *  I/O handlers
 *
 *************************************/

/*
 *  0: DIP switches
 *  1: Percentage key
 *  2: Lamps + switches (J10)
 *  3: Lamps + switches (J10)
 *  4: Lamps + switches (J10)
 *      ---- ---x   Back door
 *      ---- --x-   Cash door
 *      ---- -x--   Refill key
 *  5: Lamps + switches (J9)
 *  6: Lamps + switches (J9)
 *  7: Lamps + switches (J9)
 *  8: Payslides
 *  9: Coin mechanism
 */
static READ16_HANDLER( optos_r )
{
	int i;

	for (i=0; i<6; i++)
	{
		if ( stepper_optic_state(i) ) optic_pattern |= (1 << i);
		else                          optic_pattern &= ~(1 << i);
	}
	return optic_pattern;
}

/*************************************
 *
 *  Mysterious stuff
 *
 *************************************/

static WRITE16_HANDLER( jpmioawp_w )
{
	int i;
	UINT64 cycles  = space->machine->firstcpu->total_cycles();
	switch (offset)
	{
		case 0x00:
		{
			output_set_value("PWRLED",!(data&0x100));
			output_set_value("STATLED",!(data&0x200));
			break;
		}


		case 0x02:
		{
			for (i=0; i<4; i++)
			{
				stepper_update(i, (data >> i)& 0x0F );
			}
			break;
		}
		case 0x04:
		{
			for (i=0; i<2; i++)
			{
				stepper_update(i, (data >> (i + 4)& 0x0F ));
			}
			break;
		}
		case 0x06:
		{
			if ( data & 0x10 )
			{   // PAYEN ?
				if ( data & 0xf )
				{
			//      slide = 1;
				}
				else
				{
				//  slide = 0;
				}
			}
			else
//          slide = 0;
			Mechmtr_update(0, cycles, data >> 10);
			if ( data )
			{
				duart_1.IP &= ~0x10;
			}
			else
			{
				duart_1.IP |= 0x10;
			}
			break;
		}

		case 0x08:
		{
			jpm_draw_lamps(data, lamp_strobe);
			break;
		}

		case 0x0b:
		{
			output_set_digit_value(lamp_strobe,data);
			break;
		}
		case 0x0f:
		{
			if (data & 0x10)
			{
				lamp_strobe = (data & 0x0f);
			}
			break;
		}
	}
}

static READ16_HANDLER( ump_r )
{
	return 0xffff;
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/
static ADDRESS_MAP_START( awp68k_program_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM
	AM_RANGE(0x00100000, 0x001fffff) AM_ROM
	AM_RANGE(0x00400000, 0x00403fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x00480000, 0x0048001f) AM_READWRITE(duart_1_r, duart_1_w)
	AM_RANGE(0x00480020, 0x00480033) AM_READ(inputs1_r)
	AM_RANGE(0x00480034, 0x00480035) AM_READ(ump_r)
	AM_RANGE(0x00480040, 0x00480041) AM_READ(optos_r)
	AM_RANGE(0x00480060, 0x00480067) AM_DEVREADWRITE8("ppi8255_0", ppi8255_r, ppi8255_w, 0xff00)
	AM_RANGE(0x004800a0, 0x004800af) AM_READWRITE(jpmio_r, jpmioawp_w)
	AM_RANGE(0x004800e0, 0x004800e1) AM_WRITE(unk_w)
	AM_RANGE(0x004801dc, 0x004801dd) AM_READ(unk_r)
	AM_RANGE(0x004801de, 0x004801df) AM_READ(unk_r)
	AM_RANGE(0x00480080, 0x00480081) AM_DEVWRITE("upd", upd7759_w)
	AM_RANGE(0x00480082, 0x00480083) AM_DEVWRITE("upd",volume_w)
	AM_RANGE(0x00480084, 0x00480085) AM_DEVREAD("upd", upd7759_r)
//  AM_RANGE(0x004801e0, 0x004801ff) AM_READWRITE(duart_2_r, duart_2_w)
//  AM_RANGE(0x00800000, 0x00800007) AM_READWRITE(m68k_tms_r, m68k_tms_w)
	AM_RANGE(0x00c00000, 0x00cfffff) AM_ROM
	AM_RANGE(0x00d00000, 0x00dfffff) AM_ROM
	AM_RANGE(0x00e00000, 0x00efffff) AM_ROM
	AM_RANGE(0x00f00000, 0x00ffffff) AM_ROM
ADDRESS_MAP_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( impctawp )
	MDRV_CPU_ADD("maincpu",M68000, 8000000)
	MDRV_CPU_PROGRAM_MAP(awp68k_program_map)

	MDRV_QUANTUM_TIME(HZ(30000))

	MDRV_MACHINE_START(impctawp)
	MDRV_MACHINE_RESET(impctawp)
	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_PPI8255_ADD( "ppi8255_0", ppi8255_intf[0] )

	MDRV_TIMER_ADD( "duart_1_timer", duart_1_timer_event)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MDRV_DEFAULT_LAYOUT(layout_awpvid16)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( m_tbirds )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "thbi-3_1.bin", 0x000000, 0x020000, CRC(ecbdd85c) SHA1(626306a72d658af6ecfa4b65212cf8aa35630539) )
	ROM_LOAD16_BYTE( "thbi-3_2.bin", 0x000001, 0x020000, CRC(61e98f71) SHA1(315044637bde8ab862af08bf9825917c87fcc77d) )

	ROM_FILL(						 0x040000, 0x100000, 0xff )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "tbcl-snd.bin", 0x000000, 0x80000, CRC(1cc197be) SHA1(2247aa1a0e6aab7389b3222f373890f54e907361) )
ROM_END

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 199?, m_tbirds, 0,      impctawp, trivialp, 0, ROT0, "JPM", "Thunderbirds", GAME_NOT_WORKING )
