/***************************************************************************

    JPM IMPACT (aka System 6)
     and
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

    Mechanical games note:

    Anything writing to 4800a0 within the first few instructions is guessed
    to be an IMPACT game, some things could be misplaced, some could be
    missing video roms, many are missing sound roms (or they're in the wrong
    sets)

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
    480060-480067   R/W   -------- xxxxxxxx   uPD71055C (NEC clone of 8255 PPI)
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
#include "machine/nvram.h"

/*************************************
 *
 *  MC68681 DUART (TODO)
 *
 *************************************/

#define MC68681_1_CLOCK		3686400
#define MC68681_2_CLOCK		3686400


/*************************************
 *
 *  68000 IRQ handling
 *
 *************************************/

static void update_irqs(running_machine &machine)
{
	jpmimpct_state *state = machine.driver_data<jpmimpct_state>();
	cputag_set_input_line(machine, "maincpu", 2, state->m_tms_irq ? ASSERT_LINE : CLEAR_LINE);
	cputag_set_input_line(machine, "maincpu", 5, state->m_duart_1_irq ? ASSERT_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static MACHINE_START( jpmimpct )
{
	jpmimpct_state *state = machine.driver_data<jpmimpct_state>();
	state_save_register_global(machine, state->m_tms_irq);
	state_save_register_global(machine, state->m_duart_1_irq);
	state_save_register_global(machine, state->m_touch_cnt);
	state_save_register_global_array(machine, state->m_touch_data);

	/* TODO! */
	state_save_register_global(machine, state->m_duart_1.ISR);
	state_save_register_global(machine, state->m_duart_1.IMR);
	state_save_register_global(machine, state->m_duart_1.CT);
}


static MACHINE_RESET( jpmimpct )
{
	jpmimpct_state *state = machine.driver_data<jpmimpct_state>();
	memset(&state->m_duart_1, 0, sizeof(state->m_duart_1));

	/* Reset states */
	state->m_duart_1_irq = state->m_tms_irq = 0;
	state->m_touch_cnt = 0;

//  state->m_duart_1.IVR=0x0f;
}


/*************************************
 *
 *  TMS34010 host interface
 *
 *************************************/

static WRITE16_HANDLER( m68k_tms_w )
{
	tms34010_host_w(space->machine().device("dsp"), offset, data);
}

static READ16_HANDLER( m68k_tms_r )
{
	return tms34010_host_r(space->machine().device("dsp"), offset);
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
	jpmimpct_state *state = timer.machine().driver_data<jpmimpct_state>();
	state->m_duart_1.tc = 0;
	state->m_duart_1.ISR |= 0x08;

	state->m_duart_1_irq = 1;
	update_irqs(timer.machine());
}

static READ16_HANDLER( duart_1_r )
{
	jpmimpct_state *state = space->machine().driver_data<jpmimpct_state>();
	struct duart_t &duart_1 = state->m_duart_1;
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
			val = input_port_read(space->machine(), "TEST/DEMO");
			break;
		}
		case 0xe:
		{
			attotime rate = attotime::from_hz(MC68681_1_CLOCK) * (16 * duart_1.CT);
			timer_device *duart_timer = space->machine().device<timer_device>("duart_1_timer");
			duart_timer->adjust(rate, 0, rate);
			break;
		}
		case 0xf:
		{
			state->m_duart_1_irq = 0;
			update_irqs(space->machine());
			duart_1.ISR |= ~0x8;
			break;
		}
	}

	return val;
}

static WRITE16_HANDLER( duart_1_w )
{
	jpmimpct_state *state = space->machine().driver_data<jpmimpct_state>();
	struct duart_t &duart_1 = state->m_duart_1;
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
	jpmimpct_state *state = space->machine().driver_data<jpmimpct_state>();
	switch (offset)
	{
		case 0x9:
		{
			if (state->m_touch_cnt == 0)
			{
				if ( input_port_read(space->machine(), "TOUCH") & 0x1 )
				{
					state->m_touch_data[0] = 0x2a;
					state->m_touch_data[1] = 0x7 - (input_port_read(space->machine(), "TOUCH_Y") >> 5) + 0x30;
					state->m_touch_data[2] = (input_port_read(space->machine(), "TOUCH_X") >> 5) + 0x30;

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
			UINT16 val = state->m_touch_data[state->m_touch_cnt];

			if (state->m_touch_cnt++ == 3)
				state->m_touch_cnt = 0;

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
			val = input_port_read(space->machine(), "DSW");
			break;
		}
		case 2:
		{
			val = input_port_read(space->machine(), "SW2");
			break;
		}
		case 4:
		{
			val = input_port_read(space->machine(), "SW1");
			break;
		}
		case 9:
		{
			val = input_port_read(space->machine(), "COINS");
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

static void jpm_draw_lamps(jpmimpct_state *state, int data, int lamp_strobe)
{
	int i;
	for (i=0; i<16; i++)
	{
		state->m_Lamps[16*(state->m_lamp_strobe+i)] = data & 1;
		output_set_lamp_value((16*lamp_strobe)+i, (state->m_Lamps[(16*lamp_strobe)+i]));
		data = data >> 1;
	}
}

static READ16_HANDLER( jpmio_r )
{
	return 0xffff;
}

static WRITE16_HANDLER( jpmio_w )
{
	jpmimpct_state *state = space->machine().driver_data<jpmimpct_state>();
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
			MechMtr_update(0, data >> 10);
			state->m_duart_1.IP &= ~0x10;
			break;
		}

		case 0x08:
		{
			jpm_draw_lamps(state, data, state->m_lamp_strobe);
			break;
		}

		case 0x0b:
		{
			output_set_digit_value(state->m_lamp_strobe,data);
			break;
		}
		case 0x0f:
		{
			if (data & 0x10)
			{
				state->m_lamp_strobe = (data +1) & 0x0f;
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
static ADDRESS_MAP_START( m68k_program_map, AS_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM
	AM_RANGE(0x00100000, 0x001fffff) AM_ROM
	AM_RANGE(0x00400000, 0x00403fff) AM_RAM AM_SHARE("nvram")
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

static ADDRESS_MAP_START( tms_program_map, AS_PROGRAM, 16 )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_READWRITE(tms34010_io_register_r, tms34010_io_register_w)
	AM_RANGE(0x00000000, 0x003fffff) AM_MIRROR(0xf8000000) AM_RAM AM_BASE_MEMBER(jpmimpct_state, m_vram)
	AM_RANGE(0x00800000, 0x00ffffff) AM_MIRROR(0xf8000000) AM_ROM AM_REGION("user1", 0x100000)
	AM_RANGE(0x02000000, 0x027fffff) AM_MIRROR(0xf8000000) AM_ROM AM_REGION("user1", 0)
//  AM_RANGE(0x01000000, 0x0100003f) AM_MIRROR(0xf87fffc0) AM_READWRITE(jpmimpct_bt477_r, jpmimpct_bt477_w)
	AM_RANGE(0x01000000, 0x017fffff) AM_MIRROR(0xf8000000) AM_MASK(0x1f) AM_READWRITE(jpmimpct_bt477_r, jpmimpct_bt477_w)
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

static void jpmimpct_tms_irq(device_t *device, int state)
{
	jpmimpct_state *drvstate = device->machine().driver_data<jpmimpct_state>();
	drvstate->m_tms_irq = state;
	update_irqs(device->machine());
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

static MACHINE_CONFIG_START( jpmimpct, jpmimpct_state )
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(m68k_program_map)

	MCFG_CPU_ADD("dsp", TMS34010, 40000000)
	MCFG_CPU_CONFIG(tms_config)
	MCFG_CPU_PROGRAM_MAP(tms_program_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(30000))
	MCFG_MACHINE_START(jpmimpct)
	MCFG_MACHINE_RESET(jpmimpct)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TIMER_ADD( "duart_1_timer", duart_1_timer_event)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MCFG_SCREEN_RAW_PARAMS(40000000/4, 156*4, 0, 100*4, 328, 0, 300)
	MCFG_SCREEN_UPDATE(tms340x0)
	MCFG_PALETTE_LENGTH(256)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_VIDEO_START(jpmimpct)
MACHINE_CONFIG_END


/**************************************************************************

Mechanical IMPACT Games

IMPACT apparently stands for Interactive Moving Picture Amusment Control
Technology, and is intended as a replacement for the JPM System 5 board.
Large sections of the processing were moved to two identical custom ASICs
(U1 and U2), only half of each is used.

Thanks to Tony Friery and JPeMU for I/O routines and documentation.

***************************************************************************/

#include "video/awpvid.h"
#include "machine/steppers.h"
#include "machine/roc10937.h"
#include "machine/i8255.h"

/*************************************
 *
 *  Initialisation
 *
 *************************************/

static READ8_DEVICE_HANDLER( hopper_b_r )
{
	jpmimpct_state *state = device->machine().driver_data<jpmimpct_state>();

	int retval;
	// B0 = 100p Hopper Out Verif
	// B1 = Hopper High
	// B2 = Hopper Low
	// B3 = 20p Hopper Opto

	// Always return hoppers full
   retval=0xed; // 1110 1101

   if (!state->m_hopinhibit)//if inhibited, we don't change these flags
   {
		if (state->m_hopper[0] && state->m_motor[0]) //&& ((state->m_hopflag1 & 0x20)==0x20))
		{//100p
			retval &= ~0x01;
		}
		if (((state->m_hopper[1] && state->m_motor[1]) || (state->m_hopper[2] && state->m_slidesout))) //&& ((state->m_hopflag2 & 0x20)==0x20))
		{
			retval &= ~0x08;
		}
   }

   return retval;
}

static READ8_DEVICE_HANDLER( hopper_c_r )
{
	jpmimpct_state *state = device->machine().driver_data<jpmimpct_state>();

	int retval;
   // C0-C2 = Alpha
   // C3
   // C4 = 20p Hopper Detect
   // C5 = Hopper Top-Up
   // C6 = 100p Hopper Detect
   // C7 = Payout Verif (Slides)

   retval=0xf0; //1111 0000

//    if (StatBtns & 0x20) // Top Up switch
//    retval &= ~0x20;

	// Which hoppers are present
	if (state->m_hopper[0])
	{
		retval &= ~0x40;
	}
	if (state->m_hopper[1])
	{
		retval &= ~0x10;
	}

	if (!state->m_hopinhibit)
	{
		if ((state->m_slidesout==1) && ((state->m_hopper[2]==0)))
		{
			state->m_slidesout=0;
			retval &= ~0x80;
		}
	}

	return retval;
}

static WRITE8_DEVICE_HANDLER( payen_a_w )
{
	jpmimpct_state *state = device->machine().driver_data<jpmimpct_state>();

	state->m_motor[0] = (data & 0x01);
	state->m_payen = (data & 0x10);
	state->m_slidesout = (data & 0x10);
	state->m_motor[1] = (data & 0x40);
	state->m_hopinhibit = (data & 0x80);
}

static WRITE8_DEVICE_HANDLER( display_c_w )
{
	jpmimpct_state *state = device->machine().driver_data<jpmimpct_state>();

	if(data & 0x04)
	{
		state->m_alpha_data_line = ((data >> 1) & 1);
		if (state->m_alpha_clock != (data & 1))
		{
			if (!state->m_alpha_clock)//falling edge
			{
				ROC10937_shift_data(0, state->m_alpha_data_line?0:1);
			}
		}
		state->m_alpha_clock = (data & 1);
	}
	else
	{
		ROC10937_reset(0);
	}
		ROC10937_draw_16seg(0);
	//?
}

static I8255_INTERFACE (ppi8255_intf)
{
	DEVCB_NULL,
	DEVCB_HANDLER(payen_a_w),
	DEVCB_HANDLER(hopper_b_r),
	DEVCB_NULL,
	DEVCB_HANDLER(hopper_c_r),
	DEVCB_HANDLER(display_c_w)
};

static MACHINE_START( impctawp )
{
	jpmimpct_state *state = machine.driver_data<jpmimpct_state>();
	state_save_register_global(machine, state->m_duart_1_irq);
	state_save_register_global(machine, state->m_touch_cnt);
	state_save_register_global_array(machine, state->m_touch_data);

	/* TODO! */
	state_save_register_global(machine, state->m_duart_1.ISR);
	state_save_register_global(machine, state->m_duart_1.IMR);
	state_save_register_global(machine, state->m_duart_1.CT);

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
	jpmimpct_state *state = machine.driver_data<jpmimpct_state>();
	memset(&state->m_duart_1, 0, sizeof(state->m_duart_1));

	/* Reset states */
	state->m_duart_1_irq = 0;

	ROC10937_init(0, MSC1937,1);//Reversed
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
static READ16_HANDLER( inputs1awp_r )
{
	UINT16 val = 0x00;

	{
		switch (offset)
		{
			case 0:
			{
				val = input_port_read(space->machine(), "DSW");
				break;
			}
			case 1:
			{
				val = input_port_read(space->machine(), "PERCENT");
				break;
			}
			case 2:
			{
				val = input_port_read(space->machine(), "KEYS");
				break;
			}
			case 3:
			{
				val = input_port_read(space->machine(), "SW2");
				break;
			}
			case 4:
			{
				val = input_port_read(space->machine(), "SW1");
				break;
			}
			case 5:
			{
				val = (input_port_read(space->machine(), "SW3") );
				break;
			}
			case 6:
			{
				val = (input_port_read(space->machine(), "SW4") );
				break;
			}
			case 7://5
			{
				val = (input_port_read(space->machine(), "SW5") );
				break;
			}
			case 9:
			{
				val = input_port_read(space->machine(), "COINS");
				break;
			}
		}
	return val & 0xff00;
	}
}

static READ16_HANDLER( optos_r )
{
	jpmimpct_state *state = space->machine().driver_data<jpmimpct_state>();
	int i;

	for (i=0; i<6; i++)
	{
		if ( stepper_optic_state(i) ) state->m_optic_pattern |= (1 << i);
		else                          state->m_optic_pattern &= ~(1 << i);
	}
	return state->m_optic_pattern;
}

static READ16_HANDLER( prot_1_r )
{
	return 0x01;
}

static READ16_HANDLER( prot_0_r )
{
	return 0x00;
}

static WRITE16_HANDLER( jpmioawp_w )
{
	jpmimpct_state *state = space->machine().driver_data<jpmimpct_state>();
	int i,metno;
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
				awp_draw_reel(i);
			}
			break;
		}
		case 0x04:
		{
			for (i=0; i<2; i++)
			{
				stepper_update(i+4, (data >> (i + 4)& 0x0F ));
				awp_draw_reel(i+4);
			}
			break;
		}
		case 0x06:
		{
			//Slides
			if ((data & 0xff)!=0x00)
			{
				state->m_slidesout=2;
			}
			if (((data & 0xff)==0x00) && (state->m_slidesout==2))
			{
				state->m_slidesout=1;
			}
	      // Meters
			metno=(data >>8) & 0xff;
			{
				switch (metno)
				{
					case 0x00:
					{
						for (i=0; i<5; i++)
						{
							MechMtr_update(i, 0);
						}
						break;
					}
					default:
					{
						MechMtr_update(((metno <<2) - 1), 1);
					}
					break;
				}
			}
			int combined_meter = MechMtr_GetActivity(0) | MechMtr_GetActivity(1) |
			MechMtr_GetActivity(2) | MechMtr_GetActivity(3) |
			MechMtr_GetActivity(4);

			if(combined_meter)
			{
				state->m_duart_1.IP &= ~0x10;
			}
			else
			{
				state->m_duart_1.IP |= 0x10;
			}
			break;
		}

		case 0x08:
		{
			jpm_draw_lamps(state, data, state->m_lamp_strobe);
			break;
		}

		case 0x0b:
		{
			output_set_digit_value(state->m_lamp_strobe,data);
			break;
		}
		case 0x0f:
		{
			if (data & 0x10)
			{
				state->m_lamp_strobe = (data & 0x0f);
			}
			break;
		}
	}
}

static READ16_HANDLER( ump_r )
{
	return 0xff;//0xffff;
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/
static ADDRESS_MAP_START( awp68k_program_map, AS_PROGRAM, 16 )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM
	AM_RANGE(0x00100000, 0x001fffff) AM_ROM
	AM_RANGE(0x00400000, 0x00403fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x00480000, 0x0048001f) AM_READWRITE(duart_1_r, duart_1_w)
	AM_RANGE(0x00480020, 0x00480033) AM_READ(inputs1awp_r)
	AM_RANGE(0x00480034, 0x00480035) AM_READ(ump_r)
	AM_RANGE(0x00480040, 0x00480041) AM_READ(optos_r)
	AM_RANGE(0x00480060, 0x00480067) AM_DEVREADWRITE8_MODERN("ppi8255", i8255_device, read, write,0x00ff)
	AM_RANGE(0x00480080, 0x00480081) AM_DEVWRITE("upd", upd7759_w)
	AM_RANGE(0x00480082, 0x00480083) AM_DEVWRITE("upd",volume_w)
	AM_RANGE(0x00480084, 0x00480085) AM_DEVREAD("upd", upd7759_r)
	AM_RANGE(0x00480086, 0x0048009f) AM_READ(prot_1_r)
	AM_RANGE(0x004800a0, 0x004800af) AM_READWRITE(jpmio_r, jpmioawp_w)
//  AM_RANGE(0x004800b0, 0x004800df) AM_READ(prot_1_r)
//  AM_RANGE(0x004800e0, 0x004800e1) AM_WRITE(unk_w)
//  AM_RANGE(0x00480086, 0x006576ff) AM_READ(prot_1_r)
	AM_RANGE(0x004801dc, 0x004801dd) AM_READ(prot_1_r)
	AM_RANGE(0x004801de, 0x006575ff) AM_READ(prot_1_r)
	AM_RANGE(0x00657600, 0x00657601) AM_READ(prot_0_r)
	AM_RANGE(0x00657602, 0x00ffffff) AM_READ(prot_1_r)

//  AM_RANGE(0x004801dc, 0x004801dd) AM_READ(unk_r)
//  AM_RANGE(0x004801de, 0x004801df) AM_READ(unk_r)
	//AM_RANGE(0x00657602, 0x00bfffff) AM_READ(prot_1_r)
//  AM_RANGE(0x004801e0, 0x004801ff) AM_READWRITE(duart_2_r, duart_2_w)
//  AM_RANGE(0x00c00000, 0x00cfffff) AM_ROM
//  AM_RANGE(0x00d00000, 0x00dfffff) AM_ROM
//  AM_RANGE(0x00e00000, 0x00efffff) AM_ROM
//  AM_RANGE(0x00f00000, 0x00ffffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( tbirds )
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

	PORT_START("PERCENT")
	PORT_CONFNAME( 0x0F, 0x00, "Percentage Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 68% (Invalid for UK Games)"  )
	PORT_CONFSETTING(    0x01, "70" )
	PORT_CONFSETTING(    0x02, "72" )
	PORT_CONFSETTING(    0x03, "74" )
	PORT_CONFSETTING(    0x04, "76" )
	PORT_CONFSETTING(    0x05, "78" )
	PORT_CONFSETTING(    0x06, "80" )
	PORT_CONFSETTING(    0x07, "82" )
	PORT_CONFSETTING(    0x08, "84" )
	PORT_CONFSETTING(    0x09, "86" )
	PORT_CONFSETTING(    0x0A, "88" )
	PORT_CONFSETTING(    0x0B, "90" )
	PORT_CONFSETTING(    0x0C, "92" )
	PORT_CONFSETTING(    0x0D, "94" )
	PORT_CONFSETTING(    0x0E, "96" )
	PORT_CONFSETTING(    0x0F, "98" )

	PORT_START("KEYS")
	PORT_CONFNAME( 0x0F, 0x0F, "Jackpot / Prize Key" )
	PORT_CONFSETTING(    0x0F, "Not fitted"  )
	PORT_CONFSETTING(    0x0E, "3 GBP"  )
	PORT_CONFSETTING(    0x0D, "4 GBP"  )
	PORT_CONFSETTING(    0x0C, "5 GBP"  )
	PORT_CONFSETTING(    0x0B, "6 GBP"  )
	PORT_CONFSETTING(    0x0A, "6 GBP Token"  )
	PORT_CONFSETTING(    0x09, "8 GBP"  )
	PORT_CONFSETTING(    0x08, "8 GBP Token"  )
	PORT_CONFSETTING(    0x07, "10 GBP"  )
	PORT_CONFSETTING(    0x06, "15 GBP"  )
	PORT_CONFSETTING(    0x05, "25 GBP"  )
	PORT_CONFSETTING(    0x04, "25 GBP (Licensed Betting Office Profile)"  )
	PORT_CONFSETTING(    0x03, "35 GBP"  )
	PORT_CONFSETTING(    0x02, "70 GBP"  )
	PORT_CONFSETTING(    0x01, "Reserved"  )
	PORT_CONFSETTING(    0x00, "Reserved"  )

	PORT_CONFNAME( 0xF0, 0x00, "Stake Key" )
	PORT_CONFSETTING(    0x00, "Not fitted / 5p"  )
	PORT_CONFSETTING(    0x80, "10p" )
	PORT_CONFSETTING(    0x40, "20p" )
	PORT_CONFSETTING(    0xC0, "25p" )
	PORT_CONFSETTING(    0x20, "30p" )
//  PORT_CONFSETTING(    0x20, "40p" )
	PORT_CONFSETTING(    0x60, "50p" )
	PORT_CONFSETTING(    0xE0, "1 GBP" )

	PORT_START("SW5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "'3'" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "'2'" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "'1'" )

	PORT_START("SW4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "'3'" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "'2'" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "'1'" )

	PORT_START("SW3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "'3'" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "'2'" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "'1'" )

	PORT_START("SW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Collect" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "'3'" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "'2'" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "'1'" )

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_TOGGLE PORT_NAME( "Back Door" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_TOGGLE PORT_NAME( "Cash Door" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_TOGGLE PORT_NAME( "Refill Key" )

	PORT_START("TEST/DEMO")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE4 ) PORT_NAME( "Test/Demo" )

	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 1 pound" )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 50p" )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 20p" )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 10p" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_IMPULSE(1) PORT_NAME( "Token: 20" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 ) PORT_IMPULSE(1) PORT_NAME( "Coin: 5p" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( impctawp, jpmimpct_state )
	MCFG_CPU_ADD("maincpu",M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(awp68k_program_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(30000))

	MCFG_MACHINE_START(impctawp)
	MCFG_MACHINE_RESET(impctawp)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_I8255_ADD( "ppi8255", ppi8255_intf )
	MCFG_TIMER_ADD( "duart_1_timer", duart_1_timer_event)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_DEFAULT_LAYOUT(layout_awpvid16)
MACHINE_CONFIG_END

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

ROM_START( cluedod )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7326.bin", 0x000000, 0x080000, CRC(6c6b523e) SHA1(3a140aff92c00da45433698c3c946fc0134b4863) )
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

ROM_START( trivialpd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1426.bin", 0x000000, 0x080000, CRC(36c84b55) SHA1(c01dc797bd578dfe5979f39a6acfdb3c5744b298) ) //was labelled 1424, typo?
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

ROM_START( trivialpo )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	// set only contained these 2 roms.. others are assumed to be the same for now
	ROM_LOAD16_BYTE( "tpswpp1", 0x000000, 0x080000, CRC(9d3cb9b7) SHA1(959cc0e2254aa3a3a4e9f5814ca6ee2b0e486fb3) )
	ROM_LOAD16_BYTE( "tpswpp2", 0x000001, 0x080000, CRC(4a2f1476) SHA1(c08a5c99b44ee3e5457cb26a29405b2f01fd5a27) )
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

ROM_START( scrabbled )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "1564.bin", 0x000000, 0x080000, CRC(bfc1b98b) SHA1(09278f06efa18c1578f61e9b1bfed0f4f6657cb6) )
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
	ROM_LOAD( "s60-3.bin", 0x000000, 0x0117, CRC(19e1d28b) SHA1(12dff4bea16b95807f1a9455b6785468ca5de858) )
	ROM_LOAD( "s61-6.bin", 0x000000, 0x0117, CRC(c72cec0e) SHA1(9d6e5510600987f9359af9ecc3e95f5bd8444bcd) )
	ROM_LOAD( "ig1.1.bin", 0x000000, 0x02DD, CRC(4e11fa4e) SHA1(ded2d2086c4360708462024054e5409962ea8589) )
	ROM_LOAD( "ig2.1.bin", 0x000000, 0x0157, CRC(2365878b) SHA1(d91d9906aadcfd8cff7ee6b92449c522f73a29e1) )
	ROM_LOAD( "ig3.2.bin", 0x000000, 0x0117, CRC(4970dad7) SHA1(c5931db3d66c7d1027a762be10f9e3d9e321b70f) )
	ROM_LOAD( "jpms6.bin", 0x000000, 0x0117, CRC(1fba3b6f) SHA1(0e33e49cbf24e836deb1ef16385ff20549ef188e) )
	ROM_LOAD( "mem-2.bin", 0x000000, 0x0157, CRC(92832445) SHA1(b6edcc6d4f721f0e91e9fcf322163db017afaee1) )
ROM_END

ROM_START( hngmnjpmd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20266.bin", 0x000000, 0x080000, CRC(38f6c73b) SHA1(71bdeee0656686bd420d9cf1928a8118372c57e4) )
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
	ROM_LOAD( "s60-3.bin", 0x000000, 0x0117, CRC(19e1d28b) SHA1(12dff4bea16b95807f1a9455b6785468ca5de858) )
	ROM_LOAD( "s61-6.bin", 0x000000, 0x0117, CRC(c72cec0e) SHA1(9d6e5510600987f9359af9ecc3e95f5bd8444bcd) )
	ROM_LOAD( "ig1.1.bin", 0x000000, 0x02DD, CRC(4e11fa4e) SHA1(ded2d2086c4360708462024054e5409962ea8589) )
	ROM_LOAD( "ig2.1.bin", 0x000000, 0x0157, CRC(2365878b) SHA1(d91d9906aadcfd8cff7ee6b92449c522f73a29e1) )
	ROM_LOAD( "ig3.2.bin", 0x000000, 0x0117, CRC(4970dad7) SHA1(c5931db3d66c7d1027a762be10f9e3d9e321b70f) )
	ROM_LOAD( "jpms6.bin", 0x000000, 0x0117, CRC(1fba3b6f) SHA1(0e33e49cbf24e836deb1ef16385ff20549ef188e) )
	ROM_LOAD( "mem-2.bin", 0x000000, 0x0157, CRC(92832445) SHA1(b6edcc6d4f721f0e91e9fcf322163db017afaee1) )
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

ROM_START( coronatnd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20826.bin", 0x000000, 0x080000, CRC(9d3d85d8) SHA1(a6ab622fac9ece04f9b255e10eac7812549afb8a) )
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

ROM_START( tqst ) // this looks like a video game.. but probably incomplete
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prom1n.bin",0x000000, 0x080000, CRC(a9cacb88) SHA1(2cc565e8083926acab8c8b14ad90bd50f7597038) )
	ROM_LOAD16_BYTE( "prom2.bin", 0x000001, 0x080000, CRC(a665e72e) SHA1(76440ae69f61eac1c6fe59dae295826a145bc940) )
	ROM_LOAD16_BYTE( "u16.bin",   0x100000, 0x080000, CRC(ae9b6829) SHA1(2c8ed5060d751bca0af54305164512fae8ff88e9) )
	ROM_LOAD16_BYTE( "u17.bin",   0x100001, 0x080000, CRC(7786340d) SHA1(96ded0af403fa3f0e7604f9ae0952036b3652665) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "prom1p.bin", 0x0000, 0x080000, CRC(c13b499b) SHA1(e8389568e5bec6462e02b69949691b14e29d7d8e) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "025rs1-0.bin", 0x0000, 0x080000, CRC(c4dbff24) SHA1(2e4d1d1905b9cd8254989d1653beb6756664839e) )
ROM_END

ROM_START( snlad ) // probably incomplete
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8718.bin", 0x00000, 0x080000, CRC(599ca023) SHA1(fe6792ac97d18e2a04dbe8700d9f16b95be0f486) )
	ROM_LOAD16_BYTE( "8719.bin", 0x00001, 0x080000, CRC(155156dc) SHA1(7f43d52413c31c5f44907ebb9eb419ccb8047c68) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8720.bin", 0x0000, 0x080000, CRC(316d2230) SHA1(f2e330bcbc55dc0a47571f10d8c31e0e272ef8a9) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gr1.bin", 0x000000, 0x100000, NO_DUMP )
	ROM_LOAD16_BYTE( "gr2.bin", 0x000001, 0x100000, NO_DUMP )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "slswpsnd.bin", 0x0000, 0x080000, CRC(9a48b772) SHA1(d8fbaa60f09a1d31cf6c61c6dd02ad1bd7b7ffc9) )
ROM_END








/* Mechanical Below */

// I believe all IMPACT roms should have a samples rom (it's the only sound output?) so any without are almost
// certainly missing it.

// 68k dies, mismatched set?
ROM_START( j6fifth )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fdm30dsk.1", 0x000000, 0x020000, CRC(ec6a2687) SHA1(4cafd1c8d6d20fb034493c16d3abfafa2a1906f5) )
	ROM_LOAD16_BYTE( "fdm30.2",    0x000001, 0x020000, CRC(b24995cc) SHA1(3ee23e0e8e805c077c2c60c463d9d0214c61fc2d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6aceclb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aocd1.bin", 0x000000, 0x020000, CRC(932fe07a) SHA1(4f4dbfa627364edf8408264e547d0f21d306c59e) )
	ROM_LOAD16_BYTE( "aocd2.bin", 0x000001, 0x020000, CRC(d18d8b24) SHA1(df09419c3702efb340da3e7327e1139c54351926) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "aceofclubssnd.bin", 0x000000, 0x080000, CRC(9ea8bce4) SHA1(0849be87528168dbc5c07a31138edcb30a611c5c) )
ROM_END

ROM_START( j6aceclba )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aocdp1.b8", 0x000000, 0x020000, CRC(78b1b442) SHA1(b2fe66e3ac5b7a2eb4b0dbcc6237259aef61abfd) )
	ROM_LOAD16_BYTE( "aocd2.bin", 0x000001, 0x020000, CRC(d18d8b24) SHA1(df09419c3702efb340da3e7327e1139c54351926) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "aceofclubssnd.bin", 0x000000, 0x080000, CRC(9ea8bce4) SHA1(0849be87528168dbc5c07a31138edcb30a611c5c) )
ROM_END




ROM_START( j6acehi )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "aceshigh.p1", 0x000000, 0x020000, CRC(13c48483) SHA1(43e6a9a00ee9c128700f763a18451abb5634e78f) )
	ROM_LOAD16_BYTE( "aceshigh.p2", 0x000001, 0x020000, CRC(21d2b908) SHA1(b6fc67d2d5f79e21be154d8f84c54f0e4a0b702c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "aceshighsnd.bin", 0x000000, 0x080000, CRC(d0d8ec93) SHA1(3731c64c194fe856f0bebe5b1b4430bd82ee905f) )
ROM_END


ROM_START( j6amdrm )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "americandreamgame1.bin", 0x000000, 0x020000, CRC(03006f4d) SHA1(f2da5d3bc1cb76380f8fc1182e48d3906f0c23b8) )
	ROM_LOAD16_BYTE( "americandreamgame2.bin", 0x000001, 0x020000, CRC(ae336948) SHA1(f620b6098a488c613fd2468f99b5b80347aaea9e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "americandreamsound.bin", 0x000000, 0x080000, CRC(8a23d027) SHA1(c475032411e28c4c889a3c112ca332ba222b93fe) )
ROM_END


ROM_START( j6arcade )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "arca-9_1.bin", 0x000000, 0x020000, CRC(09838350) SHA1(ffb37a229e20fd2f9690659bc4a2841269b5d918) )
	ROM_LOAD16_BYTE( "arca-9_2.bin", 0x000001, 0x020000, CRC(a2170a26) SHA1(ed23f9a4b197c1603014eff238c73564fca1bd0c) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "arca-9a1.bin", 0x000000, 0x020000, CRC(5b474331) SHA1(a8cfadab98ece97fb66fcf2e87e97be528ab5bf7) )
	ROM_LOAD16_BYTE( "arca-9n1.bin", 0x000000, 0x020000, CRC(38b53b94) SHA1(a28d7ac1eb594251d9534901ee38b797cd563dc7) )
	ROM_LOAD16_BYTE( "arca-9np.bin", 0x000000, 0x020000, CRC(d32b6fac) SHA1(1cb16c214da888adacba090d59f88694d1d9e25b) )
	ROM_LOAD16_BYTE( "arca-9p1.bin", 0x000000, 0x020000, CRC(e21dd768) SHA1(94132c7ba9b90ab6b52a56e60b68bf4e5d30e063) )
	ROM_LOAD16_BYTE( "arca10_1.bin", 0x000000, 0x020000, CRC(4c632e9e) SHA1(1e76e26941b164ba3a51c1c3caaa3b4d384a90d3) )
	ROM_LOAD16_BYTE( "arca10_2.bin", 0x000000, 0x020000, CRC(2175520c) SHA1(0a12506a72a93c8cd74f6666d41bacdfd4e72a54) )
	ROM_LOAD16_BYTE( "arcadia.p2",	 0x000000, 0x020000, CRC(1533ea6f) SHA1(0dff53bcee961781312eb108cd705664f772ce1d) ) //differs by 2 bytes from arca10_2.bin
	ROM_LOAD16_BYTE( "arca10a1.bin", 0x000000, 0x020000, CRC(1ea7eeff) SHA1(8b8a1b0543d53d95fd2fd44add1114c6ad48b6c7) )
	ROM_LOAD16_BYTE( "arca10n1.bin", 0x000000, 0x020000, CRC(7d55965a) SHA1(96ca11708bbba84ccbb0c43b6ddaca8c9285ffb8) )
	ROM_LOAD16_BYTE( "arca10np.bin", 0x000000, 0x020000, CRC(96cbc262) SHA1(0fdb783443240db94bf22a09f2177de958a2ecfe) )
	ROM_LOAD16_BYTE( "arca10p1.bin", 0x000000, 0x020000, CRC(a7fd7aa6) SHA1(9f083900ee50d1669f23eecc7d6d7779d2a27301) )
	ROM_LOAD( "arcadiaarca10-1.bin", 0x000000, 0x080000, CRC(998ae103) SHA1(cabbd7ec8dcaf5107de0c23a8d67680254acbe5d) )
	ROM_LOAD( "arcadiaarca10-2.bin", 0x000000, 0x080000, CRC(98c15e57) SHA1(3207c9760ca1b9f11b24253d6c974e4efeb0c46c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "arca-snd.bin", 0x000000, 0x080000, CRC(111c3c40) SHA1(9ce6da8101eb9c26c7ff5616ef24e7c119803777) )
ROM_END


ROM_START( j6bnkrcl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clba-6_1.bin", 0x000000, 0x020000, CRC(b0407200) SHA1(88792a7cc71be3830a0156fa10195bad3cc58066) )
	ROM_LOAD16_BYTE( "clba-6_2.bin", 0x000001, 0x020000, CRC(fd055187) SHA1(7634c1dfbfab66c13c2a0453dc70643f9b33edef) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "clba-6f1.bin", 0x000000, 0x020000, CRC(749a9110) SHA1(8607d6edd4d2f2d799d3a60ead779f9a80e93823) )
	ROM_LOAD16_BYTE( "clba-6p1.bin", 0x000000, 0x020000, CRC(5bde2638) SHA1(d7247d74acb030c986aecc58991c56207ae2ade9) )

	ROM_LOAD16_BYTE( "cl_banke.p1", 0x00000, 0x020000, CRC(4604d13d) SHA1(8e96b5ac7537fbe496178f26be0786049efcbc49) )
	ROM_LOAD16_BYTE( "cl_banke.p2", 0x00001, 0x020000, CRC(6aea1ffd) SHA1(cf9356106f5e64ebfde6d98314afad3a5377eaf4) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cbsnd.bin", 0x000000, 0x080000, CRC(e7e587c9) SHA1(fde7a7761253dc4133340b766d220873731c11c7) )
ROM_END


ROM_START( j6big50 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9347.bin", 0x000000, 0x020000, CRC(2d05fc2b) SHA1(c5c3cf89b5d75876eecc9f9acf0426b58bacbd79) )
	ROM_LOAD16_BYTE( "9348.bin", 0x000001, 0x020000, CRC(d131967e) SHA1(9028c3d55d365de9e90ea4a2af29aaace0d8e588) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9349.bin", 0x000000, 0x020000, CRC(c69ba813) SHA1(6b12aac1ed5bfdb71cc6ece67162fc890ec924a6) )
	ROM_LOAD16_BYTE( "9350.bin", 0x000000, 0x020000, CRC(7fc13c4a) SHA1(a260914f04ea05ac9e9fbf53d914f54a3e84b3ee) )
	ROM_LOAD16_BYTE( "9351.bin", 0x000000, 0x020000, CRC(4f688da3) SHA1(0abb33b3783a73ddd5f1677bf84075682da09bcf) )
	ROM_LOAD16_BYTE( "bg50v8p1", 0x000000, 0x020000, CRC(a28eadba) SHA1(fb7bb3305324a80454c1b4fa053c185d721e387e) )
	ROM_LOAD16_BYTE( "bg50v8p2", 0x000000, 0x020000, CRC(6724f61d) SHA1(9ccd7940c934f1edbba7c7defc87f8ac12ed5374) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "1386.bin", 0x000000, 0x080000, CRC(72ddabc9) SHA1(c68b0896b4c25b591029231dff045b16eab61ac4) )
ROM_END


ROM_START( j6bigbnk )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20346.bin", 0x000000, 0x020000, CRC(6e717ecd) SHA1(f285a2d0fb0aa56b743a922087c416fa66ec1f52) )
	ROM_LOAD16_BYTE( "20347.bin", 0x000001, 0x020000, CRC(c515e34e) SHA1(fc3aeb2f3fa9ba463f8f39cf26d3795be869ffbc) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20348.bin", 0x000000, 0x020000, CRC(85ef2af5) SHA1(ccfd4e5296e3889045b0b8ba11a066104b266f13) )
	ROM_LOAD16_BYTE( "20349.bin", 0x000000, 0x020000, CRC(3cb5beac) SHA1(017fe4f39faff27e23ef1eba7dd687a276824da7) )
	ROM_LOAD16_BYTE( "20350.bin", 0x000000, 0x020000, CRC(0c1c0f45) SHA1(6da230716fd6bcb2df55857cf8bfa1d3a6fd0484) )
	ROM_LOAD16_BYTE( "6996.bin", 0x000000, 0x020000, CRC(5fcf0a19) SHA1(07ca6d27db65b7af16f330e5fb284f7b72220a23) )
	ROM_LOAD16_BYTE( "6997.bin", 0x000000, 0x020000, CRC(ef4bb8ba) SHA1(1fdbffbc5e504934310cca90eb6f2ca4194eea77) )
	ROM_LOAD16_BYTE( "6998.bin", 0x000000, 0x020000, CRC(b4515e21) SHA1(b8fc75056b4e5be288a6a7e210c11d9ea6c7b48a) )
	ROM_LOAD16_BYTE( "6999.bin", 0x000000, 0x020000, CRC(0d0bca78) SHA1(8b28af82f4f21d4e7102a9787aa54413ed46e21c) )
	ROM_LOAD16_BYTE( "7000.bin", 0x000000, 0x020000, CRC(5382a428) SHA1(86f513cbd969dbfbcac53de637e3796daab03037) )
	ROM_LOAD16_BYTE( "7306.bin", 0x000000, 0x020000, CRC(b33ec3b3) SHA1(12abb4621b9bce57994c1958c9df66b1048d8819) )
	ROM_LOAD16_BYTE( "7373.bin", 0x000000, 0x020000, CRC(36c4b97c) SHA1(e925c85220ff96af5dde67300f7c0b9e3e54dcb2) )
	ROM_LOAD16_BYTE( "7426.bin", 0x000000, 0x020000, CRC(d8cbee6c) SHA1(47a86689a8dc2946765520633d3d1f0fb4a2a051) )
	ROM_LOAD16_BYTE( "7427.bin", 0x000000, 0x020000, CRC(ceab9d77) SHA1(56b56781983509db84eeb53c2f70010d1d221fff) )
	ROM_LOAD16_BYTE( "7428.bin", 0x000000, 0x020000, CRC(3355ba54) SHA1(48eb078d0ec0b56a997fa949a2054da1fd24b1f2) )
	ROM_LOAD16_BYTE( "7660.bin", 0x000000, 0x020000, CRC(af560c17) SHA1(1a3b8db230d5ade8dbdd2f1d776c54d1c3601992) )
	ROM_LOAD16_BYTE( "7661.bin", 0x000000, 0x020000, CRC(d0365768) SHA1(29d11dbeb5ffbd692247a42a9f14668cb376d56d) )
	ROM_LOAD16_BYTE( "7662.bin", 0x000000, 0x020000, CRC(44c8582f) SHA1(e877134bfd9b3ac638340f9e0e02f3f63d52c434) )
	ROM_LOAD16_BYTE( "7663.bin", 0x000000, 0x020000, CRC(fd92cc76) SHA1(f566a1c6fdbbee6db5efcb0eef8bce39d48a4cf2) )
	ROM_LOAD16_BYTE( "bb20p1ac", 0x000000, 0x020000, CRC(9865f664) SHA1(e44263ade7dbe4c4e0d0616afd6502780b058d8f) )
	ROM_LOAD16_BYTE( "bb20p2ac", 0x000000, 0x020000, CRC(16d40b7e) SHA1(df81916860495f6d53ed4016c435dde8d6d7ad5f) )
	ROM_LOAD16_BYTE( "bb841ac", 0x000000, 0x020000, CRC(793d3d36) SHA1(d62e2ee62ed4f3a19c05edae1ea092679a57e8a5) )
	ROM_LOAD16_BYTE( "bba915p1", 0x000000, 0x020000, CRC(3b8ccfbc) SHA1(7f42d9dbcf976c12721324ea29cf942ad634985d) )
	ROM_LOAD16_BYTE( "bba915p2", 0x000000, 0x020000, CRC(bad068d5) SHA1(dd77602f7bdbcb04c7e6d3cb4a52e270fd3e7d52) )
	ROM_LOAD16_BYTE( "bbv48p2", 0x000000, 0x020000, CRC(96a9f1f5) SHA1(2b8b4f06edbe35f5b73d25bac8a973feeaa77b47) )
	ROM_LOAD16_BYTE( "bbv4p18c", 0x000000, 0x020000, CRC(793d3d36) SHA1(d62e2ee62ed4f3a19c05edae1ea092679a57e8a5) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "1109.bin", 0x000000, 0x080000, CRC(b4d7ac12) SHA1(ac194d15e9d4e5cdadddbf2dc3c9660b52f116c2) )
ROM_END


ROM_START( j6bigbuk )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7479.bin", 0x000000, 0x020000, CRC(a70145c7) SHA1(8da5b84c1842071b2273381f3d88d5bea7794ca1) )
	ROM_LOAD16_BYTE( "7480.bin", 0x000001, 0x020000, CRC(6fa0b3b0) SHA1(dde0e133e5efd3ebb245da4e51e9c8ca91374659) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7481.bin", 0x000000, 0x020000, CRC(4c9f11ff) SHA1(20e05cfd2a7166becd58711d48d12fdd90877953) )
	ROM_LOAD16_BYTE( "7482.bin", 0x000000, 0x020000, CRC(f5c585a6) SHA1(7c080e9113e1b7eedd3bfd1eb7096591943bdd43) )
	ROM_LOAD16_BYTE( "9478.bin", 0x000000, 0x020000, CRC(28f6194b) SHA1(e4ecf64eed37812b86f52aa06417594e780982d6) )
	ROM_LOAD16_BYTE( "9479.bin", 0x000000, 0x020000, CRC(194010ff) SHA1(abf4b6fd1cd97fd4352d57b5d85188b78be13887) )
	ROM_LOAD16_BYTE( "9480.bin", 0x000000, 0x020000, CRC(c3684d73) SHA1(dab582b30104bd581e5c11c574d38b985182605e) )
	ROM_LOAD16_BYTE( "9481.bin", 0x000000, 0x020000, CRC(7a32d92a) SHA1(e4e1d9c95373e0c0a28f6e1886441ebdb57ca1bb) )
	ROM_LOAD16_BYTE( "9482.bin", 0x000000, 0x020000, CRC(4a9b68c3) SHA1(69a475efb305722f6a0bf7165d32396de104bb82) )
	ROM_LOAD16_BYTE( "bb101.hex", 0x000000, 0x020000, CRC(d38e13bd) SHA1(1665160ad34d693774145b44da751c36c5b316b8) )
	ROM_LOAD16_BYTE( "bb102.hex", 0x000000, 0x020000, CRC(3772ae99) SHA1(68d6a8ffc35568742952811a698da465af0e6925) )
	ROM_LOAD16_BYTE( "bbuv8p1", 0x000000, 0x020000, CRC(0db84f74) SHA1(2e568576ee33c5ad7bc7e813c7e5598f5bd82dd9) )
	ROM_LOAD16_BYTE( "bbuv8p2", 0x000000, 0x020000, CRC(4b016cf7) SHA1(94a846c92a5ac39fd9f734de833e7dfeba031ee4) )
	ROM_LOAD16_BYTE( "bibuv8p1", 0x000000, 0x020000, CRC(5f7c8f15) SHA1(fa40783a7f8731ccc279dff723f554a7170a41d7) )
	ROM_LOAD16_BYTE( "bibv58p1", 0x000000, 0x020000, CRC(3a89174d) SHA1(a356bcc6421d185ba8403da6fffda384a929676b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "bb_snd.bin", 0x000000, 0x080000, CRC(d4d57f9f) SHA1(2ec38b62928d8c208880015b3a5e348e9b1c2079) )
ROM_END


ROM_START( j6bigcsh )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bcm4_2.p1", 0x000000, 0x020000, CRC(f412dcf8) SHA1(5e008b1966bd1aed3173836bbd2b09e6b368ac52) )
	ROM_LOAD16_BYTE( "bcm4_2.p2", 0x000001, 0x020000, CRC(edc32567) SHA1(5e1682399ac7d6f06d63c840c79d7ead6c0e4bd6) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "bcmsnd.bin", 0x000000, 0x080000, CRC(4acd8905) SHA1(49c19a25fd6a7bdddc6a3d8bed663019fc6b0ccc) )
ROM_END


ROM_START( j6bigpct )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "big11nsk.1", 0x000000, 0x020000, CRC(fabe2c0d) SHA1(522cd19e6e947afb485e6fd81e3589a97ec5ba0b) )
	ROM_LOAD16_BYTE( "big11.2",	   0x000001, 0x020000, CRC(34c3695e) SHA1(c3ab8710ebdc4d5f368d5b2a0c4803e939bc8bd8) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "big11dsk.1", 0x000000, 0x020000, CRC(11207835) SHA1(6e175d6fbe27446b058f885ae1a1ca98dc3ef409) )
	ROM_LOAD16_BYTE( "big11nak.1", 0x000000, 0x020000, CRC(a87aec6c) SHA1(1893f89e673a05926d20f6b6c318af09859d8f7d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6bigtop )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20870.bin", 0x000000, 0x020000, CRC(3a9d8758) SHA1(0736eefb516f104272cc6269f1850b5f03b6186d) )
	ROM_LOAD16_BYTE( "20868.bin", 0x000001, 0x020000, CRC(92fce54d) SHA1(4396ad175fae258fd00f7dc362c36d6065b4bfb4) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "btcl-4f1.bin", 0x000000, 0x020000, CRC(52a253d2) SHA1(bd518da4d9daab3dfcd26f723e907bca16576e63) )
	ROM_LOAD16_BYTE( "btcl-4p1.bin", 0x000000, 0x020000, CRC(7de6e4fa) SHA1(c709d3d155fa689ea53f6839a0900b1c3f452a66) )
	ROM_LOAD16_BYTE( "btcl-4s1.bin", 0x000000, 0x020000, CRC(9678b0c2) SHA1(e20c778935f0c710444230d6f06d6572c976d5dd) )
	ROM_LOAD16_BYTE( "btcl-4s2.bin", 0x000000, 0x020000, CRC(25a38540) SHA1(ee1cabb62f998e43f10a0c34bdca916ca2f1b01c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "btcl-snd.bin", 0x000000, 0x080000, CRC(160d0317) SHA1(bb111b0a96fde85acd197ef9147eae2b7059da36) )
ROM_END


ROM_START( j6bigwhl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9538.bin", 0x000000, 0x020000, CRC(4a3fee6e) SHA1(3a55a1c89a562877f9db805bd036d7566c2cb4a2) )
	ROM_LOAD16_BYTE( "9539.bin", 0x000001, 0x020000, CRC(98335286) SHA1(57792f661e82cbb7afe6e1723a419219c7b6e7b7) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9540.bin", 0x000000, 0x020000, CRC(a1a1ba56) SHA1(1ac886e66182d86797495467edfb099bba8f03be) )
	ROM_LOAD16_BYTE( "9541.bin", 0x000000, 0x020000, CRC(18fb2e0f) SHA1(fa8f348cbd465f6ba9c74144cfbeb74b31f57be0) )
	ROM_LOAD16_BYTE( "9542.bin", 0x000000, 0x020000, CRC(28529fe6) SHA1(2b110dd8c1f99545025ad354857fef849a9f7ace) )
	ROM_LOAD16_BYTE( "bwheelp1", 0x000000, 0x020000, CRC(fc8a8aa3) SHA1(13949e37a89eab9c906d91ca93398b778839011c) )
	ROM_LOAD16_BYTE( "bwheelp2", 0x000000, 0x020000, CRC(67ee23ae) SHA1(0059c40e7379958a71e0d5ba3e17622a879b59ba) )

	ROM_LOAD16_BYTE( "bigwheel8cash-p1.bin", 0x0000, 0x020000, CRC(9fa585a9) SHA1(10c6c42772bec0e974c86a96029e8cf42c14c983) )
	ROM_LOAD16_BYTE( "bigwheel8cash-p2.bin", 0x0000, 0x020000, CRC(3375b0e0) SHA1(f1d85364ec2dee48ddf1891c96d4059e38e8902e) )



	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "bigwheelsnd.bin", 0x000000, 0x080000, CRC(90a19aaa) SHA1(7b17e9fda01d4fb163e09107759a6bf473fc6dc0) )
ROM_END


ROM_START( j6bnza )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "6855.bin", 0x000000, 0x020000, CRC(0dc81f2b) SHA1(6bf9e75cf5daa1a16423854bc89e041755dece8e) )
	ROM_LOAD16_BYTE( "6856.bin", 0x000001, 0x020000, CRC(985853b4) SHA1(3492b8ee3de5c90ca110c3d07d28e5efe5bee15b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "6857.bin", 0x000000, 0x020000, CRC(e6564b13) SHA1(b822a057c342a5995759be3e44e4b476a67457e5) )
	ROM_LOAD16_BYTE( "6858.bin", 0x000000, 0x020000, CRC(5f0cdf4a) SHA1(72826b95b7a1288e172053f0ad94ca9f24cb556d) )
	ROM_LOAD16_BYTE( "6859.bin", 0x000000, 0x020000, CRC(0185b11a) SHA1(e6ebab44496ac47cd89c766b3842364b811b4bed) )
	ROM_LOAD16_BYTE( "8570.bin", 0x000000, 0x020000, CRC(5695eabe) SHA1(ed34a3996fdfcf587ce4c87e6bcd365c89ac922b) )
	ROM_LOAD16_BYTE( "8571.bin", 0x000000, 0x020000, CRC(9358e394) SHA1(37ea22c74bc659305c0212211ba3b79fdb6754d1) )
	ROM_LOAD16_BYTE( "8572.bin", 0x000000, 0x020000, CRC(bd0bbe86) SHA1(d709f84ea42b77e3f2b51987f1ba4c2d48f3f2ba) )
	ROM_LOAD16_BYTE( "8573.bin", 0x000000, 0x020000, CRC(04512adf) SHA1(39dbb25c80db04a25c6966a0401daa196d165b8f) )
	ROM_LOAD16_BYTE( "8616.bin", 0x000000, 0x020000, CRC(5ad8448f) SHA1(32c74f9316b8f79b4e303de64fa29d64ee9a1f54) )
	ROM_LOAD16_BYTE( "bon5p18c", 0x000000, 0x020000, CRC(c05ecd81) SHA1(f16f9e6bb44eb0df0c9c7467d8624f1f60884bf4) )
	ROM_LOAD16_BYTE( "bon8p2", 0x000000, 0x020000, CRC(c728b655) SHA1(5a50e5204e64b550fb0ecd6caf4f43cc3d936f1d) )
	ROM_LOAD16_BYTE( "bon8std", 0x000000, 0x020000, CRC(cc1363b0) SHA1(b4e5cf134cd25ad99e487209bed2a078fbd644ee) )
	ROM_LOAD16_BYTE( "bona8.1i", 0x000000, 0x020000, CRC(e156ba97) SHA1(a5f489313bcc65bb7e49dcabc4ea985a49afcb01) )
	ROM_LOAD16_BYTE( "bonv5p1", 0x000000, 0x020000, CRC(cc1363b0) SHA1(b4e5cf134cd25ad99e487209bed2a078fbd644ee) )
	ROM_LOAD16_BYTE( "bonv5p2", 0x000000, 0x020000, CRC(c728b655) SHA1(5a50e5204e64b550fb0ecd6caf4f43cc3d936f1d) )
	ROM_LOAD16_BYTE( "bonz10p1", 0x000000, 0x020000, CRC(96de5d60) SHA1(9a41b514aebf68581d82459040f62f97b10ef02f) )
	ROM_LOAD16_BYTE( "bonz10p2", 0x000000, 0x020000, CRC(475a8098) SHA1(08a6573fe6c9df790b54dc036884b8bdc5e52612) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "1064.bin", 0x000000, 0x080000, CRC(266edecb) SHA1(c985081bd2a4500889aae0dc7ecd7d8e4cbd1591) )
ROM_END


ROM_START( j6brkout )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bk30cz_04_1.b8", 0x000000, 0x020000, CRC(b6e8611d) SHA1(c2cf0e73d4a3fb94320abe48a76e9fff692b4a9c) )
	ROM_LOAD16_BYTE( "bk30cz_04_2.b8", 0x000001, 0x020000, CRC(b243e4eb) SHA1(093fec6e46c1c26e73011fca1ff8b7f847d27d96) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "bk75cz_04_1.b8", 0x000000, 0x020000, CRC(baa5cf2c) SHA1(d24262431b3f15adf784006fd422efb3c79e1197) )
	//ROM_LOAD16_BYTE( "bk75cz_04_2.b8", 0x000000, 0x020000, CRC(b243e4eb) SHA1(093fec6e46c1c26e73011fca1ff8b7f847d27d96) ) // == bk30cz_04_2.b8

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "bksnd.bin", 0x000000, 0x080000, CRC(f72bd4f4) SHA1(ef8651fe7bb5f5340e41d35ca0669cba7d9c1372) )
ROM_END


ROM_START( j6bucks )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "bucksfizz.p1", 0x000000, 0x020000, CRC(6ebfa6e5) SHA1(16a7a0f8e4271edaf95f898bbf4c4f5cb8936e41) )
	ROM_LOAD16_BYTE( "bucksfizz.p2", 0x000001, 0x020000, CRC(2986a6b8) SHA1(4bfdc3828fa5723491a8e8dcb8e48dfea1e897fe) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "bucksfizzsnd.bin", 0x000000, 0x080000, CRC(5f68aabc) SHA1(e8ce0cf8b43337e0bd9e098acadd19480c98c3bf) )
ROM_END


ROM_START( j6buzz ) // extra hw at 80000?
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prom1n.bin", 0x000000, 0x080000, CRC(2b47efd8) SHA1(bc96a5ea2511081f73a120e025249018c517c638) )
	ROM_LOAD16_BYTE( "prom2.bin",  0x000001, 0x080000, CRC(3a1c38a3) SHA1(cb85e1a9535ba646724db5e3dfbdb81384ada918) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END







ROM_START( j6cpclb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cpalaceclubr1", 0x000000, 0x020000, CRC(e556eb51) SHA1(0d63e35b4cf7cbd0f56e2ee9b250d1499cec6614) )
	ROM_LOAD16_BYTE( "cpalaceclubr2", 0x000001, 0x020000, CRC(0d0ca65f) SHA1(b24f4d3127c610191f823899538a89110de471ec) )

	ROM_REGION( 0x1000000, "altrevs", 0 ) // from a set marked as crystal
	ROM_LOAD( "cpc.1.b8", 0x000000, 0x020000, CRC(42bf4422) SHA1(9f78a61d3cb929c5b8c4a56f34c87a7117e48b96) )
	ROM_LOAD( "cpc2.b8", 0x000000, 0x020000, CRC(7212cd14) SHA1(074830d0bd7519adcdd8906e480c308f38025f7b) )
	ROM_LOAD( "cpcp1.b8", 0x000000, 0x020000, CRC(a921101a) SHA1(9a24f8e7ec34225074402a8eeb8ff7d96ee96532) )
	ROM_LOAD( "cpc-c2.bin", 0x000000, 0x020000, CRC(c91c14ce) SHA1(a0aea950f45e0e110b5a8ff5e12590dd5822f31b) )
	ROM_LOAD( "cpc-cp1.bin", 0x000000, 0x020000, CRC(7e00d415) SHA1(d2f08457cd638479bb7c3c7a2673894b59e727c3) )


	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cpalaceclubsnd", 0x000000, 0x080000, CRC(ef433c44) SHA1(049bdbbe8d88fb77dbfc9c2690e62982e7fe20ea) )
ROM_END


ROM_START( j6camelt )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20298.bin", 0x000000, 0x020000, CRC(9d773b08) SHA1(98ce12b4a7983c8b2765358fb3b070249ca25a4a) )
	ROM_LOAD16_BYTE( "20299.bin", 0x000001, 0x020000, CRC(740b93b2) SHA1(cd4ffab8d5c229f236c4771f3d1ff6d7ea94074d) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20300.bin", 0x000000, 0x020000, CRC(76e96f30) SHA1(28e65cdc38a93652f4650b48e1c3ff689b13c8bd) )
	ROM_LOAD16_BYTE( "20301.bin", 0x000000, 0x020000, CRC(cfb3fb69) SHA1(0ce50a66ff856bcbcb81beb78be14fb3d0ba7b2b) )
	ROM_LOAD16_BYTE( "20302.bin", 0x000000, 0x020000, CRC(ff1a4a80) SHA1(82f9856264831cee7b5ee99617767625ec9504be) )
	ROM_LOAD16_BYTE( "cam15p1", 0x000000, 0x020000, CRC(88537c3d) SHA1(a0e26a1114fae1e495ae634bf90142ab5953745e) )
	ROM_LOAD16_BYTE( "cam15p2", 0x000000, 0x020000, CRC(048c536d) SHA1(0053e13c86c11ae37cfb1b802b4aeb2b6a34deb8) )
	ROM_LOAD16_BYTE( "camv8p1", 0x000000, 0x020000, CRC(88537c3d) SHA1(a0e26a1114fae1e495ae634bf90142ab5953745e) )
	ROM_LOAD16_BYTE( "camv8p2", 0x000000, 0x020000, CRC(048c536d) SHA1(0053e13c86c11ae37cfb1b802b4aeb2b6a34deb8) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "camsound.bin", 0x000000, 0x080000, CRC(70d5a16f) SHA1(0e8ec67387274298637598bf1ab8c479aa108c54) )
ROM_END


ROM_START( j6scarlt )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "captscar.p1", 0x000000, 0x020000, CRC(ce9bc3f8) SHA1(ac303b33df4a2b022ebdff2f64ef181c5d59e968) )
	ROM_LOAD16_BYTE( "captscar.p2", 0x000001, 0x020000, CRC(38ff9c39) SHA1(4ec58fe670e7b64352181773c40e42618a96ac51) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "captscarsnd.bin", 0x000000, 0x080000, CRC(89768137) SHA1(1f93afaae31f421d07d840b44e25578a90868910) )
ROM_END


ROM_START( j6cshbox )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cbc2_0.p1", 0x000000, 0x020000, CRC(e2b9b11f) SHA1(cf5cffa6300f569f2e4cca4b3706c9eeeecc7949) )
	ROM_LOAD16_BYTE( "cbc2_0.p2", 0x000001, 0x020000, CRC(8b93ee17) SHA1(515491a68651e57bfd77a4470f100e50b3287c2b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cbc3_6.p1", 0x000000, 0x020000, CRC(25051c70) SHA1(5f60bb99fc1b30f6ef6496a440570f2f09e042a0) )
	ROM_LOAD16_BYTE( "cbc3_6.p2", 0x000000, 0x020000, CRC(78adbff4) SHA1(a4e2817b8df2f56bdb128f7b8987f3cb5592b6e2) )
	ROM_LOAD16_BYTE( "cbc4_0.p1", 0x000000, 0x020000, CRC(a0b52d70) SHA1(d23ef23a86b6e3d4dc58c6ba024bde5646749f39) )
	ROM_LOAD16_BYTE( "cbc4_0.p2", 0x000000, 0x020000, CRC(2f2864d6) SHA1(6d50cc57c762bd3ded60356318e70ab5d24cd0b6) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cbcsnd.bin", 0x000000, 0x080000, CRC(bf209b9b) SHA1(d2c079b05baeae80ed772509c3d9640e682addcd) )
ROM_END


ROM_START( j6cshbeu )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cbe0_3.p1", 0x000000, 0x020000, CRC(184e945c) SHA1(a0659d1ddc9c1a9c973687769103b07b445b85c4) )
	ROM_LOAD16_BYTE( "cbe0_3.p2", 0x000001, 0x020000, CRC(e1f7c860) SHA1(95e0d8b802b1e171b092d509affcd9cfbb10eb80) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cbcsnd.bin", 0x000000, 0x080000, CRC(bf209b9b) SHA1(d2c079b05baeae80ed772509c3d9640e682addcd) )
ROM_END


ROM_START( j6cshbst )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20341.bin", 0x000000, 0x020000, CRC(b1935c10) SHA1(6b3555d66205cd7f9954bfbab16ac80d70781e3d) )
	ROM_LOAD16_BYTE( "20342.bin", 0x000001, 0x020000, CRC(236e20e4) SHA1(12d1e726ed4ab41cff4ff1fa94203ee684c3f763) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20343.bin", 0x000000, 0x020000, CRC(5a0d0828) SHA1(b39460e9d83d240eaec830222a3229d23db0e8d4) )
	ROM_LOAD16_BYTE( "20344.bin", 0x000000, 0x020000, CRC(e3579c71) SHA1(3df10417be704d8487d75de42fda6656f3eb2705) )
	ROM_LOAD16_BYTE( "20345.bin", 0x000000, 0x020000, CRC(d3fe2d98) SHA1(97d635372bb30f90334599155b0648a8c7c40768) )
	ROM_LOAD16_BYTE( "cbus15p1", 0x000000, 0x020000, CRC(152b259e) SHA1(3fa8c6465c7fc06f6de390386dccc34fc2545d2e) )
	ROM_LOAD16_BYTE( "cbus15p2", 0x000000, 0x020000, CRC(20feeb0f) SHA1(29a73ac17f24cba17d1efe6d354b9d775fc5e244) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "as1.bin", 0x000000, 0x080000, CRC(96127e49) SHA1(58bec4a024eb557995c67ac81880ad3a9de84ac0) )
ROM_END


ROM_START( j6cshcnt )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cd30cz07_1.b8", 0x000000, 0x020000, CRC(62a81565) SHA1(302f8887e4453b88b623a100dbaecca11a261eae) )
	ROM_LOAD16_BYTE( "cd30cz07_2.b8", 0x000001, 0x020000, CRC(63d0ca74) SHA1(a5a146c3463f555e77efbbf7eea65c6edc7ce37b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cd751.bin", 0x000000, 0x020000, CRC(6ee5bb54) SHA1(9f9d693fb06ae06512b568656855b204b242a4ec) )
	ROM_LOAD16_BYTE( "cd752.bin", 0x000000, 0x020000, CRC(63d0ca74) SHA1(a5a146c3463f555e77efbbf7eea65c6edc7ce37b) )
	ROM_LOAD16_BYTE( "cd75cz07_1.b8", 0x000000, 0x020000, CRC(6ee5bb54) SHA1(9f9d693fb06ae06512b568656855b204b242a4ec) )
	ROM_LOAD16_BYTE( "cd75cz07_2.b8", 0x000000, 0x020000, CRC(63d0ca74) SHA1(a5a146c3463f555e77efbbf7eea65c6edc7ce37b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cd75snd.bin", 0x000000, 0x080000, CRC(352e28cd) SHA1(c98307f5eaf511c9d281151d1c07ffd83f24244c) )
ROM_END


ROM_START( j6cshrd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cara-4a1.bin", 0x000000, 0x020000, CRC(6deb5704) SHA1(518ccc9f5c88e993ac430db1f3c200669da6fbd4) )
	ROM_LOAD16_BYTE( "cara-4a2.bin", 0x000001, 0x020000, CRC(b2d2acde) SHA1(b4e5ad405d9ab85901122fe3d10f29cd5b14ab89) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cara-4p1.bin", 0x000000, 0x020000, CRC(d4b1c35d) SHA1(ed3e99619731d4920c800c34e07f9a7d65d75605) )
	ROM_LOAD16_BYTE( "cara-4s1.bin", 0x000000, 0x020000, CRC(3f2f9765) SHA1(966eeee083901151f17ce03e12acc76ad5804591) )
	ROM_LOAD16_BYTE( "cara-4s2.bin", 0x000000, 0x020000, CRC(b2d2acde) SHA1(b4e5ad405d9ab85901122fe3d10f29cd5b14ab89) )
	ROM_LOAD16_BYTE( "cara-4wp.bin", 0x000000, 0x020000, CRC(b6dcb2d5) SHA1(dc542e3838e08b40f9e3dac2d7df9dfc841a519b) )
	ROM_LOAD16_BYTE( "cash raiders p2 23c4.bin", 0x000000, 0x020000, CRC(d61e26bf) SHA1(282d9455a8ab401a0eb04703b5e1b8f9ab37bfea) )
	ROM_LOAD16_BYTE( "cash raiders std 9661.bin", 0x000000, 0x020000, CRC(b97d1093) SHA1(e36d2c924e66f5868026bc0f1e31df955a49f874) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cashraider8b11as1 24-9-98.bin", 0x000000, 0x080000, CRC(8d28ba3c) SHA1(5d403bdc4cfd6a3e14c1e9458dcda112ed1770c5) )
//  ROM_LOAD( "carasnd.bin", 0x000000, 0x080000, CRC(8d28ba3c) SHA1(5d403bdc4cfd6a3e14c1e9458dcda112ed1770c5) )
ROM_END


ROM_START( j6cshtwr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cashtowers.p1", 0x000000, 0x020000, CRC(43a3d2bc) SHA1(5f96fef540075d3ded635975e4638dd4ab41cedf) )
	ROM_LOAD16_BYTE( "cashtowers.p2", 0x000001, 0x020000, CRC(fe4cd1d6) SHA1(4b45f9c84b7c23fd2176bd5a0bb7b38c0f08db7a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cashtowerssnd.bin", 0x000000, 0x080000, CRC(c51444db) SHA1(8c5b726cab00f73f9efb074ca060c29c80c4348b) )
ROM_END


ROM_START( j6cshvgs )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cast-31.bin", 0x000000, 0x020000, CRC(5b887503) SHA1(a1484bf3d8bdc354be78b5bdd83bb0fc5933d55a) )
	ROM_LOAD16_BYTE( "cast-32.bin", 0x000001, 0x020000, CRC(ddfe1279) SHA1(9131786a3861185f71bfb00be0ae98a907315c9a) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "3749", 0x000000, 0x020000, CRC(6ad9defb) SHA1(098cb32aea036321f9fe30067376856072e31d00) )
	ROM_LOAD16_BYTE( "3781", 0x000000, 0x020000, CRC(092ba65e) SHA1(6376ab330465f0690fcd80dae3f9cc848fc47fba) )
	ROM_LOAD16_BYTE( "3788", 0x000000, 0x020000, CRC(d3834aa2) SHA1(4d89f0c020be8731fa991c97d9a81657835187f7) )
	ROM_LOAD16_BYTE( "3789", 0x000000, 0x020000, CRC(381d1e9a) SHA1(7ee4e4f5aa01267e89af626dba819302933de0fc) )
	ROM_LOAD16_BYTE( "cast-3a1.bin", 0x000000, 0x020000, CRC(094cb562) SHA1(48d6a068ceb09580f0d6f4ccc71782ea18074d09) )
	ROM_LOAD16_BYTE( "cast-3n1.bin", 0x000000, 0x020000, CRC(6abecdc7) SHA1(ec227d63c0b309564c48412566a6f1d38282a8b8) )
	ROM_LOAD16_BYTE( "cast-3p1.bin", 0x000000, 0x020000, CRC(b016213b) SHA1(ca90373912f1b35d23ef9e8441348907529a074d) )
	ROM_LOAD16_BYTE( "cast3np1.bin", 0x000000, 0x020000, CRC(812099ff) SHA1(d63d87578866140c7a7f09c0d52ebc04cd42f087) )
	ROM_LOAD16_BYTE( "cc85", 0x000000, 0x020000, CRC(a7343977) SHA1(1de3372e1f9af30968c43d2c8dbacc40a115da71) )
	ROM_LOAD16_BYTE( "cvs-11.bin", 0x000000, 0x020000, CRC(895e45db) SHA1(15e2c51a08827322ed34220178476d87cce0b615) )
	ROM_LOAD16_BYTE( "cvs-12.bin", 0x000000, 0x020000, CRC(97aa7764) SHA1(d9a9db1112ccd2721a52ab45de9c0dc85ba027a1) )
	ROM_LOAD16_BYTE( "cvs-1a1.bin", 0x000000, 0x020000, CRC(db9a85ba) SHA1(9f459629aebcb4842e726c6c55427cd253e5eca8) )
	ROM_LOAD16_BYTE( "cvs-1n1.bin", 0x000000, 0x020000, CRC(b868fd1f) SHA1(3aefc4f96ea4dc386f98dd2fd70626c1b7e49c54) )
	ROM_LOAD16_BYTE( "cvs-1np1.bin", 0x000000, 0x020000, CRC(53f6a927) SHA1(6ba558699a3e64c7f785d963174e114dc208c253) )
	ROM_LOAD16_BYTE( "cvs-1p1.bin", 0x000000, 0x020000, CRC(62c011e3) SHA1(6d075f07583588f57ba9b21404e4dc292ebac1db) )
	ROM_LOAD16_BYTE( "cvs-41.bin", 0x000000, 0x020000, CRC(48c01ca7) SHA1(17d961b43cf7dc65fc67f3991ecd8839314885bc) )
	ROM_LOAD16_BYTE( "cvs-42.bin", 0x000000, 0x020000, CRC(d30d313c) SHA1(f3b3195e376702d9239d520e1bfe5733c91f9164) )
	ROM_LOAD16_BYTE( "cvs-4a1.bin", 0x000000, 0x020000, CRC(1a04dcc6) SHA1(6a1134aaa86ef37b4a092b8e6893f34d88a313a7) )
	ROM_LOAD16_BYTE( "cvs-4l1.bin", 0x000000, 0x020000, CRC(79f6a463) SHA1(5d48edc10dba3e4db3dad260bdc038aeeeababeb) )
	ROM_LOAD16_BYTE( "cvs-4lp1.bin", 0x000000, 0x020000, CRC(9268f05b) SHA1(d2fb3ce71943fd07f16887c414e3313b3fe2094e) )
	ROM_LOAD16_BYTE( "cvs-4p1.bin", 0x000000, 0x020000, CRC(a35e489f) SHA1(f652e1380a5b8a5c7953d88d1bcc51e94bb05f98) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cvssnd.bin", 0x000000, 0x080000, CRC(f2f828f5) SHA1(3d141884ea68d3e440ac43eaec3e8133fa8ae776) )
ROM_END


ROM_START( j6cas5 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ca5l-2_1.bin", 0x000000, 0x020000, CRC(91cd1258) SHA1(a6fc4be9754b906eef25995207eb62f60988e0dc) )
	ROM_LOAD16_BYTE( "ca5l-2_2.bin", 0x000001, 0x020000, CRC(1e69935e) SHA1(6322dbb7a9ed31a225355dc1b4fa951f2ee863bc) )


	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ca5l-2p1.bin", 0x000000, 0x020000, CRC(7a534660) SHA1(25be6e8d31af60c87227c536236157ff4154ea2e) )
	ROM_LOAD16_BYTE( "ca5l-3_1.bin", 0x000000, 0x020000, CRC(1a15f55c) SHA1(9fd2198a644a843a64e9ee22825f23b4cfbcc11e) )
	ROM_LOAD16_BYTE( "ca5l-3_2.bin", 0x000000, 0x020000, CRC(ae6403f9) SHA1(56b85018f9548f7f3e64112fde7d5cb7d619ef72) )
	ROM_LOAD16_BYTE( "ca5l-3p1.bin", 0x000000, 0x020000, CRC(f18ba164) SHA1(419efdd6a4fde98d46d0ed67cd1373d4c8b97702) )
	ROM_LOAD16_BYTE( "ca5l-4_1.bin", 0x000000, 0x020000, CRC(cff457d4) SHA1(c57cfadd07ae93deb435e19834ccf116d8f235b5) )
	ROM_LOAD16_BYTE( "ca5l-4_2.bin", 0x000000, 0x020000, CRC(d0db6d9c) SHA1(2c225a099fdfc45bc0c04a1a698259cb25db8db3) )
	ROM_LOAD16_BYTE( "ca5l-4a1.bin", 0x000000, 0x020000, CRC(9d3097b5) SHA1(c0b538bb17d14af08e7f68b4d9c7cc1bdfb1d53f) )
	ROM_LOAD16_BYTE( "ca5l-4p1.bin", 0x000000, 0x020000, CRC(246a03ec) SHA1(ffe8723390638982bcc7d5309c31f6049cf3f8cf) )
	ROM_LOAD16_BYTE( "ca5l-4w1.bin", 0x000000, 0x020000, CRC(ad99265c) SHA1(dec9e882045933b7d52c376340a0ca7fd9bb477c) )
	ROM_LOAD16_BYTE( "cfli-51.bin", 0x000000, 0x020000, CRC(b9f1b8dc) SHA1(ae37f689884c1163da5ce76dbafca48c78dc735f) )
	ROM_LOAD16_BYTE( "cfli-52.bin", 0x000000, 0x020000, CRC(c34ab739) SHA1(0289061bfaf96445c35ee11a6d494c23e0d7efc1) )
	ROM_LOAD16_BYTE( "cfli-5a1.bin", 0x000000, 0x020000, CRC(eb3578bd) SHA1(83a65664585eced36ab24c6853f6f97d11dd2beb) )
	ROM_LOAD16_BYTE( "cfli-5p1.bin", 0x000000, 0x020000, CRC(526fece4) SHA1(cb407bb9de53ac0b967e5568db1dd20672be37c1) )
	ROM_LOAD16_BYTE( "cfli-5w1.bin", 0x000000, 0x020000, CRC(db9cc954) SHA1(da44fb85c15c970c0e0fb75ed0cd0f652d1b8d82) )

	ROM_LOAD16_BYTE( "cas5line.p1", 0x00000, 0x020000, CRC(def7635e) SHA1(d47fb7d3d1212892fb4ca7c6deb508ba79b2d665) )
	ROM_LOAD16_BYTE( "cas5line.p2", 0x00001, 0x020000, CRC(7004a4d1) SHA1(003809bdaad1a6c9b72d2c16a2203bc62bc065a5) )

	// these two just look like random garabage, or are encrypted, something else?
	//  see note with j6fbcrz set
	//ROM_LOAD16_BYTE( "c-1.bin", 0x000000, 0x020000, CRC(fc45950a) SHA1(8721758e8f2ac41f26700965ed942cd1a311bb22) )
	//ROM_LOAD16_BYTE( "c-2.bin", 0x000001, 0x020000, CRC(e36aaf42) SHA1(c9da129f85c7b8ce27ea8cb9f090ae647eeac10d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cfl-snd.bin", 0x000000, 0x080000, CRC(0016ab04) SHA1(82d133f485b325b29db901f6254c80ca959abd3e) )
ROM_END




// check hw
ROM_START( j6cascz )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7648.bin", 0x000000, 0x020000, CRC(ac772dd4) SHA1(c288cdcf678edccdca7c24d62bb0d5b1bdddeae2) )
	ROM_LOAD16_BYTE( "7649.bin", 0x000001, 0x020000, CRC(9184a594) SHA1(a7db14191b04e7e9fba1f167157af0f187f2d591) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7650.bin", 0x000000, 0x020000, CRC(47e979ec) SHA1(9a0e9e0e43c720d6095e75dd735d98210e3fbc00) )
	ROM_LOAD16_BYTE( "7651.bin", 0x000000, 0x020000, CRC(feb3edb5) SHA1(9c6a33ce799a4baebc8f0aae8af6f7fba4697679) )
	ROM_LOAD16_BYTE( "8159.bin", 0x000000, 0x020000, CRC(36904fce) SHA1(b949fdba64d1642495ef3542f65f3b21765b6bc1) )
	ROM_LOAD16_BYTE( "8160.bin", 0x000000, 0x020000, CRC(165670b8) SHA1(ad78d8fb8a3c8b4a6cf5d62a0aae8cb35f3ebb8b) )
	ROM_LOAD16_BYTE( "8162.bin", 0x000000, 0x020000, CRC(64548faf) SHA1(b818d69cd172492137335e32fe892359f584b94d) )
	ROM_LOAD16_BYTE( "9504.bin", 0x000000, 0x020000, CRC(b49c7755) SHA1(a29c87c9b233041bc22b7a4124750947c5c95bba) )
	ROM_LOAD16_BYTE( "9505.bin", 0x000000, 0x020000, CRC(ab03c1ee) SHA1(2ad919fddf3917b0c112be1b0231e22863b33c20) )
	ROM_LOAD16_BYTE( "9506.bin", 0x000000, 0x020000, CRC(5f02236d) SHA1(6ded85fa771d4b27d597f7675b535c21dfafd222) )
	ROM_LOAD16_BYTE( "9507.bin", 0x000000, 0x020000, CRC(e658b734) SHA1(3bc8d74e4ec0eb4757c8f5ea43edd638b66edab8) )
	ROM_LOAD16_BYTE( "9508.bin", 0x000000, 0x020000, CRC(d6f106dd) SHA1(a60b01a23fe01e7cb1b009149bbcb6f9dd1ecee5) )
	ROM_LOAD16_BYTE( "cacr15p1", 0x000000, 0x020000, CRC(6e9b193d) SHA1(a129d741974804f34526902430738ff8adc40bea) )
	ROM_LOAD16_BYTE( "cacr15p2", 0x000000, 0x020000, CRC(a6f43b86) SHA1(18ad61127243a6c02f800dfe87942da1b22f1a5c) )
	ROM_LOAD16_BYTE( "ccrv12p1", 0x000000, 0x020000, CRC(b37544eb) SHA1(b684406bd95b76b5705e3980e143cbb2d0c35caa) )
	ROM_LOAD16_BYTE( "ccrv12p2", 0x000000, 0x020000, CRC(34c13ba3) SHA1(d20b7565b1183e95b68cd6f300953bec1d0c8086) )
	ROM_LOAD16_BYTE( "ccv12p1.bin", 0x000000, 0x020000, CRC(e1b1848a) SHA1(a5060b695029fab63f9b2ce3588b5d60e2be3682) )
	ROM_LOAD16_BYTE( "ccv12p2.bin", 0x000000, 0x020000, CRC(34c13ba3) SHA1(d20b7565b1183e95b68cd6f300953bec1d0c8086) )
	ROM_LOAD16_BYTE( "crazycasino.p1", 0x000000, 0x020000, CRC(8306dded) SHA1(c52c1283a83bfd7c25d80173722f33f53c404c52) )
	ROM_LOAD16_BYTE( "crazycasino.p2", 0x000000, 0x020000, CRC(7ef38dcb) SHA1(57b0110876b842065ff88806eec901daf6a06318) )
	ROM_LOAD16_BYTE( "ccrzy8p1", 0x000000, 0x020000, CRC(5057f874) SHA1(c609384f5673d7e86d55c9b4e73dc43fba017dd9) )
	ROM_LOAD16_BYTE( "ccrzy8p2", 0x000001, 0x020000, CRC(4d5c4236) SHA1(a69de914a6fc6b73eac29eacfaa105e5dfd3fc87) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "1274.bin", 0x000000, 0x080000, CRC(90a19aaa) SHA1(7b17e9fda01d4fb163e09107759a6bf473fc6dc0) )
	ROM_LOAD( "crazycasinosnd.bin", 0x000000, 0x080000, CRC(d10b8005) SHA1(e499e4e119956c7831dcec8dc8a6e338423afafb) )
ROM_END


ROM_START( j6cccla )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cccc-21.bin", 0x000000, 0x020000, CRC(95be44b1) SHA1(3057eb6b9e844e9ae025d657a4c59443590dfc84) )
	ROM_LOAD16_BYTE( "cccc-22.bin", 0x000001, 0x020000, CRC(6ba426e1) SHA1(0a6610b6f8383e64f342a029136944b385e611c4) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cccc-2f1.bin", 0x000000, 0x020000, CRC(5164a7a1) SHA1(1701df4590125822a68d0a4d06ec559e78785cd9) )
	ROM_LOAD16_BYTE( "cccc-2n1.bin", 0x000000, 0x020000, CRC(f7d33539) SHA1(9b92ce4993e1d36b9606e42ce4e6c4340182949d) )
	ROM_LOAD16_BYTE( "cccc-2p1.bin", 0x000000, 0x020000, CRC(7e201089) SHA1(146fdde9bd24bf850c5c673fd4a72ae5a561e5cd) )
	ROM_LOAD16_BYTE( "cccc-31.bin", 0x000000, 0x020000, CRC(31682e84) SHA1(3980b5d5a865ef9fabd68c7c382e76998d6bcdc3) )
	ROM_LOAD16_BYTE( "cccc-32.bin", 0x000000, 0x020000, CRC(a61a20a0) SHA1(e216cce234d8ceef25078bbbe90a72fca4de8d52) )
	ROM_LOAD16_BYTE( "cccc-3b1.bin", 0x000000, 0x020000, CRC(3d2580b5) SHA1(51517f69f6cfc26e698a31e40deaeb003f452af8) )
	ROM_LOAD16_BYTE( "cccc-3e1.bin", 0x000000, 0x020000, CRC(29f372e6) SHA1(4400374824d678a12bfa6068d4a7fea3b29e436e) )
	ROM_LOAD16_BYTE( "cccc-3f1.bin", 0x000000, 0x020000, CRC(f5b2cd94) SHA1(ce7dbab1fadddc1c2d6f3b1441339385104769fe) )
	ROM_LOAD16_BYTE( "cccc-3n1.bin", 0x000000, 0x020000, CRC(53055f0c) SHA1(eb2d848d562d5aa4f7557d5e84ec82b6452c7772) )
	ROM_LOAD16_BYTE( "cccc-3p1.bin", 0x000000, 0x020000, CRC(daf67abc) SHA1(c9750156a5ac02894bfd2a9d7412b47cb0e18aae) )
	ROM_LOAD16_BYTE( "cccc2fn1.bin", 0x000000, 0x020000, CRC(3309d629) SHA1(8fc3e20e6340ec36166bd12eb48943ed71ddcb88) )
	ROM_LOAD16_BYTE( "cccc3fb1.bin", 0x000000, 0x020000, CRC(f9ff63a5) SHA1(08795ebb856302b391288512c4c378cc37f62326) )
	ROM_LOAD16_BYTE( "cccccep1.bin", 0x000000, 0x020000, CRC(669b8877) SHA1(48372a46ff03d361e3f04bd224ce887c84689aaf) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6cascla )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cccc-11.bin", 0x000000, 0x020000, CRC(921262ff) SHA1(54f71f0e66e7d484c178daa4959ba6aff75230ba) )
	ROM_LOAD16_BYTE( "cccc-12.bin", 0x000001, 0x020000, CRC(98605d3e) SHA1(80dcc318ddcb46982da544ec43b8045692b21f70) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "badf", 0x000000, 0x020000, CRC(053fe05a) SHA1(3f2c3531ecfbb5b593709778023668e1bb49663a) )
	ROM_LOAD16_BYTE( "baee", 0x000000, 0x020000, CRC(481626fa) SHA1(923a8c6da5033716ae999be4680d1db7325f42ff) )
	ROM_LOAD16_BYTE( "baef", 0x000000, 0x020000, CRC(a38872c2) SHA1(6888e5c5261edda721eed18924dc4f4b1bde8268) )
	ROM_LOAD16_BYTE( "baf7", 0x000000, 0x020000, CRC(f0d3bb8e) SHA1(bbe9123fa9b1eb3640f6ee270ae88223178a8278) )
	ROM_LOAD16_BYTE( "bafb", 0x000000, 0x020000, CRC(d97e5f28) SHA1(589845eaef52cfacb8a07c803c431e3a743c70e8) )
	ROM_LOAD16_BYTE( "bafe", 0x000000, 0x020000, CRC(2a7b5772) SHA1(68e4cd32553c70aef8c301ea3b6bf738f59211f3) )
	ROM_LOAD16_BYTE( "baff", 0x000000, 0x020000, CRC(c1e5034a) SHA1(4716069d318fee3e948ff2e4f1ad743b9477070c) )
	ROM_LOAD16_BYTE( "cccc-1f1.bin", 0x000000, 0x020000, CRC(56c881ef) SHA1(b7dfb4a808c032f8806cadc82bc44b3c009832f0) )
	ROM_LOAD16_BYTE( "cccc-1n1.bin", 0x000000, 0x020000, CRC(f07f1377) SHA1(009f3f5eefd200a86b5720d2ca34cd193d05cc69) )
	ROM_LOAD16_BYTE( "cccc-1p1.bin", 0x000000, 0x020000, CRC(798c36c7) SHA1(01653581142ed5dd8cb9fcb69a16baf8aa5b6ca6) )
	ROM_LOAD16_BYTE( "cccc-c1.bin", 0x000000, 0x020000, CRC(959e802d) SHA1(73d32ad724da30131d3d7125bdb6b74a49e6e29e) )
	ROM_LOAD16_BYTE( "cccc-c2.bin", 0x000000, 0x020000, CRC(c91c14ce) SHA1(a0aea950f45e0e110b5a8ff5e12590dd5822f31b) )
	ROM_LOAD16_BYTE( "cccc-cb1.bin", 0x000000, 0x020000, CRC(8148727e) SHA1(022d44edc60a8df6874f34973734515f5a4b180b) )
	ROM_LOAD16_BYTE( "cccc-ce1.bin", 0x000000, 0x020000, CRC(8d05dc4f) SHA1(9308f51e64415688a87185b9258d70ae82045f2e) )
	ROM_LOAD16_BYTE( "cccc-cf1.bin", 0x000000, 0x020000, CRC(5144633d) SHA1(c52c512adddc90821aac85743f12acae34c82320) )
	ROM_LOAD16_BYTE( "cccc-cn1.bin", 0x000000, 0x020000, CRC(f7f3f1a5) SHA1(ebbff1909e91ce708a97f4ed2b5811c47951d27a) )
	ROM_LOAD16_BYTE( "cccc-cp1.bin", 0x000000, 0x020000, CRC(7e00d415) SHA1(d2f08457cd638479bb7c3c7a2673894b59e727c3) )
	ROM_LOAD16_BYTE( "cccc1np1.bin", 0x000000, 0x020000, CRC(1be1474f) SHA1(9a032b28ef7ec70000d9f7e14624cae0b39501b7) )
	ROM_LOAD16_BYTE( "cccccnp1.bin", 0x000000, 0x020000, CRC(1c6da59d) SHA1(ab77a898e000c2fecf8ef266390f1aea399d763d) )
	ROM_LOAD16_BYTE( "f983", 0x000000, 0x020000, CRC(f58c0577) SHA1(8dcbc071f4f11d03bff8d0e9656e4e1991423c56) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6casclb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20059.bin", 0x000000, 0x020000, CRC(523d8116) SHA1(5a2763d7095ab51e3d660f700d53d225594ff6ca) )
	ROM_LOAD16_BYTE( "20060.bin", 0x000001, 0x020000, CRC(7da19bcc) SHA1(6121f0d5d08b343316123730624b3fd6a7c43c9e) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20061.bin", 0x000000, 0x020000, CRC(b9a3d52e) SHA1(2b2af88674a80d64073dec38992b2e5a76463389) )
	ROM_LOAD16_BYTE( "ccc1enp1.bin", 0x000000, 0x020000, CRC(037a1b2d) SHA1(8838a56d08b95efb23e3137a22e2eb64fcacf812) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "1274.bin", 0x000000, 0x080000, CRC(90a19aaa) SHA1(7b17e9fda01d4fb163e09107759a6bf473fc6dc0) )
	ROM_LOAD( "cccsnd.bin", 0x000000, 0x080000, CRC(facc0580) SHA1(4f0307a6439e5df97ee4e80f6300e7bf056f3dad) )
ROM_END


ROM_START( j6caslas )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9282.bin", 0x000000, 0x020000, CRC(a15cc050) SHA1(d08187db0577d28adb1b6cf62b040ee47d52c82a) )
	ROM_LOAD16_BYTE( "9283.bin", 0x000001, 0x020000, CRC(628fbb1e) SHA1(714a5077644c049d69837caf8e9ce6562fd2eb6c) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9284.bin", 0x000000, 0x020000, CRC(4ac29468) SHA1(6f511cb9420078c9a800f6758592756170369012) )
	ROM_LOAD16_BYTE( "9285.bin", 0x000000, 0x020000, CRC(f3980031) SHA1(f833f5301d644efaa8d4c82c953538c06733dc1b) )
	ROM_LOAD16_BYTE( "9286.bin", 0x000000, 0x020000, CRC(c331b1d8) SHA1(c3eefdc8001b9b3990537902f616a573cc13803a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "clve-snd.bin", 0x000000, 0x020000, CRC(f77c12c7) SHA1(a2084be41949949aed787c1dccb64b969de81c86) ) // looks like a bad dump, most missing
	ROM_LOAD( "cas_las_vegas_snd.bin", 0x000000, 0x080000, CRC(d49c212e) SHA1(e4ddd743bdb9404afb5d6c3d2ae4ea1d625c331d) )
	ROM_LOAD( "caslasvegas30-06-97-4133as1.bin", 0x000000, 0x080000, CRC(d49c212e) SHA1(e4ddd743bdb9404afb5d6c3d2ae4ea1d625c331d) )
ROM_END


ROM_START( j6cheque )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "chequemate.p1", 0x000000, 0x020000, CRC(108ac160) SHA1(0b3322975246325968d8966613927b395107775b) )
	ROM_LOAD16_BYTE( "chequemate.p2", 0x000001, 0x020000, CRC(425ad113) SHA1(9dae4056699547da3b1ddebbee7c2ae108bfae4d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "chequematesnd.bin", 0x000000, 0x080000, CRC(310fc9a6) SHA1(636b6a0185ebe521441e01d7f381da630e31b3d9) )
ROM_END


ROM_START( j6cluclb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clcl-c1.bin", 0x000000, 0x020000, CRC(5ca7cc7f) SHA1(58947a86f365f7daa3995f21a7846774dd03bccc) )
	ROM_LOAD16_BYTE( "clcl-c2.bin", 0x000001, 0x020000, CRC(cbb0f1f8) SHA1(af30a8c1ae0eabd2c3ee52dc5ae5c353be4ac4ee) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9204", 0x000000, 0x020000, CRC(08149887) SHA1(e6be7e4b129c636b8c5bbf86c818a3705363e610) )
	ROM_LOAD16_BYTE( "b7df", 0x000000, 0x020000, CRC(cc06140e) SHA1(39da46d6dededad43b91802f88b19d22c99db1c9) )
	ROM_LOAD16_BYTE( "b7ef", 0x000000, 0x020000, CRC(6ab18696) SHA1(36358c4b8dac26480bbf0a236b4b326bb2ee44a3) )
	ROM_LOAD16_BYTE( "b7f7", 0x000000, 0x020000, CRC(39ea4fda) SHA1(d72ee50e81a9963ae90570d4d19b1cccf3641ee3) )
	ROM_LOAD16_BYTE( "b7fb", 0x000000, 0x020000, CRC(1047ab7c) SHA1(23a1cc3a86b952592dc834f305e8faa7460ce474) )
	ROM_LOAD16_BYTE( "b7fe", 0x000000, 0x020000, CRC(e342a326) SHA1(b59af7877378d9887e38e8b2d553dc6ddaad70ee) )
	ROM_LOAD16_BYTE( "b7ff", 0x000000, 0x020000, CRC(08dcf71e) SHA1(97d7a7b482cd095c5455e889f512baa0056c2507) )
	ROM_LOAD16_BYTE( "clcl-cb1.bin", 0x000000, 0x020000, CRC(50ea624e) SHA1(8bfc31f8e4e8ae1b56ea8443c94f00aa2f0c54d2) )
	ROM_LOAD16_BYTE( "clcl-ce1.bin", 0x000000, 0x020000, CRC(443c901d) SHA1(fe8f0ee17369f5c268ab22669b559c2f5ddd9c55) )
	ROM_LOAD16_BYTE( "clcl-cf1.bin", 0x000000, 0x020000, CRC(987d2f6f) SHA1(640c146610d52bf7119b770a7c6b98512f241434) )
	ROM_LOAD16_BYTE( "clcl-cn1.bin", 0x000000, 0x020000, CRC(3ecabdf7) SHA1(81b085aace4e5847132e811585266986c6203c28) )
	ROM_LOAD16_BYTE( "clcl-cp1.bin", 0x000000, 0x020000, CRC(b7399847) SHA1(21b536613c8a8a9df08c1dfeea2985e7353e5578) )
	ROM_LOAD16_BYTE( "clclcnp1.bin", 0x000000, 0x020000, CRC(d554e9cf) SHA1(a987e60a7190d3ec616075f98caa8d821b7506c7) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "clubcluedosnd.bin", 0x000000, 0x080000, CRC(80491c1b) SHA1(432ce61b26f77da10f5dc9230d8e3d4d988db4b4) )
ROM_END



ROM_START( j6col )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "coliseum.p1", 0x000000, 0x020000, CRC(9830062b) SHA1(75865b49516b01754a8f63d5b33a08cd354dc3a6) )
	ROM_LOAD16_BYTE( "coliseum.p2", 0x000001, 0x020000, CRC(8b96052d) SHA1(558fb066f83c647668024f4e379c8ab150574d45) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "coliseum_awp.bin", 0x000000, 0x020000, CRC(599d9e54) SHA1(37159694a31701a1d505e0d41b95c7d056d57e4c) )
	ROM_LOAD16_BYTE( "cseum1_0.p1", 0x000000, 0x020000, CRC(34d68c28) SHA1(a0a81bcea32226c4d9f98a02e455ee7cd8f01415) )
	ROM_LOAD16_BYTE( "cseum1_0.p2", 0x000000, 0x020000, CRC(75ebc601) SHA1(9a46491151adb020c2f5831b9bf8310e54fa319b) )

	ROM_LOAD16_BYTE( "coli-4_1.bin", 0x000000, 0x020000, CRC(599d9e54) SHA1(37159694a31701a1d505e0d41b95c7d056d57e4c) )
	ROM_LOAD16_BYTE( "coli-4_2.bin", 0x000000, 0x020000, CRC(7636f9f2) SHA1(80f4d47b0171b98d7a4dc632562f95a3443ee95c) )
	ROM_LOAD16_BYTE( "coli-4a1.bin", 0x000000, 0x020000, CRC(0b595e35) SHA1(f72d2d1d028333e954bedfc7139b37f33a85f2ff) )
	ROM_LOAD16_BYTE( "coli-4n1.bin", 0x000000, 0x020000, CRC(68ab2690) SHA1(d07abd3cd19c0622a35a55eaf1e6ca34acfd0b98) )
	ROM_LOAD16_BYTE( "coli-4np.bin", 0x000000, 0x020000, CRC(833572a8) SHA1(5d2a910180314bf28e186839d0e599dff5538cdd) )
	ROM_LOAD16_BYTE( "coli-4p1.bin", 0x000000, 0x020000, CRC(b203ca6c) SHA1(07fddcaf9e7ea36f63873dda49507db29e0bb17a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "coli_snd.bin", 0x000000, 0x080000, CRC(0f75b32e) SHA1(29e9e04d9a1686dfdf6cbdcd3acca23d6e64a048) )
ROM_END


ROM_START( j6colcsh )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "coli-6_1.bin", 0x000000, 0x020000, CRC(e560f327) SHA1(48c619dc65e023621a530419ef6363865ff59c2a) )
	ROM_LOAD16_BYTE( "coli-6_2.bin", 0x000001, 0x020000, CRC(318c2ee5) SHA1(2ca39615775d92879353505b514dab9e3e63754b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "coli-6a1.bin", 0x000000, 0x020000, CRC(b7a43346) SHA1(7e21e73a8d44b020aeb82eb1f17fe5c220fa1467) )
	ROM_LOAD16_BYTE( "coli-6n1.bin", 0x000000, 0x020000, CRC(d4564be3) SHA1(a13bd319c8d06d1e8c189becaaa84af15b8de443) )
	ROM_LOAD16_BYTE( "coli-6p1.bin", 0x000000, 0x020000, CRC(0efea71f) SHA1(b0f5a5f59378e7c4f9ae3266d48f76c0eddbb567) )
	ROM_LOAD16_BYTE( "coli6np1.bin", 0x000000, 0x020000, CRC(3fc81fdb) SHA1(b3edd3c4fbe9e97fbcb2f43aa8d3518c6c5058fc) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "coli-snd.bin", 0x000000, 0x080000, CRC(a194e5af) SHA1(649d6ecc4e15afd60c2f57d082a6d9846013c107) )
ROM_END


ROM_START( j6colmon )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "colourofmoney.p1", 0x000000, 0x020000, CRC(d63f7a33) SHA1(414bff43b9a4e4ad387df01cd61980b5c2d696dd) )
	ROM_LOAD16_BYTE( "colourofmoney.p2", 0x000001, 0x020000, CRC(56e9ec06) SHA1(3bedfeadc7a321925057a77281794f7b50a9c46d) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "como-1_1.bin", 0x000000, 0x020000, CRC(5fcc5f83) SHA1(ad5389d2bf1cc6838a5427f804ae52e14f2c091b) )
	ROM_LOAD16_BYTE( "como-1a1.bin", 0x000000, 0x020000, CRC(0d089fe2) SHA1(93df5a9808039c82548e1a5248addac620c9f0ba) )
	ROM_LOAD16_BYTE( "como-1n1.bin", 0x000000, 0x020000, CRC(6efae747) SHA1(5753e21804dd3ab646b4ffb04b252c186e2ba99a) )
	ROM_LOAD16_BYTE( "como-1p1.bin", 0x000000, 0x020000, CRC(b4520bbb) SHA1(cd4533ad746d3306eaf20d3a169820e41d5c49d2) )
	ROM_LOAD16_BYTE( "como-1w1.bin", 0x000000, 0x020000, CRC(3da12e0b) SHA1(64ea48087e9926949137a916d9e396899eec409b) )
	ROM_LOAD16_BYTE( "como-2_1.bin", 0x000000, 0x020000, CRC(6ad59b66) SHA1(1fdd07e9ac46b6fdaabd5b98bcdeebff1e960efc) )
	ROM_LOAD16_BYTE( "como-2_2.bin", 0x000000, 0x020000, CRC(f37e7fc6) SHA1(421120f17afadb8bc806755f89562466ba39b386) )
	ROM_LOAD16_BYTE( "como-2a1.bin", 0x000000, 0x020000, CRC(38115b07) SHA1(fd15616ea2a6c88337ee7891f29d351c596f2bc5) )
	ROM_LOAD16_BYTE( "como-2n1.bin", 0x000000, 0x020000, CRC(5be323a2) SHA1(1fae13b25d9acc70047779a22fe4cd36e2d532cd) )
	ROM_LOAD16_BYTE( "como-2p1.bin", 0x000000, 0x020000, CRC(814bcf5e) SHA1(3a2147cbe9d97eed4f90373f1323a1ed61a0c9d8) )
	ROM_LOAD16_BYTE( "como2np1.bin", 0x000000, 0x020000, CRC(b07d779a) SHA1(101617eb7ded22b28cb19c91eb736b9a7dc91369) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "colourofmoneysnd.bin", 0x000000, 0x080000, CRC(289d74c8) SHA1(04938d526e4a3079e9570fa946e5c6b9cc0ba311) )
ROM_END


ROM_START( j6coprob )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "60000024.p1", 0x000000, 0x020000, CRC(7ada6793) SHA1(9e2aa2e033bc6535d39caffe435ac6a7cc57d4f9) )
	ROM_LOAD16_BYTE( "60000024.p2", 0x000001, 0x020000, CRC(4b89f47b) SHA1(e2e5c2242afd4d8c338d40cf8b3da58ff876d2e3) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "60001024.p1", 0x000000, 0x020000, CRC(75491f8f) SHA1(b905488d6742b8ae2eefdb7f8b99476c3d839ab7) )
	ROM_LOAD16_BYTE( "60001024.p2", 0x000000, 0x020000, CRC(9b154ee7) SHA1(5be113031c877c407f06fa48d4565c6f4ddd4961) )
	ROM_LOAD16_BYTE( "cnr_5f8c_v200.p1", 0x000000, 0x020000, CRC(ce95a60f) SHA1(3661c680eebb2fcda7482255168a60b8224870f1) )
	ROM_LOAD16_BYTE( "cnr_6a28_v200.p1", 0x000000, 0x020000, CRC(54c790d1) SHA1(d14e03af85bbccb3160243e91a2c8c95901f645a) )
	ROM_LOAD16_BYTE( "cnr_a9e3_v200.p1", 0x000000, 0x020000, CRC(3103d119) SHA1(087221127ed533e64377c4704bae659a82cbd97e) )
	ROM_LOAD16_BYTE( "cnr_d1cf_v200.p2", 0x000000, 0x020000, CRC(88e0bb1c) SHA1(c70d39d918cfc7b5c91328fb52017a6899c0c602) )
	ROM_LOAD16_BYTE( "cnr_dbea_v200.p2", 0x000000, 0x020000, CRC(ca6960c8) SHA1(4224e6a6bece51dce698f40f795bafdc4e92d4a4) )
	ROM_LOAD16_BYTE( "cnr_fdc0_v200.p2", 0x000000, 0x020000, CRC(2109c6d7) SHA1(2c72231feca1888dd3a1d69081d8057d3847ce4f) )
	ROM_LOAD16_BYTE( "cnr_v200.p1", 0x000000, 0x020000, CRC(8d99ddd3) SHA1(465dfeb5316050c44f9a6c062da12bf62ab626d0) )
	ROM_LOAD16_BYTE( "cnr_v200.p2", 0x000000, 0x020000, CRC(81326a7f) SHA1(99f71768aaf49edaf20c3e72b18caea21df8686f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "crsound.bin", 0x000000, 0x080000, CRC(d39dd4f1) SHA1(9ce7870c00f9ccb797182af41a7d22e41624c8ce) )
ROM_END


ROM_START( j6crack )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "crac-2_1.bin", 0x000000, 0x020000, CRC(519116d1) SHA1(a99342eb7644ade2f904a148b13ea6e81c96d8ec) )
	ROM_LOAD16_BYTE( "crac-2_2.bin", 0x000001, 0x020000, CRC(60a9914a) SHA1(d5f8dad456e36cb455b0f5278b0f38edc78e4b49) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "crac-2a1.bin", 0x000000, 0x020000, CRC(0355d6b0) SHA1(7a929131720c8f82c9efaac005641a54f31f6758) )
	ROM_LOAD16_BYTE( "crac-2n1.bin", 0x000000, 0x020000, CRC(60a7ae15) SHA1(3fd13083775348871be6afae7dd82adf1143af1c) )
	ROM_LOAD16_BYTE( "crac-2np.bin", 0x000000, 0x020000, CRC(8b39fa2d) SHA1(12e76c767ccfe08d685933ea7734cdd3aef763c7) )
	ROM_LOAD16_BYTE( "crac-2p1.bin", 0x000000, 0x020000, CRC(ba0f42e9) SHA1(24db750b30e6ed5cc29b4e621a2be1ad1a514506) )
	ROM_LOAD16_BYTE( "crac-2w1.bin", 0x000000, 0x020000, CRC(33fc6759) SHA1(f0c8136093196870d188d90c334c0f3197df7f4f) )
	ROM_LOAD16_BYTE( "crac-3_1.bin", 0x000000, 0x020000, CRC(94d59bb4) SHA1(04a62de8ea746009d9c658e37b1d4900645f942f) )
	ROM_LOAD16_BYTE( "crac-3_2.bin", 0x000000, 0x020000, CRC(d2d8fb65) SHA1(4014e3f48c7b800c5232fda2b48023a3d56b4150) )
	ROM_LOAD16_BYTE( "crac-3a1.bin", 0x000000, 0x020000, CRC(c6115bd5) SHA1(bd926b903afcf665c66aa5b6a65e1b1a8d4155fc) )
	ROM_LOAD16_BYTE( "crac-3n1.bin", 0x000000, 0x020000, CRC(a5e32370) SHA1(2c64fd2c3fadb2e882ed5edb0009e70ec16d62ad) )
	ROM_LOAD16_BYTE( "crac-3p1.bin", 0x000000, 0x020000, CRC(7f4bcf8c) SHA1(170e2f3cf206282ab51b1872809f92676f1ffda5) )
	ROM_LOAD16_BYTE( "crac3np1.bin", 0x000000, 0x020000, CRC(4e7d7748) SHA1(c1d2e085b0b239a5217ca29e50f88caa5b24dcd3) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "crck_snd.hex", 0x000000, 0x080000, CRC(106bb6b5) SHA1(a2e109fd71575dcceec190efed0a9c81ec1f4048) )
ROM_END



ROM_START( j6crzclb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "01ff.p1", 0x000000, 0x020000, CRC(5c9ad223) SHA1(4b8ffe604f0a71b8b7a63919a1dceaab92a2e7d4) )
	ROM_LOAD16_BYTE( "1f83.p2", 0x000001, 0x020000, CRC(5dfa134a) SHA1(4127cb36f2c5a6a1ce0a5023f8064bffb29c4799) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "01df", 0x000000, 0x020000, CRC(98403133) SHA1(b43c964c02a5aa307540541bd45bfea9456571c4) )
	ROM_LOAD16_BYTE( "01f7", 0x000000, 0x020000, CRC(6dac6ae7) SHA1(8868f6eadbbb12704ef7ee5fff52569f19a628d9) )
	ROM_LOAD16_BYTE( "01fe", 0x000000, 0x020000, CRC(b704861b) SHA1(6e4c34d9a18c1db99e900bd9dc17270587c96fde) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6crsfir )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cf30cz_05_1.b8", 0x000000, 0x020000, CRC(1b65c071) SHA1(972626f73b3c7fdce97aae913ceea523f70a6ccc) )
	ROM_LOAD16_BYTE( "cf30cz_05_2.b8", 0x000001, 0x020000, CRC(dc630ed6) SHA1(9f3370126ceca05f21ebc13bba4d9efea7cb8b46) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "cf75cz_05_1.b8", 0x000000, 0x020000, CRC(17286e40) SHA1(979c02e6005705b258c04c1e38ad8d0896b61b3d) )
	ROM_LOAD16_BYTE( "cf75cz_05_2.b8", 0x000000, 0x020000, CRC(dc630ed6) SHA1(9f3370126ceca05f21ebc13bba4d9efea7cb8b46) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "crossfiresnd.bin", 0x000000, 0x080000, CRC(266edecb) SHA1(c985081bd2a4500889aae0dc7ecd7d8e4cbd1591) )
ROM_END


ROM_START( j6daygld )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dg30cz04_1.b8", 0x000000, 0x020000, CRC(ff3aefc9) SHA1(80e5485f0f6e67dac3bfaa5b3ba947ff63f9b745) )
	ROM_LOAD16_BYTE( "dg30cz04_2.b8", 0x000001, 0x020000, CRC(abedad68) SHA1(4a49695527ab0441e9c16cef9c632f6ac74d2d9a) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "dg30cz05_1.b8", 0x000000, 0x020000, CRC(e6808c66) SHA1(e0a6513ca62299c0906e458e84de46b3475e8f85) )
	ROM_LOAD16_BYTE( "dg30cz05_2.b8", 0x000000, 0x020000, CRC(a4d2f2cd) SHA1(77890d29c50ef9a355de898ef54a214b68c155c1) )
	ROM_LOAD16_BYTE( "dg30cz9_1.b8", 0x000000, 0x020000, CRC(03ec9247) SHA1(65d5d9f287da6fa8c832215b278033c65b62398a) )
	ROM_LOAD16_BYTE( "dg75cz02_1.b8", 0x000000, 0x020000, CRC(2c73b140) SHA1(d1172f435abaad65ba7c3c8e8401bd88284caca5) )
	ROM_LOAD16_BYTE( "dg75cz02_2.b8", 0x000000, 0x020000, CRC(f400645b) SHA1(59fdb302b421398985989bda02ee82b4e36b51d1) )
	ROM_LOAD16_BYTE( "dg75cz05_1.b8", 0x000000, 0x020000, CRC(eacd2257) SHA1(1c116944e5964d8d2b665e629c16cac61bc9fe9b) )
	ROM_LOAD16_BYTE( "dg75cz9_1.b8", 0x000000, 0x020000, CRC(0fa13c76) SHA1(e2ba888057f6b0ddafc47e57b428f548c3d45bb8) )
	ROM_LOAD16_BYTE( "dg75cza.bin", 0x000000, 0x020000, CRC(2c73b140) SHA1(d1172f435abaad65ba7c3c8e8401bd88284caca5) )
	ROM_LOAD16_BYTE( "dg75czb.bin", 0x000000, 0x020000, CRC(f400645b) SHA1(59fdb302b421398985989bda02ee82b4e36b51d1) )
	ROM_LOAD16_BYTE( "dg9_2.b8", 0x000000, 0x020000, CRC(7214a595) SHA1(bbaea30fc89afdf981536592016fcb4589447d7b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	// dgsnd.bin = popsnd.bin            m_popeye   Popeye (20p/8 GBP Token)
	ROM_LOAD( "dgsnd2.b8", 0x000000, 0x080000, CRC(7e8c05ce) SHA1(616b0f94b94331f86d7b1fec11dd05cf9b0968cf) )
ROM_END


ROM_START( j6dayml )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dm30cz08_1.b8", 0x000000, 0x020000, CRC(488a9da7) SHA1(604a2a6f7f07a9506906abeea79066459f375e61) )
	ROM_LOAD16_BYTE( "dm30cz08_2.b8", 0x000001, 0x020000, CRC(be52b8c0) SHA1(54e7a936d94cf65e246ddf6458e9d52c5b44cb8e) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "dm75cz08_1.b8", 0x000000, 0x020000, CRC(44c73396) SHA1(70ea5927a56bc44f4abb1b6d870c05935169c2d5) )
	ROM_LOAD16_BYTE( "dymil751.epr", 0x000000, 0x020000, CRC(99a735bd) SHA1(100fcaed9eca910424fd0872d03d9a594f96e0b5) )
	ROM_LOAD16_BYTE( "dymil752.epr", 0x000000, 0x020000, CRC(e6f2770d) SHA1(15d0ec40e31dc03c271774c873bc9651bd35336a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "daytona-sound_4m.epr", 0x000000, 0x080000, CRC(a74fa29c) SHA1(3db3322910717d4eda81b5df5988453fdebec7bf) )
ROM_END


ROM_START( j6dmnjkr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "demonjok_73001.b8", 0x000000, 0x020000, CRC(83670696) SHA1(d98713d54e42002ca2ba9b5c80a671121342400e) )
	ROM_LOAD16_BYTE( "demonjok_73002.b8", 0x000001, 0x020000, CRC(2a7658ab) SHA1(4286a4a76b8d95a4da4e8aad2f81b091d2d2f96a) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "demonjok_77501.b8", 0x000000, 0x020000, CRC(8f2aa8a7) SHA1(ea5b8c1418deaaf2bda58a40cacca3c77d6d5a08) )
	ROM_LOAD16_BYTE( "demonjok_77502.b8", 0x000000, 0x020000, CRC(2a7658ab) SHA1(4286a4a76b8d95a4da4e8aad2f81b091d2d2f96a) )
	ROM_LOAD16_BYTE( "dj30cz08_1.b8", 0x000000, 0x020000, CRC(b0470eae) SHA1(fe1c8e613083740c09be1703f7424fa9907a850a) )
	ROM_LOAD16_BYTE( "dj30cz08_2.b8", 0x000000, 0x020000, CRC(ba4aba56) SHA1(8c9ad521e35fc83ff52974622c26c537961abab3) )
	ROM_LOAD16_BYTE( "dj75cz08_1.b8", 0x000000, 0x020000, CRC(bc0aa09f) SHA1(f8de115fdc6bc0fca24590822f0f14acbfcdb23c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "demonjocker-sound_4m.epr", 0x000000, 0x04a018, CRC(796e1b35) SHA1(e9c8e5a350823275c9ba9238781872ea359d5049) )
ROM_END


ROM_START( j6dmngz )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "diamondgeezer.p1", 0x000000, 0x020000, CRC(518cd4e2) SHA1(93810c41f264666d0683179779f35dbbf3b86aa3) )
	ROM_LOAD16_BYTE( "diamondgeezer.p2", 0x000001, 0x020000, CRC(24ac143a) SHA1(e25f64021b09f12a5d0f8146bf071f6e565e0ba5) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "diamondgeezersnd.bin", 0x000000, 0x080000, CRC(6aa61ba4) SHA1(173b7aa31da9a9ee322653634711283f602b2743) )
ROM_END


ROM_START( j6dyfl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "dyfl-2_1.bin", 0x000000, 0x020000, CRC(4bd437c8) SHA1(70458993498b3803a723deb7e519708c6860d6cf) )
	ROM_LOAD16_BYTE( "dyfl-2_2.bin", 0x000001, 0x020000, CRC(25b31fc7) SHA1(1af3b14fd42de9db7834b2973f223cad282eb74d) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "dyfl-2a1.bin", 0x000000, 0x020000, CRC(1910f7a9) SHA1(2200a1ed2dea0e2c486885ae19ef918e709af244) )
	ROM_LOAD16_BYTE( "dyfl-2n1.bin", 0x000000, 0x020000, CRC(7ae28f0c) SHA1(fd3bf97fcf485fa34e2c8cace5f2880643dedb49) )
	ROM_LOAD16_BYTE( "dyfl-2np.bin", 0x000000, 0x020000, CRC(917cdb34) SHA1(931ec1ab8bb675f549adc3169d6379f83f11726f) )
	ROM_LOAD16_BYTE( "dyfl-2p1.bin", 0x000000, 0x020000, CRC(a04a63f0) SHA1(40c46d0e121e340000417aa07e8adba229dba499) )
	ROM_LOAD16_BYTE( "dyfl-2w1.bin", 0x000000, 0x020000, CRC(29b94640) SHA1(d8608d8f3d5def75b6a4fa0b04e7d1604abc86ea) )
	ROM_LOAD16_BYTE( "dyfl-3_1.bin", 0x000000, 0x020000, CRC(98242812) SHA1(ed2e678c4864d7f2b544e161ef2515a1a21178c2) )
	ROM_LOAD16_BYTE( "dyfl-3_2.bin", 0x000000, 0x020000, CRC(66e2a419) SHA1(969a1c61f5be9acf2d3a345c2ade59438e648f4a) )
	ROM_LOAD16_BYTE( "dyfl-3a1.bin", 0x000000, 0x020000, CRC(cae0e873) SHA1(cee90a05f04d3fbba2130728adda5051a7886c1f) )
	ROM_LOAD16_BYTE( "dyfl-3n1.bin", 0x000000, 0x020000, CRC(a91290d6) SHA1(f703e280a3c87c5e0c1dc9386b723b4194305894) )
	ROM_LOAD16_BYTE( "dyfl-3p1.bin", 0x000000, 0x020000, CRC(73ba7c2a) SHA1(a48209d5322e49788d3657d80426cc66f8092b12) )
	ROM_LOAD16_BYTE( "dyfl3np1.bin", 0x000000, 0x020000, CRC(428cc4ee) SHA1(d49b4bb84f036d154bdee723925a26278253441b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "doyoufeelluckysnd.bin", 0x000000, 0x080000, CRC(9148112e) SHA1(8976a03eb68b1f08c6260c095bddb8d8731539af) )
ROM_END


ROM_START( j6drdogh )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "drdough.p1", 0x000000, 0x080000, CRC(e669d9eb) SHA1(0ba5a02f0ba34f79ca86536ea04908ea6bd63b99) )
	ROM_LOAD16_BYTE( "drdough.p2", 0x000001, 0x080000, CRC(400ba359) SHA1(3c2a0f9df9fd81a011f82fcc2b4f9a9fe850ee9c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "drdoughsnd.bin", 0x000000, 0x080000, CRC(491676a5) SHA1(967841bf033aceead84db2e40acdd5f2e7e2b1e9) )
ROM_END


ROM_START( j6euphor )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "euph-1a1.bin", 0x000000, 0x020000, CRC(cb88f5e6) SHA1(247250040b44f4129e35bc3a77eb42e4cb14d8bd) )
	ROM_LOAD16_BYTE( "euph-1a2.bin", 0x000001, 0x020000, CRC(75c3deeb) SHA1(02d045c2fe2977195d02807f8f8576d2a9d48b63) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "euph-11.bin", 0x000000, 0x020000, CRC(2016a1de) SHA1(f11182b5903696f4a1e206a975fd7552e25baaf0) )
	ROM_LOAD16_BYTE( "euph-1n1.bin", 0x000000, 0x020000, CRC(a87a8d43) SHA1(0f2619d84a47c4e10895d228a770ff3a178aabca) )
	ROM_LOAD16_BYTE( "euph-1p1.bin", 0x000000, 0x020000, CRC(72d261bf) SHA1(7fcc2e9fc17742e4c43360ca8ffb1f45b1a3dbf4) )
	ROM_LOAD16_BYTE( "euph11.bin", 0x000000, 0x020000, CRC(994c3587) SHA1(013f98d5d1ca6927628cbd63ad7f4de12ec009a5) )
	ROM_LOAD16_BYTE( "euph1np1.bin", 0x000000, 0x020000, CRC(43e4d97b) SHA1(5b95539b84a5688dc1706d53e150b962494ac924) )
	ROM_LOAD( "euphoria.p1", 0x0000, 0x020000, CRC(27d40f99) SHA1(5732aa3b8d0e67acef3dda640453ad60caec1bcb) )
	ROM_LOAD( "euphoria.p2", 0x0000, 0x020000, CRC(480b239c) SHA1(9d0cdb979b2f63bdf5eef2d8b7a4718100b0c1bd) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "euphoriasnd.bin", 0x000000, 0x080000, CRC(d3097d34) SHA1(3db500b5ee38dfa580336b4bac43b139a31d2638) )
ROM_END


ROM_START( j6fastfr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clubfastfruits_v200.p1", 0x000000, 0x020000, CRC(7eef851a) SHA1(1ae689c08ff977644d8a0820e8f1c5dc82806939) )
	ROM_LOAD16_BYTE( "clubfastfruits_v200.p2", 0x000001, 0x020000, CRC(477a5c1a) SHA1(04db692a2b8258c5b8404a9143836c681ca43544) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "60003002.evn", 0x000000, 0x020000, CRC(18edd2fc) SHA1(eaf890328d90fe1f80a254877d2bc276da10a16f) )
	ROM_LOAD16_BYTE( "60003002.odd", 0x000000, 0x020000, CRC(97b39bc2) SHA1(9edccd3a4ecf62c04620e64350ba517aa086b2b1) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cffsnd.bin", 0x000000, 0x080000, CRC(29355a37) SHA1(5810f0eafe58b5d03cd104381eb92f55b1e08baa) )
ROM_END


 ROM_START( j6fasttk )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9331.bin", 0x000000, 0x020000, CRC(54dbf894) SHA1(a3ffff82883cc192108f44d36a7465d4afeaf114) )
	ROM_LOAD16_BYTE( "9332.bin", 0x000001, 0x020000, CRC(ecf1632a) SHA1(5d82a46672adceb29744e82de1b0fa5fcf4dbc51) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9333.bin", 0x000000, 0x020000, CRC(bf45acac) SHA1(ec624bc2d135901ecbdb6c6b3dbd9cc4b618b4de) )
	ROM_LOAD16_BYTE( "9334.bin", 0x000000, 0x020000, CRC(061f38f5) SHA1(459b39d2380fcfdb763eeb6937752be192cb8244) )
	ROM_LOAD16_BYTE( "9335.bin", 0x000000, 0x020000, CRC(36b6891c) SHA1(013b663f2dc59a4d2834ef2f7e86bcc608e98b39) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6filth )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7729.bin", 0x000000, 0x020000, CRC(5f272354) SHA1(23d2c710a628af9731ea67877ffd7b8309469c09) )
	ROM_LOAD16_BYTE( "7730.bin", 0x000001, 0x020000, CRC(83bbd350) SHA1(2171c3ddec8787b7ee0b48a022046490ebcf3bf9) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8198.bin", 0x000000, 0x020000, CRC(c77862d7) SHA1(33befc3976e68016489ce773fa964974fe62a8ce) )
	ROM_LOAD16_BYTE( "8199.bin", 0x000000, 0x020000, CRC(58a07ea1) SHA1(c33c464b2646e058a3f3013922298ed7ee3a3d67) )
	ROM_LOAD16_BYTE( "8200.bin", 0x000000, 0x020000, CRC(2ce636ef) SHA1(c8980f5b4e7786f103d24effa7f258bfddc1e7b2) )
	ROM_LOAD16_BYTE( "8201.bin", 0x000000, 0x020000, CRC(03a281c7) SHA1(fb5adbc3dafda25b4133730c0fff800014e295af) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "1151.bin", 0x000000, 0x080000, CRC(c3a2bf9b) SHA1(31536613fd9dcce0878109d460344591570c4334) )
//  ROM_LOAD( "frcl-snd.bin", 0x000000, 0x080000, CRC(c3a2bf9b) SHA1(31536613fd9dcce0878109d460344591570c4334) )
ROM_END




ROM_START( j6firbl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fireball.p1", 0x000000, 0x020000, CRC(c20a33dd) SHA1(4489b796d3b0121fbbeb9e226200566c0467dab6) )
	ROM_LOAD16_BYTE( "fireball.p2", 0x000001, 0x020000, CRC(fcc66f23) SHA1(15d4b65300377734692553edd58863627500eb41) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "fire-2_1.bin", 0x000000, 0x020000, CRC(a1e44b18) SHA1(871a7c8c85c31c46c885737a7dc2d5ea38301c25) )
	ROM_LOAD16_BYTE( "fire-2_2.bin", 0x000000, 0x020000, CRC(ef3ac4d2) SHA1(3cbae5282f6c5adb75c7bdc76f93350671389155) )
	ROM_LOAD16_BYTE( "fire-2a1.bin", 0x000000, 0x020000, CRC(f3208b79) SHA1(1014ba090ba8dc5968fc2069e20c8a89c70f8038) )
	ROM_LOAD16_BYTE( "fire-2p1.bin", 0x000000, 0x020000, CRC(4a7a1f20) SHA1(dfdf29eb821548e17ce43c1c3089c23ff7f9ce5c) )
	ROM_LOAD16_BYTE( "fire-2w1.bin", 0x000000, 0x020000, CRC(c3893a90) SHA1(dca996d03c7fecd358b9a78352c0cf672b36b44a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "fireballsnd.bin", 0x000000, 0x080000, CRC(e47444c7) SHA1(535ae2abdf5f9a1931c8b2afccf9a63b0778e5e3) )
ROM_END


ROM_START( j6fireck )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ficr-11.bin", 0x000000, 0x020000, CRC(655efa46) SHA1(f861bb97cd029353027379ab1a049218c3c987f5) )
	ROM_LOAD16_BYTE( "ficr-12.bin", 0x000001, 0x020000, CRC(b6f39b01) SHA1(67a80eb40923a282760ccb52c1265eff1c6623b2) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "04a9", 0x000000, 0x020000, CRC(6401b1ac) SHA1(10f57318bb8cd4b6dc1e12538de697c77c4ce71c) )
	ROM_LOAD16_BYTE( "3cbf", 0x000000, 0x020000, CRC(ba64f80f) SHA1(ae106127a2c085c7bc9758d0a03af2d94a625b42) )
	ROM_LOAD16_BYTE( "3cde", 0x000000, 0x020000, CRC(c7e48f46) SHA1(b81367724fb90834fc2431828ee9758dd19189d2) )
	ROM_LOAD16_BYTE( "3cf6", 0x000000, 0x020000, CRC(3208d492) SHA1(0d1947f80d2c8cc022229eff9120c689b48af4eb) )
	ROM_LOAD16_BYTE( "3cfe", 0x000000, 0x020000, CRC(033e6c56) SHA1(ec6f71eca170f7b7d367c476b0c742e1de6a597a) )
	ROM_LOAD16_BYTE( "3cff", 0x000000, 0x020000, CRC(e8a0386e) SHA1(2127c9e2419f0a92803b670a2c1962c2e23122fa) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "fccs1.bin", 0x000000, 0x080000, CRC(0421526d) SHA1(9dad850c208cb9f4a3a4c62e05a18217466d227e) )
ROM_END


ROM_START( j6firclb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "firecracker_club.p1", 0x000000, 0x020000, CRC(b0655026) SHA1(b0e556bbd1450035dd0a373eaf01a09d9cf90c60) )
	ROM_LOAD16_BYTE( "firecracker_club.p2", 0x000001, 0x020000, CRC(2838d65f) SHA1(cfbb1cf5d9ee7c2b5ef1ace8e29436244c762e67) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "13e6", 0x000000, 0x020000, CRC(2838d65f) SHA1(cfbb1cf5d9ee7c2b5ef1ace8e29436244c762e67) )
	ROM_LOAD16_BYTE( "35df", 0x000000, 0x020000, CRC(74bfb336) SHA1(03fce8fa82eaaa9c12f99b94a19f01143fce6ae6) )
	ROM_LOAD16_BYTE( "35f7", 0x000000, 0x020000, CRC(8153e8e2) SHA1(0eb27f81c18ea09f45bdecb6d5985b95108383dd) )
	ROM_LOAD16_BYTE( "35fe", 0x000000, 0x020000, CRC(5bfb041e) SHA1(b1009eaee844ea75665754f4026080f1497b421f) )
	ROM_LOAD16_BYTE( "35ff", 0x000000, 0x020000, CRC(b0655026) SHA1(b0e556bbd1450035dd0a373eaf01a09d9cf90c60) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "firecracker_club_sound.bin", 0x000000, 0x080000, CRC(0421526d) SHA1(9dad850c208cb9f4a3a4c62e05a18217466d227e) )
ROM_END


ROM_START( j6fivalv )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "facl-6s1.bin", 0x000000, 0x020000, CRC(c52940cf) SHA1(bdfb4719d265f429f58400169d48ecab18b89296) )
	ROM_LOAD16_BYTE( "facl-6s2.bin", 0x000001, 0x020000, CRC(028959e5) SHA1(b5c64e4ddafe33642708ab9e13a8092e989ba0d6) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "facl-6f1.bin", 0x000000, 0x020000, CRC(01f3a3df) SHA1(93ab023af070f2d3edce0a729803e70103b38747) )
	ROM_LOAD16_BYTE( "facl-6p1.bin", 0x000000, 0x020000, CRC(2eb714f7) SHA1(4e50e746c5c538ccd111fd40d8cb5cf25433f2f2) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "fivealivesnd.bin", 0x000000, 0x080000, CRC(4e4e94d9) SHA1(b8d1f241c4257436fd0e552494d2c9af1c8661dd) )
ROM_END


ROM_START( j6fiveln )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "39.bin", 0x000000, 0x020000, CRC(f960d6dd) SHA1(d69f868201e1cd7ccceb155f6c219aa81791e3a3) )
	ROM_LOAD16_BYTE( "40.bin", 0x000001, 0x020000, CRC(ee1163eb) SHA1(f0723b67343a1f0c4cc7c20d2177ef5c3e156aed) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "41.bin", 0x000000, 0x020000, CRC(12fe82e5) SHA1(7ff4e94c0207df73ac20a390500e5ca7bf035524) )
	ROM_LOAD16_BYTE( "42.bin", 0x000000, 0x020000, CRC(aba416bc) SHA1(31815dc414d94be7eaad5458a85f5a50c248ea99) )
	ROM_LOAD16_BYTE( "43.bin", 0x000000, 0x020000, CRC(9b0da755) SHA1(a0e1e8da3333f3361a27505b7ca251a2c586251a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "c-snd.bin", 0x000000, 0x080000, CRC(0016ab04) SHA1(82d133f485b325b29db901f6254c80ca959abd3e) )
ROM_END



ROM_START( j6frc10 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9445.bin", 0x000000, 0x020000, CRC(d9d99afc) SHA1(3d2072ee7596f5d9dec8fc77af5963266afc2a75) )
	ROM_LOAD16_BYTE( "9446.bin", 0x000001, 0x020000, CRC(50d87000) SHA1(02889506e8a009c85c73608db3196f88409007a1) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9447.bin", 0x000000, 0x020000, CRC(3247cec4) SHA1(3ec8619b4c1a75987b79f64263d9e17b6738a0fc) )
	ROM_LOAD16_BYTE( "9448.bin", 0x000000, 0x020000, CRC(8b1d5a9d) SHA1(8978660d6804a3886a0c08ce6bce2128491babbc) )
	ROM_LOAD16_BYTE( "9449.bin", 0x000000, 0x020000, CRC(bbb4eb74) SHA1(1b8f2290d332c273dd1f230cce62d96e890bc6bb) )
	ROM_LOAD16_BYTE( "f1015p1", 0x000000, 0x020000, CRC(404fd4c9) SHA1(fa74aaa5fe554d223592fbf3e6f59999bbe04ec8) )
	ROM_LOAD16_BYTE( "f1015p2", 0x000000, 0x020000, CRC(36508a26) SHA1(e31a3e2e5e1048e087ef112cd9ef949a4e60228b) )
	ROM_LOAD16_BYTE( "force1015-p1.bin", 0x000000, 0x020000, CRC(d9d99afc) SHA1(3d2072ee7596f5d9dec8fc77af5963266afc2a75) )
	ROM_LOAD16_BYTE( "force1015-p2.bin", 0x000000, 0x020000, CRC(50d87000) SHA1(02889506e8a009c85c73608db3196f88409007a1) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "force10snd.bin", 0x000000, 0x080000, CRC(e7e587c9) SHA1(fde7a7761253dc4133340b766d220873731c11c7) )
	ROM_LOAD( "fo10-snd.bin", 0x000000, 0x080000, CRC(e7e587c9) SHA1(fde7a7761253dc4133340b766d220873731c11c7) )
ROM_END



ROM_START( j6framft )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "frame.p1.bin", 0x000001, 0x020000, CRC(0bfaa4ff) SHA1(5080b3ce9692fe10b7e9e3fd75390513bbb60f99) )
	ROM_LOAD16_BYTE( "frame.p2.bin", 0x000000, 0x020000, CRC(a227fe30) SHA1(4b6034ea46c5c482e8b631af77ff6aac381eb941) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "frame_snd.bin", 0x000000, 0x080000, CRC(8053766a) SHA1(429e1476c7854a8a4d73aed0ec5a5efb31e6da4e) )
ROM_END


ROM_START( j6frtmch )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fruitmachine.p1", 0x000000, 0x020000, CRC(343f7fbc) SHA1(2d6594556cf4defb107d3a74c54ad9e1d2df63c6) )
	ROM_LOAD16_BYTE( "fruitmachine.p2", 0x000001, 0x020000, CRC(34c71b1f) SHA1(afc15df46fbcd845b963adcfe76764eba58da1e3) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "fruitmachinesnd.bin", 0x000000, 0x080000, CRC(24df2399) SHA1(8ed92b710d866b852143989f2f8d84da90cd1a63) )
ROM_END




ROM_START( j6frtpot )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fruitpots.p1", 0x000000, 0x080000, CRC(c8c5ebf7) SHA1(39040b08d6b67723388c4d90433f0965637590fb) )
	ROM_LOAD16_BYTE( "fruitpots.p2", 0x000001, 0x080000, CRC(5922976b) SHA1(a11ced9f363a085c5e77e5a85d5bbc785b6600c0) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "60000200.evn", 0x000000, 0x080000, CRC(1cd3d39f) SHA1(9cd873c16c5698e7fe9e07bbd05481e5edb8a0e0) )
	ROM_LOAD16_BYTE( "60000200.odd", 0x000000, 0x080000, CRC(7e338bc0) SHA1(b970815d7cb951a349a4ddafcb834fa3c793a2d8) )
	ROM_LOAD16_BYTE( "60001201.evn", 0x000000, 0x080000, CRC(42bd9e8e) SHA1(11e9e1fa94651ed566075244a3718966eff9c86c) )
	ROM_LOAD16_BYTE( "60001201.odd", 0x000000, 0x080000, CRC(7cfc531a) SHA1(583b58793184001d0e66b7a7922ecc26e5f78b79) )
	ROM_LOAD16_BYTE( "60001202.evn", 0x000000, 0x080000, CRC(e586058b) SHA1(4322dc1f21a17fcec8dbface7d0a545dc95191ec) )
	ROM_LOAD16_BYTE( "60001202.odd", 0x000000, 0x080000, CRC(2d70ab1d) SHA1(e1c9ad7b498ea2de8bc82631ab5468bad7ff4225) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "fruitpotssnd.bin", 0x000000, 0x080000, CRC(1aacc429) SHA1(7ee38a34087a05d06fbfff78b57bf794c4f25d0c) )
ROM_END


ROM_START( j6gforce )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gfor-3s1.bin", 0x000000, 0x020000, CRC(a289af04) SHA1(d8a5de1ea8dddaf276693b7f1858442211ca0d79) )
	ROM_LOAD16_BYTE( "gfor-3s2.bin", 0x000001, 0x020000, CRC(fc02d84f) SHA1(80f1066896a2e24c202b9754c04b542b909f6658) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "gfor-3a1.bin", 0x000000, 0x020000, CRC(f04d6f65) SHA1(72dcd029e04afeed849fe271bafb731c8735420f) )
	ROM_LOAD16_BYTE( "gfor-3p1.bin", 0x000000, 0x020000, CRC(4917fb3c) SHA1(d2badd3cc06cadff62a3e1e43e44ff5fe084c00b) )
	ROM_LOAD16_BYTE( "gfor-3wp.bin", 0x000000, 0x020000, CRC(2b7a8ab4) SHA1(ffa0cb80194add811ccddb80fffcf8fa91491ec0) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gfor-snd.bin", 0x000000, 0x080000, CRC(4b710c8a) SHA1(af93c795d4c46cb95d92c48ac60a48db7f6724ac) )
ROM_END

ROM_START( j6gforceb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "g_force.p1",   0x000000, 0x020000, CRC(724afeba) SHA1(b7eca2138b7c04031fa3b6a35a91180a7b487920) )
	ROM_LOAD16_BYTE( "g_force.p2",   0x000001, 0x020000, CRC(bc5a491e) SHA1(a8df6f26396c6d54b9de82717231f56342373516) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gfor-snd.bin", 0x000000, 0x080000, CRC(4b710c8a) SHA1(af93c795d4c46cb95d92c48ac60a48db7f6724ac) )
ROM_END

ROM_START( j6gidogh )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gidough.p1", 0x000000, 0x020000, CRC(92b2ad84) SHA1(710630c05dafeaaa65d913348ac545d1d74e900e) )
	ROM_LOAD16_BYTE( "gidough.p2", 0x000001, 0x020000, CRC(a997c758) SHA1(0ddd76d78e5efe4ae1e044eedf2d3710bde99224) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gidoughsnd.bin", 0x000000, 0x080000, CRC(69339dd1) SHA1(e69445471ff5f102d1a234f0859b01b3f7a82498) )
ROM_END


ROM_START( j6guab )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8574.bin", 0x000000, 0x020000, CRC(a58dc7e1) SHA1(b853217ccbae59e9485931464dd808d2684c331a) )
	ROM_LOAD16_BYTE( "8575.bin", 0x000001, 0x020000, CRC(dd8ff2cb) SHA1(400693667f6b459421a9589546b113286c58508b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8576.bin", 0x000000, 0x020000, CRC(4e1393d9) SHA1(8d88bcb8ff267de916b279ce5347963d88081cf6) )
	ROM_LOAD16_BYTE( "8577.bin", 0x000000, 0x020000, CRC(c7e0b669) SHA1(2e2baa1c7b3d91a2af63eb97999b1a15a65a9b0a) )
	ROM_LOAD16_BYTE( "9305.bin", 0x000000, 0x020000, CRC(cfaab78e) SHA1(86161fe76d7d462dc3d6b8e466850bce14ba21ed) )
	ROM_LOAD16_BYTE( "9306.bin", 0x000000, 0x020000, CRC(1273a76c) SHA1(f2fcb3cb2780c8dd530727cffda32700fa046060) )
	ROM_LOAD16_BYTE( "9307.bin", 0x000000, 0x020000, CRC(2434e3b6) SHA1(d09bb8dfd51fd2b52a277e92009623587bd14399) )
	ROM_LOAD16_BYTE( "9308.bin", 0x000000, 0x020000, CRC(9d6e77ef) SHA1(b5f6a9acb31259d2f57d1bf3d019a92fd52c424a) )
	ROM_LOAD16_BYTE( "9309.bin", 0x000000, 0x020000, CRC(adc7c606) SHA1(4ee51b8ec543ab8efe62ed95b8a30248c24467b0) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "guabs.hex", 0x000000, 0x080000, CRC(fc041c87) SHA1(bd2606e3a67e13ce937b8cb4d5fcda9fa13842a1) )
ROM_END


ROM_START( j6guabcl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8841.bin", 0x000000, 0x020000, CRC(8483bf47) SHA1(08726c55a1064ecb392e904b748f74032e77a3c9) )
	ROM_LOAD16_BYTE( "8842.bin", 0x000001, 0x020000, CRC(f203d5ec) SHA1(5748e5f482f771de3348217170e7fa0d4986048e) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8843.bin", 0x000000, 0x020000, CRC(6f1deb7f) SHA1(53f00ac5552838448e0329928e70db3c98fdd65f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "guabs.hex", 0x000000, 0x080000, CRC(fc041c87) SHA1(bd2606e3a67e13ce937b8cb4d5fcda9fa13842a1) )
ROM_END


ROM_START( j6gldclb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clgl-as1.bin", 0x000000, 0x020000, CRC(16b8560f) SHA1(68d3577bc14a60ca2c19091c05d1a65b7eae6747) )
	ROM_LOAD16_BYTE( "clgl-as2.bin", 0x000001, 0x020000, CRC(dbcf8013) SHA1(39247fcf209b1007cde1e161d72165eeb239a23c) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "clgl-af1.bin", 0x000000, 0x020000, CRC(d262b51f) SHA1(072638e672e9ace6bac08c23073c8400d7d2315a) )
	ROM_LOAD16_BYTE( "clgl-ap1.bin", 0x000000, 0x020000, CRC(fd260237) SHA1(08873e61f75227b73ea030f6a63a7f5917552f7b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gladiatorsnd.bin", 0x000000, 0x080000, CRC(13bd21c7) SHA1(3e0e087fdf8566ca6803f8f9f75597e19433fd0b) )
ROM_END


ROM_START( j6gogold )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20702.bin", 0x000000, 0x020000, CRC(c274df22) SHA1(f623bb8ba2afcc3ad7c58a4cf56ea8d8f9d1308a) )
	ROM_LOAD16_BYTE( "20703.bin", 0x000001, 0x020000, CRC(aa2a1e67) SHA1(86fc1962a4de05f3eca8ca0b02d04db005e8a174) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20704.bin", 0x000000, 0x020000, CRC(29ea8b1a) SHA1(da066336de791891a35201e51f92e1cd4190f488) )
	ROM_LOAD16_BYTE( "20705.bin", 0x000000, 0x020000, CRC(90b01f43) SHA1(ff28bbee32f5d2192a6ea391dd4f5103c1f12296) )
	ROM_LOAD16_BYTE( "20706.bin", 0x000000, 0x020000, CRC(a019aeaa) SHA1(e7d83c0b4b232687ed6491620e0b22cc93e60265) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gfgoldsnd.bin", 0x000000, 0x080000, CRC(1ccc9b9b) SHA1(d6c7d4285b569c8ed77f732d6e42e6b763d200d4) )
ROM_END


ROM_START( j6gldmin )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gm1_1.p1", 0x000000, 0x020000, CRC(a6a135bb) SHA1(2a143fd0f4b6227b2a528a4c65865ecd781706dd) )
	ROM_LOAD16_BYTE( "gm1_1.p2", 0x000001, 0x020000, CRC(6027f0f7) SHA1(f68a5f33c3d2f04dc892a0be115594e5aa577682) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gmsnd.bin", 0x000000, 0x080000, CRC(23913559) SHA1(3c71eea6f847a6eb16f76a29555c9fde5790929a) )
ROM_END


ROM_START( j6gldday )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "goldenday1.bin", 0x000000, 0x020000, CRC(8f2aa8a7) SHA1(ea5b8c1418deaaf2bda58a40cacca3c77d6d5a08) )
	ROM_LOAD16_BYTE( "goldenday2.bin", 0x000001, 0x020000, CRC(2a7658ab) SHA1(4286a4a76b8d95a4da4e8aad2f81b091d2d2f96a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gdsound.bin", 0x000000, 0x04a018, CRC(796e1b35) SHA1(e9c8e5a350823275c9ba9238781872ea359d5049) )
ROM_END


ROM_START( j6golddm )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "gd30cz03_1.b8", 0x000000, 0x020000, CRC(bf901be5) SHA1(c508706f77ff23086d5507823fa29784f9c2d83c) )
	ROM_LOAD16_BYTE( "gd30cz03_2.b8", 0x000001, 0x020000, CRC(93bd2009) SHA1(a0e2ffb9dfad123e884507e3df26aa3d457788ff) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "gd75cz03_1.b8", 0x000000, 0x020000, CRC(b3ddb5d4) SHA1(dbdf8f9c558355a20fce83470c20768358b7c819) )
	ROM_LOAD16_BYTE( "gd75cz03_1.bin", 0x000000, 0x020000, CRC(b3ddb5d4) SHA1(dbdf8f9c558355a20fce83470c20768358b7c819) )
	ROM_LOAD16_BYTE( "gd75cz03_2.bin", 0x000000, 0x020000, CRC(93bd2009) SHA1(a0e2ffb9dfad123e884507e3df26aa3d457788ff) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "gdsnd.bin", 0x000000, 0x04a018, CRC(796e1b35) SHA1(e9c8e5a350823275c9ba9238781872ea359d5049) )
ROM_END


ROM_START( j6goldgl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "goal-11.bin", 0x000000, 0x020000, CRC(124870c1) SHA1(7f42ae51f342beaf0c53f46df437ea81772f1005) )
	ROM_LOAD16_BYTE( "goal-12.bin", 0x000001, 0x020000, CRC(5e292400) SHA1(74d6a480881b4fb5deac921517e8c07586ade4f3) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "2412", 0x000000, 0x020000, CRC(54a6ab53) SHA1(24ae5a00fea82a910494fedb80546fe7d0b65313) )
	ROM_LOAD16_BYTE( "244a", 0x000000, 0x020000, CRC(3754d3f6) SHA1(a2106953e1b89b18f78052fcdb34bc8ac656f050) )
	ROM_LOAD16_BYTE( "2451", 0x000000, 0x020000, CRC(edfc3f0a) SHA1(2acfd13b3d77c682f204797114389e06296dddd7) )
	ROM_LOAD16_BYTE( "2452", 0x000000, 0x020000, CRC(06626b32) SHA1(2245bc58df15a455dfc47e66afc82e4230f0088a) )
	ROM_LOAD16_BYTE( "8832", 0x000000, 0x020000, CRC(24eb5d3a) SHA1(3a26cf8f80c7843495fbdc93a4d31876206d0004) )
	ROM_LOAD16_BYTE( "gogo-11.bin", 0x000000, 0x020000, CRC(f73cd3e9) SHA1(4068eb63f696910a0d04da3bad727f0d2225fb47) )
	ROM_LOAD16_BYTE( "gogo-12.bin", 0x000000, 0x020000, CRC(7662ec60) SHA1(1ea6c11ab0c58b92211cb7feded258ff33c4e592) )
	ROM_LOAD16_BYTE( "gogo-1a1.bin", 0x000000, 0x020000, CRC(a5f81388) SHA1(dd832fcbc63e967213bf133a50337a877f18f413) )
	ROM_LOAD16_BYTE( "gogo-1p1.bin", 0x000000, 0x020000, CRC(1ca287d1) SHA1(eda47e49a1f832c3cff096d3a1fe904d0bf2a2b3) )
	ROM_LOAD16_BYTE( "gogo-31.bin", 0x000000, 0x020000, CRC(8d4c0b6b) SHA1(ce9bbb28e0c3e92693dbf1fbd2562bd697d54431) )
	ROM_LOAD16_BYTE( "gogo-32.bin", 0x000000, 0x020000, CRC(9c36cb43) SHA1(bbc143d3727130e123fffd295422fdfc76c85d12) )
	ROM_LOAD16_BYTE( "gogo-3a1.bin", 0x000000, 0x020000, CRC(df88cb0a) SHA1(871e1ac825851f0d61c2dd5a9f2f2c768990032f) )
	ROM_LOAD16_BYTE( "gogo-3l1.bin", 0x000000, 0x020000, CRC(bc7ab3af) SHA1(fd539a5d9c9e1e3233782c278c465e6789414563) )
	ROM_LOAD16_BYTE( "gogo-3p1.bin", 0x000000, 0x020000, CRC(66d25f53) SHA1(9bb788fb1330ff3dc6c9abd166adcaafd67ee56b) )
	ROM_LOAD16_BYTE( "gogo3lp1.bin", 0x000000, 0x020000, CRC(57e4e797) SHA1(3fac191bc880596427ce563f045227f181e3c2df) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "goldengoalsnd.bin", 0x000000, 0x080000, CRC(3af9ccdb) SHA1(4a911a48816bc69743ba1ba18fdd913041636ae1) )
ROM_END


ROM_START( j6hapyhr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "happyhour.p1", 0x000000, 0x020000, CRC(0a320ed4) SHA1(5bf54339717694febbf6749c39985fc7ff4c194f) )
	ROM_LOAD16_BYTE( "happyhour.p2", 0x000001, 0x020000, CRC(3de94b07) SHA1(ce1a712845ccc5fa9ef92b3d07f8872afeec88f8) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20426.bin", 0x000000, 0x020000, CRC(58f6ceb5) SHA1(5ca6861d1532ede848f672fe08152dcd0f65be95) )
	ROM_LOAD16_BYTE( "20427.bin", 0x000000, 0x020000, CRC(3de94b07) SHA1(ce1a712845ccc5fa9ef92b3d07f8872afeec88f8) )
	ROM_LOAD16_BYTE( "20428.bin", 0x000000, 0x020000, CRC(b3689a8d) SHA1(4f690ec96f1b5e0ed30023016de767c132356430) )
	ROM_LOAD16_BYTE( "20429.bin", 0x000000, 0x020000, CRC(0a320ed4) SHA1(5bf54339717694febbf6749c39985fc7ff4c194f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "happyhoursnd.bin", 0x000000, 0x080000, CRC(ef80bbfd) SHA1(66dc0bd35054a506dc75972ac59f9ca03d886e1b) )
ROM_END




ROM_START( j6hifly )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hiflyer.p1", 0x000000, 0x020000, CRC(5309d1ec) SHA1(49df13b33dd67a8ed63cdd9b83d6c58b34f29d67) )
	ROM_LOAD16_BYTE( "hiflyer.p2", 0x000001, 0x020000, CRC(6cae4a62) SHA1(acf196220dca131bc274d91164204aeeec8fc08c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "hiflyersnd.bin", 0x000000, 0x080000, CRC(a0f1b9d2) SHA1(738e9a6ac1d8af1e1610d96195a86ce34fe53e33) )
ROM_END


ROM_START( j6impact )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hiim-2s1.bin", 0x000000, 0x020000, CRC(9cfa16b2) SHA1(520d38ec96652914c6506c55fd59c01fcb8a67c4) )
	ROM_LOAD16_BYTE( "hiim-2s2.bin", 0x000001, 0x020000, CRC(8a087a6d) SHA1(38463d0a474f3a2d137b7e2a825d8e65d9d043c0) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hiim-2a1.bin", 0x000000, 0x020000, CRC(ce3ed6d3) SHA1(a059a2f279df4f8432a9e7fa780ed174d63d2ac5) )
	ROM_LOAD16_BYTE( "hiim-2p1.bin", 0x000000, 0x020000, CRC(7764428a) SHA1(4f83a0d3ce7c484617122191a66e90432aa39e4e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "hiim-snd.bin", 0x000000, 0x080000, CRC(3f54a54c) SHA1(fb3b2561f10391f01ee97e4501e8492fcfe4fd2b) )
ROM_END

ROM_START( j6impactb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hi_imp.p1",    0x000000, 0x020000, CRC(d742d6c9) SHA1(cf4ccc22a2cabfe06339ab079f7e5b9bb6297e8b) )
	ROM_LOAD16_BYTE( "hi_imp.p2",    0x000001, 0x020000, CRC(9c3de3c5) SHA1(428d101146a99ae713251ccf070049c0985b577b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "hiim-snd.bin", 0x000000, 0x080000, CRC(3f54a54c) SHA1(fb3b2561f10391f01ee97e4501e8492fcfe4fd2b) )
ROM_END

ROM_START( j6hilosv )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9287.bin", 0x000000, 0x020000, CRC(9ea4dec0) SHA1(14a9239fb3a94f8f3c4a46c5f1d189a3f4a54868) )
	ROM_LOAD16_BYTE( "9288.bin", 0x000001, 0x020000, CRC(97184aba) SHA1(5b95e138998e78a8bf7f12186fd9f1849d2efa7b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9289.bin", 0x000000, 0x020000, CRC(753a8af8) SHA1(258c390235a45d9b08f13e975807a379f558453e) )
	ROM_LOAD16_BYTE( "9290.bin", 0x000000, 0x020000, CRC(cc601ea1) SHA1(6a5637dd37c83355817294090596ed5ed678509e) )
	ROM_LOAD16_BYTE( "9291.bin", 0x000000, 0x020000, CRC(fcc9af48) SHA1(3f1cd018b48c7fe55bb16981d00b69892dadd241) )

	// from a 'crystal' set
	ROM_LOAD16_BYTE( "20575.bin", 0x000000, 0x020000, CRC(97958436) SHA1(8d6afa9431a217eb022223ba5875321b3e6a9546) )
	ROM_LOAD16_BYTE( "20577.bin", 0x000000, 0x020000, CRC(c5514457) SHA1(a6cfcb2b22f24fa994f9913c6efbb0495c77e6f8) )


// this looks like something else
//  ROM_LOAD16_BYTE( "hihosilver1.1.bin", 0x000000, 0x008000, CRC(e8b5532d) SHA1(45a62028649cde884f313cfabadee994e13f1ea3) )
//  ROM_LOAD16_BYTE( "hihosilver1.2.bin", 0x000000, 0x008000, CRC(8cffe4ac) SHA1(c81ea16fca08f6f2daa8ff629a0f53e7a641c792) )
//  ROM_LOAD16_BYTE( "hihosilver1.3.bin", 0x000000, 0x008000, CRC(ea18c31f) SHA1(a6ac9e2e70cb156e453821723f3eed23b4ade3c4) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "hi_lo_silver_snd.bin", 0x000000, 0x080000, CRC(fd88e3a6) SHA1(07c2fec617faea189ceddc46ec477fb09c0ec4a9) )

ROM_END


ROM_START( j6hirol )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hiro-1s1.bin", 0x000000, 0x020000, CRC(98e426ce) SHA1(16140aad2efa8b19eedb411909dccdb5ca5561cf) )
	ROM_LOAD16_BYTE( "hiro-1s2.bin", 0x000001, 0x020000, CRC(6b903488) SHA1(c62e3f5446c62d45926190d48fb2a34916d1f098) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hiro-1a1.bin", 0x000000, 0x020000, CRC(ca20e6af) SHA1(a6b2e94cf13b2f5628426d4b26f44c69b45896bc) )
	ROM_LOAD16_BYTE( "hiro-1p1.bin", 0x000000, 0x020000, CRC(737a72f6) SHA1(d2658e91b283ec8fe860d4dc9a130afcaac525bb) )
	ROM_LOAD16_BYTE( "hiro-1w1.bin", 0x000000, 0x020000, CRC(fa895746) SHA1(a5ad82d76c96004d578cc217a46b0eef78e867ac) )
	ROM_LOAD16_BYTE( "hiro-1wp.bin", 0x000000, 0x020000, CRC(1117037e) SHA1(8488ef64d50d78a55857d24a0d25e4060e34c35e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "hiro-snd.bin", 0x000000, 0x080000, CRC(5843c195) SHA1(0665e913e4c1a919aa5331cce7a467c841722388) )
ROM_END


ROM_START( j6hirlcl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hrcl-3_1.bin", 0x000000, 0x020000, CRC(680182bf) SHA1(54e1b6db179934a29453ff5c5664fd00352377ea) )
	ROM_LOAD16_BYTE( "hrcl-3_2.bin", 0x000001, 0x020000, CRC(c096ad69) SHA1(1a778f4a54bc98db32e5df20fb74318bea00f6d3) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hrcl-3f1.bin", 0x000000, 0x020000, CRC(acdb61af) SHA1(80b372c584aefd0534ed0be2c5f0ac44341bde18) )
	ROM_LOAD16_BYTE( "hrcl-3n1.bin", 0x000000, 0x020000, CRC(0a6cf337) SHA1(812f87724ebd6f5373ea4914efce5ebd64a2cb35) )
	ROM_LOAD16_BYTE( "hrcl-3p1.bin", 0x000000, 0x020000, CRC(839fd687) SHA1(c0259308321c8f1d8c1ba1dad333e223c06e9c4a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "hrcl-snd.bin", 0x000000, 0x080000, CRC(5843c195) SHA1(0665e913e4c1a919aa5331cce7a467c841722388) )
ROM_END


ROM_START( j6histk )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "60000082.p1", 0x000000, 0x020000, CRC(37de3dd8) SHA1(3d5aaf9112ca79eeb72bcbd78cf5a89c26ecc9e1) )
	ROM_LOAD16_BYTE( "60000082.p2", 0x000001, 0x020000, CRC(91614eea) SHA1(7869d9f0326ef5b3b7841a4430e7f90dcb6dfb96) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "histakes.p1", 0x000000, 0x020000, CRC(8a021897) SHA1(7b8bd6add7f3341a719a384680615abe8e19cbac) )
	ROM_LOAD16_BYTE( "histakes.p2", 0x000000, 0x020000, CRC(12467e9d) SHA1(d2f3caeaf63392f2b6ba157e054cb40c1a73c19d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "histakessnd.bin", 0x000000, 0x080000, CRC(7bffa191) SHA1(e3a4a4eef878fb093240a3e145cf405d266bec74) )
ROM_END


ROM_START( j6hiphop )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hiho-11.bin", 0x000000, 0x020000, CRC(163aa788) SHA1(a7047fa9b6273eb5749195914c098a524e0fb68a) )
	ROM_LOAD16_BYTE( "hiho-12.bin", 0x000001, 0x020000, CRC(8d8cb4b4) SHA1(6baea853b5e0bc349980d3ef7d93a3f4c146492a) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hiho-1a1.bin", 0x000000, 0x020000, CRC(44fe67e9) SHA1(f8089203051ab3c54959bffd91064042b4ddcd55) )
	ROM_LOAD16_BYTE( "hiho-1n1.bin", 0x000000, 0x020000, CRC(270c1f4c) SHA1(f73c96dc08f3eb6d8493476f1323d4df30e1dc8a) )
	ROM_LOAD16_BYTE( "hiho-1p1.bin", 0x000000, 0x020000, CRC(fda4f3b0) SHA1(01702290b1c6ac25a221b24e2607aa68205284eb) )
	ROM_LOAD16_BYTE( "hiho1np1.bin", 0x000000, 0x020000, CRC(cc924b74) SHA1(2a82f30252a695f7bc196f6699b0871b938b7fdd) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6hotsht )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hosh-4_1.bin", 0x000000, 0x020000, CRC(833c49e9) SHA1(94552fc3c2c246412e4c62e5095c1fc3707fd73c) )
	ROM_LOAD16_BYTE( "hosh-4_2.bin", 0x000001, 0x020000, CRC(645c832d) SHA1(174dc5491d567efd37051ca10987e24c8ccea5e8) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "044p3-0n.bin", 0x000000, 0x020000, CRC(02abba15) SHA1(8073da7ea300fe5b89e6b317c6e051171fccb4a1) )
	ROM_LOAD16_BYTE( "hosh-4a1.bin", 0x000000, 0x020000, CRC(d1f88988) SHA1(5063408d5586d0dedb535c47ce725ab7365c7eef) )
	ROM_LOAD16_BYTE( "hosh-4p1.bin", 0x000000, 0x020000, CRC(68a21dd1) SHA1(68700e0c30e715795e604d1ff7606e03c4d93362) )
	ROM_LOAD16_BYTE( "hosh-4w1.bin", 0x000000, 0x020000, CRC(e1513861) SHA1(8c945a6c52670754ddeece94c0ebdb046f238405) )
	ROM_LOAD16_BYTE( "hosh-51.bin", 0x000000, 0x020000, CRC(d54e81eb) SHA1(2e67f0f3ee2fceeb56c93f6a5140395d341bf414) )
	ROM_LOAD16_BYTE( "hosh-52.bin", 0x000000, 0x020000, CRC(39b2798d) SHA1(9d66f99b4b3ffaa07018e4c70bb83903d89c0a50) )
	ROM_LOAD16_BYTE( "hosh-5a1.bin", 0x000000, 0x020000, CRC(878a418a) SHA1(32d6a5c83d65132ca4f8646661f743b3e8844224) )
	ROM_LOAD16_BYTE( "hosh-5n1.bin", 0x000000, 0x020000, CRC(e478392f) SHA1(2c973dfc64e43aed2ec46fcb81fe1b50783daf68) )
	ROM_LOAD16_BYTE( "hosh-5p1.bin", 0x000000, 0x020000, CRC(3ed0d5d3) SHA1(d04ca69a9e3b30a64c2bad1c4e263b2730ae3fc2) )
	ROM_LOAD16_BYTE( "prom1n.bin", 0x000000, 0x020000, CRC(df747d47) SHA1(68d28db3fc2bb806f88ee284a8051e15b32cf11e) )
	ROM_LOAD16_BYTE( "prom1p_0.bin", 0x000000, 0x020000, CRC(e935ee2d) SHA1(0d5abd6e3f5d04f5874015a158446d0598db8ec1) )
	ROM_LOAD16_BYTE( "prom1p_1.bin", 0x000000, 0x020000, CRC(34ea297f) SHA1(2e7476a9dc4e87a219ed3ce0fbe7b861e8a4832e) )
	ROM_LOAD16_BYTE( "prom2_0.bin", 0x000000, 0x020000, CRC(8d89d7eb) SHA1(b69501f160d9980558a342e1d1a3e4102e9b9c33) )
	ROM_LOAD16_BYTE( "prom2_1.bin", 0x000000, 0x020000, CRC(22bf5681) SHA1(4f142dee31dc00e5ecf0a2305282766b368f4c33) )
	ROM_LOAD( "hotshot.p1", 0x0000, 0x020000, CRC(4a15d870) SHA1(518fe05ee3b7039827271659777535c143f6fcc1) )
	ROM_LOAD( "hotshot.p2", 0x0000, 0x020000, CRC(5b814e89) SHA1(214c8ed823089a2c53b53b6ce85863dc3cd38da2) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "hosh-snd.bin", 0x000000, 0x080000, CRC(f5bcfe63) SHA1(4983cb4c2d69730d7f1984d648c2801b46b4ab70) )
ROM_END


ROM_START( j6impuls )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "imp15p1", 0x000000, 0x020000, CRC(115aa0fc) SHA1(0ff0c1b87ba1c6e9d875857dcbf9e67174e86962) )
	ROM_LOAD16_BYTE( "imp15p2", 0x000001, 0x020000, CRC(f9b0a1f5) SHA1(0833ffe044cdefff7836fc00e985e5b0d7e9f827) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "imp6p1", 0x000000, 0x020000, CRC(2abd3397) SHA1(36d6286d6de6c1b205ca1e593118bf5ef15e108b) )
	ROM_LOAD16_BYTE( "imp6p1s", 0x000000, 0x020000, CRC(2abd3397) SHA1(36d6286d6de6c1b205ca1e593118bf5ef15e108b) )
	ROM_LOAD16_BYTE( "imp6p2", 0x000000, 0x020000, CRC(6d8ae792) SHA1(9e703f3e4c6e74866f8a6c187851c416905dc076) )
	ROM_LOAD16_BYTE( "imp6p2s", 0x000000, 0x020000, CRC(6d8ae792) SHA1(9e703f3e4c6e74866f8a6c187851c416905dc076) )
	ROM_LOAD16_BYTE( "impu-7a1.bin", 0x000000, 0x020000, CRC(c1c7fad1) SHA1(a28db0949be931192ebacf6dc67a1b652fd048c2) )
	ROM_LOAD16_BYTE( "impu-7p1.bin", 0x000000, 0x020000, CRC(789d6e88) SHA1(0b65f00b6fed5fbdd89d9ba06ebe7d17a97344d6) )
	ROM_LOAD16_BYTE( "impu-7s1.bin", 0x000000, 0x020000, CRC(93033ab0) SHA1(6b94c72ba09a2b3bf343f199a61871f18b67ed10) )
	ROM_LOAD16_BYTE( "impu-7s2.bin", 0x000000, 0x020000, CRC(cead0007) SHA1(f5d701bd2f1d85fd907666d0fbe217dbeaae1ba7) )
	ROM_LOAD16_BYTE( "impu-7wp.bin", 0x000000, 0x020000, CRC(1af01f00) SHA1(0d3fcaa1105a5dd00d4154027dcdac6b35eb2342) )
	ROM_LOAD16_BYTE( "impu6p1a", 0x000000, 0x020000, CRC(7879f3f6) SHA1(a7e5e55946ed63bf7fd84ac8cce46102a850bae4) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "impu-snd.bin", 0x000000, 0x080000, CRC(0551d030) SHA1(7a8d012690bcea707710bf39c8069d7c074912ce) )
ROM_END


ROM_START( j6indy )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "6810.bin", 0x000000, 0x020000, CRC(67f9cf6a) SHA1(f5e63b2135f9b251bb092e2738ab280581792a08) )
	ROM_LOAD16_BYTE( "6811.bin", 0x000001, 0x020000, CRC(6efc0ce8) SHA1(2f1bc1dfd6d1df019f180e6477e524811bf7295c) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7493.bin", 0x000000, 0x020000, CRC(21ca4ac6) SHA1(37c78df9dcace53eeab72ea37a49d27056d35043) )
	ROM_LOAD16_BYTE( "7494.bin", 0x000000, 0x020000, CRC(4ddf626b) SHA1(4bbfd86530cd1a8b7b4da4e9b36d0e1d61e5d120) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "6706.bin", 0x000000, 0x080000, CRC(674c5b21) SHA1(12e12e362ae8c99414cd474a34fa13acd8f6bcb3) )
	ROM_LOAD( "indisnd.bin", 0x0000, 0x080000, CRC(90ff139b) SHA1(9555553dc01055c311d4917e6ed7f5d3b6bf3b71) )
ROM_END


ROM_START( j6showtm )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "itsshowtime.p1", 0x000000, 0x020000, CRC(5e744b6a) SHA1(db653618e4c2b86634bb10795bd6c3ad3a1b199e) )
	ROM_LOAD16_BYTE( "itsshowtime.p2", 0x000001, 0x020000, CRC(ae952cf9) SHA1(e27c616e74eb139daf98479fed0cc1bfcf5619b5) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "its showtime v1-2 (27c010)", 0x000000, 0x020000, CRC(e5db5a67) SHA1(6bdd8f48edb6c6dce9dc6ac742825b30e2efda40) )
	ROM_LOAD16_BYTE( "its showtime v1a1 (27c010)", 0x000000, 0x020000, CRC(c10295cf) SHA1(076f25a24a08e0d3657ac90f54a29af1e22603d9) )
	ROM_LOAD16_BYTE( "its showtime v2-1 (27c010)", 0x000000, 0x020000, CRC(b5ea1f52) SHA1(08d6bc1add1e659b59e2edc9de88dd976587ece3) )
	ROM_LOAD16_BYTE( "its showtime v2-2 (27c010)", 0x000000, 0x020000, CRC(ae952cf9) SHA1(e27c616e74eb139daf98479fed0cc1bfcf5619b5) )
	ROM_LOAD16_BYTE( "itsh-4a1.bin", 0x000000, 0x020000, CRC(1da0cb3e) SHA1(30b7bf37cbd8c3372ae9ea1e114c4d7cc27ec131) )
	ROM_LOAD16_BYTE( "itsh-4n1.bin", 0x000000, 0x020000, CRC(7e52b39b) SHA1(d81a6d50c7596be53f7feaa4bf975978afb22dde) )
	ROM_LOAD16_BYTE( "itsh-4p1.bin", 0x000000, 0x020000, CRC(a4fa5f67) SHA1(b038b614ba62f765018b27f0180a11979d84f45b) )
	ROM_LOAD16_BYTE( "itsh-61.bin", 0x000000, 0x020000, CRC(0385bc13) SHA1(d627fe6083bc0cec726a06e4237f1f2b34cc44b1) )
	ROM_LOAD16_BYTE( "itsh-62.bin", 0x000000, 0x020000, CRC(cc164769) SHA1(c0cbcf8bf4530882a46cfffa9e1b90eea30284e7) )
	ROM_LOAD16_BYTE( "itsh-6a1.bin", 0x000000, 0x020000, CRC(51417c72) SHA1(261a7d90842f2d1b438655b8f694cef6b616a09f) )
	ROM_LOAD16_BYTE( "itsh-6l1.bin", 0x000000, 0x020000, CRC(32b304d7) SHA1(beea0699c8dd4c4aefaee539b4ba93815803e2cb) )
	ROM_LOAD16_BYTE( "itsh-6p1.bin", 0x000000, 0x020000, CRC(e81be82b) SHA1(51738c89fd7017d52237085bf3a1f0dbcc0610b0) )
	ROM_LOAD16_BYTE( "itsh4np1.bin", 0x000000, 0x020000, CRC(95cce7a3) SHA1(6e848fae84a9a053a1aa91b5b9deb9c7f9ad5ebc) )
	ROM_LOAD16_BYTE( "itsh6lp1.bin", 0x000000, 0x020000, CRC(d92d50ef) SHA1(e0d0f68890d271c1c30f0ac78ea86a37eda41afe) )
	ROM_LOAD16_BYTE( "itsh-41.bin", 0x000000, 0x020000, CRC(4f640b5f) SHA1(20a893d9a0ba10fc0bf63380e754e141d795759c) )
	ROM_LOAD16_BYTE( "itsh-42.bin", 0x000001, 0x020000, CRC(7a14783a) SHA1(804b1438cb5b3502fee0abf8a29174efc64e6dd6) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "itsshowtimesnd.bin", 0x000000, 0x080000, CRC(7a2264fa) SHA1(55d5a15ff8c6a76c5403856bb8e64cbfdafb7a55) )
ROM_END


ROM_START( j6jackjs )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "60000150.evn", 0x000000, 0x080000, CRC(358d59c8) SHA1(9212565a1f9d50d26d4ccfff747824c42e878e52) )
	ROM_LOAD16_BYTE( "60000150.odd", 0x000001, 0x080000, CRC(4a3473c8) SHA1(4ec3367008a8e1f34a7d502a9c9387d1b6de6e98) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "60001150.evn", 0x000000, 0x080000, CRC(0df9c631) SHA1(b205fbdf0bf59b604ff6ca4af7f41f5128db9d45) )
	ROM_LOAD16_BYTE( "60001150.odd", 0x000000, 0x080000, CRC(6605981a) SHA1(a94fa61ba1f9c4e95416837b5dfe55092ef8b290) )
	ROM_LOAD16_BYTE( "60001151.evn", 0x000000, 0x080000, CRC(e13db919) SHA1(49426dfd450bbf0ffd80fb416d8ef91ac09e709d) )
	ROM_LOAD16_BYTE( "60001151.odd", 0x000000, 0x080000, CRC(d1157eac) SHA1(76504b803fa46b707e78bc50030e2dc9a64fc209) )
	ROM_LOAD16_BYTE( "jj.p1", 0x000000, 0x080000, CRC(22ec7048) SHA1(7e806963d0aeccc1c92579eebf2571dd9f29c263) )
	ROM_LOAD16_BYTE( "jj.p2", 0x000000, 0x080000, CRC(a54742da) SHA1(61f1b888c60cf157b07f853b52f644acc82c283d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "jackpotjusticesnd.bin", 0x000000, 0x080000, CRC(379e1a3d) SHA1(3b455a812284e716a831aadbaa592ee0ddab1a9d) )
ROM_END



// different startup code
ROM_START( j6jkrgld )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jg.p1", 0x000000, 0x010000, CRC(e5658ca2) SHA1(2d188899a4aa8124b7c492379331b8713913c69e) )
	ROM_LOAD16_BYTE( "jg.p2", 0x000001, 0x010000, CRC(efa0c84b) SHA1(ef511378904823ae66b7812eff13d9cef5fa621b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6jkrpls )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jp30cz04_1.b8", 0x000000, 0x020000, CRC(096baa03) SHA1(d9d3aa5616e253b49adda9254dbdaedb3e7ee72a) )
	ROM_LOAD16_BYTE( "jp30cz04_2.b8", 0x000001, 0x020000, CRC(bc023fe9) SHA1(99e5cfca3788809bf1958d21bfff37693419b846) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jp30cz05_1.b8", 0x000000, 0x020000, CRC(d36a7516) SHA1(613508c57c1e4e9ebfbdb1516d90cfa4e650721f) )
	ROM_LOAD16_BYTE( "jp30cz05_2.b8", 0x000000, 0x020000, CRC(233f2cd8) SHA1(edd02ebc4612075d9c594a5c37ff595bc695edd3) )
	ROM_LOAD16_BYTE( "jp30cz3_1.b8", 0x000000, 0x020000, CRC(939729ba) SHA1(b5a142c5e33755faebf7de065fbab1dbe9b78387) )
	ROM_LOAD16_BYTE( "jp30cz3_2.b8", 0x000000, 0x020000, CRC(8dcf654b) SHA1(076b1454b6bb4c634cb3b08922e49b7ae4f7bc32) )
	ROM_LOAD16_BYTE( "jp75cz04_1.b8", 0x000000, 0x020000, CRC(05260432) SHA1(3c68f1476be4803d901abc4e6f7bb81b14de4442) )
	ROM_LOAD16_BYTE( "jp75cz04_2.b8", 0x000000, 0x020000, CRC(bc023fe9) SHA1(99e5cfca3788809bf1958d21bfff37693419b846) )
	ROM_LOAD16_BYTE( "jp75cz05_1.b8", 0x000000, 0x020000, CRC(df27db27) SHA1(4d0d1673ffaa506f442e1d0a69ee05693ec901ae) )
	ROM_LOAD16_BYTE( "jp75cz05_2.b8", 0x000000, 0x020000, CRC(233f2cd8) SHA1(edd02ebc4612075d9c594a5c37ff595bc695edd3) )
	ROM_LOAD16_BYTE( "jp75cz3_1.b8", 0x000000, 0x020000, CRC(9fda878b) SHA1(e57944bff15d2be16c241461afb7e7b69a0ded0f) )
	ROM_LOAD16_BYTE( "jp75cz3_2.b8", 0x000000, 0x020000, CRC(8dcf654b) SHA1(076b1454b6bb4c634cb3b08922e49b7ae4f7bc32) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6jkpldx )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpd75cz03_1.b8", 0x000000, 0x020000, CRC(f48c26c5) SHA1(5a46a24a4575da3360eab54059ea994b7e8e4f8d) )
	ROM_LOAD16_BYTE( "jpd75cz03_2.b8", 0x000001, 0x020000, CRC(02d31498) SHA1(feb345442354b15a1a0a86e6b86db519aa8678fa) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "jpc75cz03_1.b8", 0x000000, 0x020000, CRC(efc89c1b) SHA1(b532e035e670dea515503b57c872640f5f39b114) )
	ROM_LOAD16_BYTE( "jpc75cz03_2.b8", 0x000000, 0x020000, CRC(8cac4066) SHA1(6950df8e9fa3cef3f1f5476c5290a9ff1308636a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6jkwld )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jpm_jokerswild_1001_1.bin", 0x000000, 0x020000, CRC(1a38aca0) SHA1(59a4d547cd22af3b5e5eaa161b564616ddcbc5ee) )
	ROM_LOAD16_BYTE( "jpm_jokerswild_1001_2.bin", 0x000001, 0x020000, CRC(a29a1961) SHA1(94e9af886609f4f0abe87d3ba71f0b533052ff2c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "jokwild.snd", 0x000000, 0x080000, CRC(352e28cd) SHA1(c98307f5eaf511c9d281151d1c07ffd83f24244c) )
ROM_END


ROM_START( j6jungfv )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "jun12nsk.1", 0x000000, 0x020000, CRC(4c867372) SHA1(6a16170f1e76ba4b49de5a963cba072b5934d23f) )
	ROM_LOAD16_BYTE( "jun12.2",    0x000001, 0x020000, CRC(4c755c51) SHA1(69df321acabed9fc83aa3137b9dafe22064568cd) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6kamel )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kameleon1.bin", 0x000000, 0x020000, CRC(d67063b5) SHA1(b48d21e7783812aae2ac6765dcef68287a3916ee) )
	ROM_LOAD16_BYTE( "kameleon2.bin", 0x000001, 0x020000, CRC(d7323d2f) SHA1(44c56c24c3fee291344eaacfb0183ec7f06b9cf7) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "kameleon.snd", 0x000000, 0x02c3f8, CRC(c294e7aa) SHA1(952fd4fd7e61125b5f8bbe6585d2aa3ca3eda605) )
ROM_END




ROM_START( j6kungfu )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "kfu20p1", 0x000000, 0x020000, CRC(f8dc50fb) SHA1(8beca856ac604d568e162d26c83f1d2984eccd6d) )
	ROM_LOAD16_BYTE( "kfu20p2", 0x000001, 0x020000, CRC(98d274ef) SHA1(9706f73a05f6ab1cda06afce8f814b19a768c646) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20067.bin", 0x000000, 0x020000, CRC(027d3728) SHA1(307456c5761e17ce2bc1d095c366f3aac92b77fb) )
	ROM_LOAD16_BYTE( "20068.bin", 0x000000, 0x020000, CRC(bdd0e987) SHA1(b6a2a3eb6b73c60c32373710653cb9da90e1a681) )
	ROM_LOAD16_BYTE( "20069.bin", 0x000000, 0x020000, CRC(e9e36310) SHA1(d36895d9c375c985daadd91159d67168d50936f2) )
	ROM_LOAD16_BYTE( "20070.bin", 0x000000, 0x020000, CRC(50b9f749) SHA1(c5796803185cec1557705cb815547f882ded53c6) )
	ROM_LOAD16_BYTE( "20071.bin", 0x000000, 0x020000, CRC(601046a0) SHA1(7e544f3734e8ccdf1c8bb475a98c4d82641dbfca) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "kufu-snd.bin", 0x000000, 0x080000, CRC(95360279) SHA1(f86c5ef3f7e790e3062ebda5150d2384ea341651) )
ROM_END



ROM_START( j6luckla )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "llv-b1.bin", 0x000000, 0x020000, CRC(b926235e) SHA1(839fae5355a1e9ed24e7cb0a70c773463a04cab5) )
	ROM_LOAD16_BYTE( "llv-b2.bin", 0x000001, 0x020000, CRC(8bab8906) SHA1(ae83bf8d87ce664f35446392b28bd89e92113dbb) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "llv-bf1.bin", 0x000000, 0x020000, CRC(7dfcc04e) SHA1(9d5e1c0265a2e7cb4090eab61cbb313104038fce) )
	ROM_LOAD16_BYTE( "llv-bp1.bin", 0x000000, 0x020000, CRC(52b87766) SHA1(2bab5d020992c7150e08bcb2b2da229a5216d5a4) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6magcir )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "magic1.bin", 0x000000, 0x020000, CRC(c7a646dc) SHA1(c4c6e2ecccdccb66421a4c926b9cac5260f855e3) )
	ROM_LOAD16_BYTE( "magic2.bin", 0x000001, 0x020000, CRC(fc4c700b) SHA1(a25900062b531956420394a412d9b08f1ef2bd02) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "927f", 0x000000, 0x020000, CRC(622fc61e) SHA1(c81fcc062c101d5895c54f772d70a31facca3c89) )
	ROM_LOAD16_BYTE( "92df", 0x000000, 0x020000, CRC(037ca5cc) SHA1(01e9608d912c6ee416bdcbe70001c3333119e26e) )
	ROM_LOAD16_BYTE( "92f7", 0x000000, 0x020000, CRC(f690fe18) SHA1(f392c48d8693fd822967b5515190bec39410a379) )
	ROM_LOAD16_BYTE( "92fe", 0x000000, 0x020000, CRC(2c3812e4) SHA1(8b3fac6f854eb0a4807b77a3626f72eff37ebc45) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "f098.bin", 0x000000, 0x080000, CRC(a4431105) SHA1(2dad84011ccf08be5b642884b2353718ebb4a6c7) )
ROM_END


ROM_START( j6mavrk )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9435.bin", 0x000000, 0x020000, CRC(b89e31a6) SHA1(9661f14fa9f655ac9748c67802755815da6a688e) )
	ROM_LOAD16_BYTE( "9436.bin", 0x000001, 0x020000, CRC(cbd565db) SHA1(afdb708a0724746feefde39dd202aaf4250a039b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9437.bin", 0x000000, 0x020000, CRC(5300659e) SHA1(f5be69a4abce517b4d00877ed8ffbf4ed6fc62ac) )
	ROM_LOAD16_BYTE( "9438.bin", 0x000000, 0x020000, CRC(ea5af1c7) SHA1(c004d418801e1caf239df4fdaf44ec1382988eea) )
	ROM_LOAD16_BYTE( "9439.bin", 0x000000, 0x020000, CRC(daf3402e) SHA1(9b57ea4649b4b198c59d49b2be2541dce00fc6af) )
	ROM_LOAD16_BYTE( "mav4_p2.bin", 0x000000, 0x020000, CRC(9813901e) SHA1(5e8e6565632ef6c0d1ee2ce9a5d68cb1de096d0c) )
	ROM_LOAD16_BYTE( "mav4p1.bin", 0x000000, 0x020000, CRC(d341b640) SHA1(8bdeb5ef07de5179685862ea82b33b337ed40055) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "1431.bin", 0x000000, 0x080000, CRC(2c95a586) SHA1(81f27d408f29bec0c79a7ac635e74a11cc93f2cc) )
ROM_END


ROM_START( j6maxod )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "maov-4s1.bin", 0x000000, 0x020000, CRC(f7b64d04) SHA1(1e534125c499a6c5bee1a84287c30caac06dd1fd) )
	ROM_LOAD16_BYTE( "maov-4s2.bin", 0x000001, 0x020000, CRC(b15d7a78) SHA1(77c14983a9ed9a58a183202390070a28f746f0a7) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "maov-4a1.bin", 0x000000, 0x020000, CRC(a5728d65) SHA1(55a25f8fc3c372c324b5eb3105e6b0a249481672) )
	ROM_LOAD16_BYTE( "maov-4p1.bin", 0x000000, 0x020000, CRC(1c28193c) SHA1(6788344909819863e624974b8a2b60b0f5d4e235) )
	ROM_LOAD16_BYTE( "maov-4w1.bin", 0x000000, 0x020000, CRC(95db3c8c) SHA1(bd5a7101ec7b04187178a5fec882a2ea5264c97f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "maov-snd.bin", 0x000000, 0x080000, CRC(9b527476) SHA1(6a6333aea592a1a7331a79372bbd6a16ff35c252) )
ROM_END


// startup code very different here.. but still makes the same 4800a0 write in the end
ROM_START( j6maxcsh )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "maxcash.p1", 0x000000, 0x020000, CRC(814459c1) SHA1(3584a772cefb252039d64f3350cfad88407dcc04) )
	ROM_LOAD16_BYTE( "maxcash.p2", 0x000001, 0x020000, CRC(6492e093) SHA1(ccbf8e997f282f914311e7fbb33bfe9088a38b2e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "maxcashsnd.bin", 0x000000, 0x080000, CRC(bfa9b42e) SHA1(2dae055df7571b4fd5feed55900f3873cfb00719) )
ROM_END


ROM_START( j6medal )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "60000101.evn", 0x000000, 0x080000, CRC(c26a8f9c) SHA1(d8ab81ee2c3e00016f215c68d3bc77b8cb3b5cf5) )
	ROM_LOAD16_BYTE( "60000101.odd", 0x000001, 0x080000, CRC(acf2ec0a) SHA1(b5d0b586c486de0185aa2c7fc991a5f13cebd9a7) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "60000103.evn", 0x000000, 0x080000, CRC(1ca919ac) SHA1(72df757a7f04b0224d84032ac8ca39caa40f6b67) )
	ROM_LOAD16_BYTE( "60000103.odd", 0x000000, 0x080000, CRC(7cba5d94) SHA1(b0f2dd057163879a653cae9270feece8570849ce) )
	ROM_LOAD16_BYTE( "60000104.evn", 0x000000, 0x080000, CRC(96389082) SHA1(03ef585ef3dc2c612d688a47399ac6b7fcd0d562) )
	ROM_LOAD16_BYTE( "60000104.odd", 0x000000, 0x080000, CRC(1add99b1) SHA1(228ef281ff45610dee999975c2aa51eb95ac4a29) )
	ROM_LOAD16_BYTE( "60001103.evn", 0x000000, 0x080000, CRC(1bf17077) SHA1(f14a0d1f255ab9665ac8ea4e02cdbc4178396b55) )
	ROM_LOAD16_BYTE( "60001103.odd", 0x000000, 0x080000, CRC(66dcc1da) SHA1(3cf289e9c44b24cfb3a74ec810f3cbd4b7bdb891) )
	ROM_LOAD16_BYTE( "60001104.evn", 0x000000, 0x080000, CRC(a3955286) SHA1(19f864dc04b158882322b1079d40d0588a41b517) )
	ROM_LOAD16_BYTE( "60001104.odd", 0x000000, 0x080000, CRC(36ec7263) SHA1(d554b702ffd6af0c7c8b700f8e09d348b387598d) )

	// what is this? looks ike something else (encrypted or bad) to me..
	ROM_LOAD16_BYTE( "80000002.p1", 0x000000, 0x080000, CRC(4789c981) SHA1(597de01ef44621cf1223694338470fde2b4e527b) )
	ROM_LOAD16_BYTE( "80000002.p2", 0x000000, 0x080000, CRC(16b2cc93) SHA1(ea0dd43e19602791cde16816c699e6d70b5cbe41) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "medjob.bin", 0x000000, 0x080000, CRC(dd8296bf) SHA1(c9209abf4276d81897476420177d24e739f0441e) )
ROM_END


ROM_START( j6megbck )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mebu-4_1.bin", 0x000000, 0x020000, CRC(85080234) SHA1(9a65e7adbf4f5f4832f7daebc2ff9abd430b74a2) )
	ROM_LOAD16_BYTE( "mebu-4_2.bin", 0x000001, 0x020000, CRC(4f34f862) SHA1(918f056a394c415d45b9a2d4cbe24b7d0ff531f7) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "mebu-4a1.bin", 0x000000, 0x020000, CRC(d7ccc255) SHA1(337be895b7af3222bf05b0dd36701684f118afac) )
	ROM_LOAD16_BYTE( "mebu-4n1.bin", 0x000000, 0x020000, CRC(b43ebaf0) SHA1(08af12e11e20d37a95528e43b0a50dc5a7e657fa) )
	ROM_LOAD16_BYTE( "mebu-4p1.bin", 0x000000, 0x020000, CRC(6e96560c) SHA1(af35cc8fda76100776312f4c53c1c15a508b175a) )
	ROM_LOAD16_BYTE( "mebu-4w1.bin", 0x000000, 0x020000, CRC(e76573bc) SHA1(58b65f7423d66d9ba839a450dd40100b4bfc6ffd) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "mebu-snd.bin", 0x000000, 0x080000, CRC(20bce62c) SHA1(50c5959eb5a5f8436a08f9a6a096b18cbf49970e) )
ROM_END


ROM_START( j6monmad )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mad14dsk.1", 0x000000, 0x020000, CRC(7b092a92) SHA1(808fd662ff5f4f02fb71b7864ab1951598847948) )
	ROM_LOAD16_BYTE( "mad14.2",    0x000001, 0x020000, CRC(f45f4187) SHA1(f571ad9174b2aab6a8bf18ee7cf768ce19e41339) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6monspd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msp10dsk.1", 0x000000, 0x080000, CRC(892aa085) SHA1(cfb8d4edbf22a88906b3b1fa52156be201d81b44) )
	ROM_LOAD16_BYTE( "msp10.2",    0x000001, 0x080000, CRC(3db5e13e) SHA1(79eb1f17a8e1b3220cd7c5f46212b8a2e1a112cb) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */

ROM_END


ROM_START( j6montlk )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "money talks 8 3-1.bin", 0x000000, 0x020000, CRC(42936de2) SHA1(e9ba9f2af8a6e2bcf887976f50ff7e3e3dcf86d7) )
	ROM_LOAD16_BYTE( "money talks 8 3-2.bin", 0x000001, 0x020000, CRC(25ad84ed) SHA1(f704eceadc6f4f422f6ff421837aa6c9f2f533f7) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "6844.bin", 0x000000, 0x020000, CRC(9d25282a) SHA1(4e8205a85853ed99e3568976edf95601e8b35599) )
	ROM_LOAD16_BYTE( "6845.bin", 0x000000, 0x020000, CRC(b747c0d4) SHA1(4bdabdf6156fdf588f5f55447f120ef5bf19621f) )
	ROM_LOAD16_BYTE( "6846.bin", 0x000000, 0x020000, CRC(76bb7c12) SHA1(a0f025a90e10cfce8bbf4c2671b30fb3ede12ba0) )
	ROM_LOAD16_BYTE( "6847.bin", 0x000000, 0x020000, CRC(cfe1e84b) SHA1(275bf18954e706a98371d463a9372f3b5e7310f3) )
	ROM_LOAD16_BYTE( "6848.bin", 0x000000, 0x020000, CRC(ff4859a2) SHA1(23bd372d6899eafc423afbee00c14954b8a37477) )
	ROM_LOAD16_BYTE( "8299.bin", 0x000000, 0x020000, CRC(360cdf5a) SHA1(947ef29e101539515c321058712bcf685909f6d6) )
	ROM_LOAD16_BYTE( "8300.bin", 0x000000, 0x020000, CRC(71650174) SHA1(6907f82e54549fe6ecc1fd9f5d342b41357578a8) )
	ROM_LOAD16_BYTE( "8301.bin", 0x000000, 0x020000, CRC(dd928b62) SHA1(f79652160c23700c88b254120b5ba3ddc3ca6cf3) )
	ROM_LOAD16_BYTE( "8302.bin", 0x000000, 0x020000, CRC(64c81f3b) SHA1(0572f8c962432b08edd442512dcbc3b1461a4ede) )
	ROM_LOAD16_BYTE( "money talks 8 3i1.bin", 0x000000, 0x020000, CRC(4edec3d3) SHA1(fa05a84819f4fc7f128b149ea4fcd6edee6d3b78) )
	ROM_LOAD16_BYTE( "mtalk8ac", 0x000000, 0x020000, CRC(4edec3d3) SHA1(fa05a84819f4fc7f128b149ea4fcd6edee6d3b78) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
//  ROM_LOAD( "1109.bin", 0x000000, 0x080000, CRC(b4d7ac12) SHA1(ac194d15e9d4e5cdadddbf2dc3c9660b52f116c2) )
	ROM_LOAD( "mtsnd.bin", 0x000000, 0x080000, CRC(b4d7ac12) SHA1(ac194d15e9d4e5cdadddbf2dc3c9660b52f116c2) )
ROM_END


ROM_START( j6mono60 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9362.bin", 0x000000, 0x020000, CRC(b90825e1) SHA1(526399eb77f876f2946b8468ca2f980b66b0d739) )
	ROM_LOAD16_BYTE( "9363.bin", 0x000001, 0x020000, CRC(0eba908b) SHA1(29bd7dc6000004039037173f6098e52f20931b1e) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8305.bin", 0x000000, 0x020000, CRC(f542b4da) SHA1(821be9da2c5f83f7c833eb729a136723b680905a) )
	ROM_LOAD16_BYTE( "8306.bin", 0x000000, 0x020000, CRC(4c182083) SHA1(6647236b6511c5292bf6159713d61be8faa2597f) )
	ROM_LOAD16_BYTE( "8307.bin", 0x000000, 0x020000, CRC(7cb1916a) SHA1(861e43efbaa28c5e8fc9f509173e855385c96131) )
	ROM_LOAD16_BYTE( "8308.bin", 0x000000, 0x020000, CRC(12914ed3) SHA1(21d5c3cab4d51a4ae623ba1c407edbee282a485a) )
	ROM_LOAD16_BYTE( "8659.bin", 0x000000, 0x020000, CRC(f90f1aeb) SHA1(ec21a7f6be4d9c5df3631ea8e42e1a6dbc63174d) )
	ROM_LOAD16_BYTE( "9364.bin", 0x000000, 0x020000, CRC(529671d9) SHA1(1807b56e63d0d82fd03a72cfc9928b5ba9cbc0b7) )
	ROM_LOAD16_BYTE( "9365.bin", 0x000000, 0x020000, CRC(ebcce580) SHA1(3323d93e7eb9ea54c79f050326d4ea77f470dc4f) )
	ROM_LOAD16_BYTE( "9366.bin", 0x000000, 0x020000, CRC(db655469) SHA1(ef6924063384a85b6d3c6503f3ca8833076cbdb6) )
	ROM_LOAD16_BYTE( "mo60.p1", 0x000000, 0x020000, CRC(1edce0e2) SHA1(626055d1df8a8ee2f9c650cee6cfeb047699e532) )
	ROM_LOAD16_BYTE( "mo60.p2", 0x000000, 0x020000, CRC(1a1ea1da) SHA1(25e14d7ae82888e1b14d0dfb391d2dbafdb1b643) )
	ROM_LOAD16_BYTE( "mon608p1", 0x000000, 0x020000, CRC(418ef9f2) SHA1(0bd2296f9b59a97befa9b87b967d21dac049f91c) )
	ROM_LOAD16_BYTE( "mon608p2", 0x000000, 0x00e000, CRC(f30f2300) SHA1(ba9e8ccbdebc0fb0f656b44886b6bee89d019b39) ) // looks bad
	ROM_LOAD16_BYTE( "mon60_p1.bin", 0x000000, 0x020000, CRC(1edce0e2) SHA1(626055d1df8a8ee2f9c650cee6cfeb047699e532) )
	ROM_LOAD16_BYTE( "mon60_p2.bin", 0x000000, 0x020000, CRC(1a1ea1da) SHA1(25e14d7ae82888e1b14d0dfb391d2dbafdb1b643) )
	ROM_LOAD16_BYTE( "monov8p1", 0x000000, 0x020000, CRC(948ae4a2) SHA1(fdf4cb950d2637d5c8c18cb2709c8c5265609425) )
	ROM_LOAD16_BYTE( "monov8p2", 0x000000, 0x020000, CRC(16379f37) SHA1(65ed96cbbd816d5ae07b489981a50f445c6b07f4) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
//  ROM_LOAD( "mon60_snd.bin", 0x000000, 0x080000, CRC(c79af6d0) SHA1(518a7b16978a843bdb83938279b11f446503361e) )
	ROM_LOAD( "monop60.snd", 0x000000, 0x080000, CRC(c79af6d0) SHA1(518a7b16978a843bdb83938279b11f446503361e) )
ROM_END

ROM_START( j6mono60a )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "s60_pound8p1", 0x00000, 0x020000, CRC(f5220142) SHA1(2d9460fbd833b477f579f0925dd999fa3d4355cc) )
	ROM_LOAD16_BYTE( "s60_pound8p2", 0x00001, 0x020000, CRC(3786bbb8) SHA1(d45d1d6713e480fa6e430fd1900778dcc56250c9) )
	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "s60snd", 0x0000, 0x080000, CRC(f597a454) SHA1(ab88779657524df2b04b91ed35f25dc9206a5623) )
ROM_END

ROM_START( j6monobn )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7935.bin", 0x000000, 0x020000, CRC(afdb6320) SHA1(33f49e796f2ee08ebd604caf140f07febdedc0d0) )
	ROM_LOAD16_BYTE( "7936.bin", 0x000001, 0x020000, CRC(782cabbb) SHA1(9103126580923427741a6bb8cea75cf4b7fe78dd) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7937.bin", 0x000000, 0x020000, CRC(44453718) SHA1(35dc2238155d9415fe1d4f518bc02368ca27a1a7) )
	ROM_LOAD16_BYTE( "7938.bin", 0x000000, 0x020000, CRC(cdb612a8) SHA1(346eecf0c301f9028691055cf83e939fc5ae303f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6outlaw )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7547.bin", 0x000000, 0x020000, CRC(7a4efbf1) SHA1(5e184e172a129aad6ad34409f63de25916414146) )
	ROM_LOAD16_BYTE( "7548.bin", 0x000001, 0x020000, CRC(e5d61efc) SHA1(2e3ce747b14341ad5fcbf815a6b7e9a38a59478a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "outlawsnd.bin", 0x000000, 0x080000, CRC(9b9f21dc) SHA1(aca23a525f1288f49a18a74eb36ac3a67efa7e20) )
ROM_END

ROM_START( j6outlawd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7549.bin", 0x000000, 0x020000, CRC(91d0afc9) SHA1(267500478d8fb73e61a869e53b598d0bea3c3caa) )
	ROM_LOAD16_BYTE( "7548.bin", 0x000001, 0x020000, CRC(e5d61efc) SHA1(2e3ce747b14341ad5fcbf815a6b7e9a38a59478a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "outlawsnd.bin", 0x000000, 0x080000, CRC(9b9f21dc) SHA1(aca23a525f1288f49a18a74eb36ac3a67efa7e20) )
ROM_END

ROM_START( j6outlawc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "outlaw.p1", 0x000000, 0x020000, CRC(42b7d388) SHA1(4d647879a95b27788ea87885f266272344e910ea) )
	ROM_LOAD16_BYTE( "outlaw.p2", 0x000001, 0x020000, CRC(4c06f95a) SHA1(42085ea0c10930a27f0b223a6f0742165ee85727) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "outlawsnd.bin", 0x000000, 0x080000, CRC(9b9f21dc) SHA1(aca23a525f1288f49a18a74eb36ac3a67efa7e20) )
ROM_END



ROM_START( j6oxo )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7613.bin", 0x000000, 0x020000, CRC(0621762a) SHA1(ec09cfed79158b09093a162785b6bdd1916ce50c) )
	ROM_LOAD16_BYTE( "7614.bin", 0x000001, 0x020000, CRC(f3bc7c8b) SHA1(a46928b9c5c7c14c5c555fb893a528763cd35963) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7425.bin", 0x000000, 0x020000, CRC(13d3b623) SHA1(bfc27e3ad60209f5567f2fa66cffb6a7a099bbce) )
	ROM_LOAD16_BYTE( "7615.bin", 0x000000, 0x020000, CRC(54e5b64b) SHA1(280f8b9f3fa871511d77c6cce0e836fa30b3bfc6) )
	ROM_LOAD16_BYTE( "7616.bin", 0x000000, 0x020000, CRC(edbf2212) SHA1(a467baae065d0b7f19d32add1af6db226d1d2d85) )
	ROM_LOAD16_BYTE( "7617.bin", 0x000000, 0x020000, CRC(25d4b1c2) SHA1(58526fd300882124cedc514bb6968729aee6a04b) )
	ROM_LOAD16_BYTE( "7618.bin", 0x000000, 0x020000, CRC(c88bdd77) SHA1(9c18c7c30e4d5c43752864eb4972a6d02865293f) )
	ROM_LOAD16_BYTE( "7619.bin", 0x000000, 0x020000, CRC(771071a3) SHA1(02c7490230b0f92cc6672934e5cc8d0fc21dba33) )
	ROM_LOAD16_BYTE( "7620.bin", 0x000000, 0x020000, CRC(ce4ae5fa) SHA1(fe46e4d3db6e5bcfd9b478e5ae5da035a677b941) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "oxo_bingo_snd.bin", 0x000000, 0x080000, CRC(008a2d6a) SHA1(a89114154489142556b373ab24cd32fadf5856b3) )
ROM_END


ROM_START( j6oxobin )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7423.bin", 0x000000, 0x020000, CRC(f84de21b) SHA1(fb401153681ec271e8ddad4cd292a8c1dccfcb19) )
	ROM_LOAD16_BYTE( "7424.bin", 0x000001, 0x020000, CRC(7a4b827b) SHA1(28bd2e527c54780f2d866baa8909c1ade39825f6) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7775.bin", 0x000000, 0x020000, CRC(ea87746c) SHA1(2608c4461362dc96527c04454b3f616b88a4dace) )
	ROM_LOAD16_BYTE( "7776.bin", 0x000000, 0x020000, CRC(a570d3b3) SHA1(1ee001078a87774167784555f57a95c6713057c1) )
	ROM_LOAD16_BYTE( "7777.bin", 0x000000, 0x020000, CRC(01192054) SHA1(7768d318532c37c08f8b695deb200a3d92cca550) )
	ROM_LOAD16_BYTE( "7967.bin", 0x000000, 0x020000, CRC(03a5cb12) SHA1(7da2bec83e8808b40c96af8ac4741dfa1d8f6c39) )
	ROM_LOAD16_BYTE( "7968.bin", 0x000000, 0x020000, CRC(ec5aeab7) SHA1(2df0fbe853e90a82f0c2ed0bc2a728b17b99062a) )
	ROM_LOAD16_BYTE( "7969.bin", 0x000000, 0x020000, CRC(61c8ba9a) SHA1(bf0e979aaa36c11dac950e06cb713bb8b2b4ccdb) )
	ROM_LOAD16_BYTE( "7970.bin", 0x000000, 0x020000, CRC(e83b9f2a) SHA1(c297ed9066487b547a605636fdd8d3c04cc726a2) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "oxo_bingo_snd.bin", 0x000000, 0x080000, CRC(008a2d6a) SHA1(a89114154489142556b373ab24cd32fadf5856b3) )
ROM_END


ROM_START( j6pacman )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pac_1.bin", 0x000000, 0x020000, CRC(23c1f010) SHA1(3e9cb8e22e700fa28e1fad6300bc70567322383f) )
	ROM_LOAD16_BYTE( "pac_2.bin", 0x000001, 0x020000, CRC(773f33db) SHA1(5f61139bcccdea0f61c064dd2085a9904fa796ce) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 ) // which sound rom is right?
	ROM_LOAD( "pacman plus snd 1 65e5.bin", 0x000000, 0x080000, CRC(f0130ebe) SHA1(fd623add85d2faa556561f5b5b74c7c17d4ba02a) )
	ROM_LOAD( "pacsndv5.bin", 0x000000, 0x080000, CRC(cdb73ef6) SHA1(d204b50981aa34271c795c95b92e48371801cdd4) )
ROM_END


ROM_START( j6papa )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pa0_4.p1", 0x000000, 0x020000, CRC(fc811398) SHA1(bdf8c8ccb67a5349f5a75502b22325a9d293a229) )
	ROM_LOAD16_BYTE( "pa0_4.p2", 0x000001, 0x020000, CRC(47b1317f) SHA1(a0f20e6ed92e8d7a4d76281649b34d181aa9638c) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "pa0_5.p1", 0x000000, 0x020000, CRC(21d0a639) SHA1(0fe3ca2cdb73c1610b7bb293a9cc267263cf9020) )
	ROM_LOAD16_BYTE( "pa0_5.p2", 0x000000, 0x020000, CRC(d78a0063) SHA1(fd24ea795929e95006ca11237678277ad03fffff) )
	ROM_LOAD16_BYTE( "pa0_7.p1", 0x000000, 0x020000, CRC(60c23acd) SHA1(ece601078fc06c2ddb9b15ee6bc2e13739839fbc) )
	ROM_LOAD16_BYTE( "pa0_7.p2", 0x000000, 0x020000, CRC(1abfb80d) SHA1(940811dbd342f1a1dd4de7b82a7ecf0560412ad4) )
	ROM_LOAD16_BYTE( "pa1_0.p1", 0x000000, 0x020000, CRC(27e8b437) SHA1(5d06915d3c757466f0dcd0c4215efb73a5bf326a) )
	ROM_LOAD16_BYTE( "pa1_0.p2", 0x000000, 0x020000, CRC(62523eb7) SHA1(9c5b4a3729c154f64c907b731cb423a987e40f52) )
	ROM_LOAD16_BYTE( "pa1_1.p1", 0x000000, 0x020000, CRC(a1a88705) SHA1(b9d52bff66845e510e8236ab98630271121f0bbc) )
	ROM_LOAD16_BYTE( "pa1_1.p2", 0x000000, 0x020000, CRC(3aac53d3) SHA1(55f36dbd7e6d4353aae6b988f102392131ec9452) )
	ROM_LOAD16_BYTE( "papa1_3.p1", 0x000000, 0x020000, CRC(5952b2ab) SHA1(3c69fd95d3470d8cbac22af91a21dc9f9e56e69d) )
	ROM_LOAD16_BYTE( "papa1_3.p2", 0x000000, 0x020000, CRC(80bd82a8) SHA1(abb497fecc37105d1436b77749137c7c6538a165) )
	ROM_LOAD16_BYTE( "papa.p1", 0x000000, 0x020000, CRC(cb57a63c) SHA1(3472df9e375c820f1f8016528736bbfc37f6def3) )
	ROM_LOAD16_BYTE( "papa.p2", 0x000000, 0x020000, CRC(2ec81ffe) SHA1(9d8cb56a54576ec2f3b7a13793bdb4e6b5a765ca) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "papasnd.bin", 0x000000, 0x080000, CRC(c2de3abc) SHA1(2885817e7d6b11c0a2b35507b5654902257db32c) )
ROM_END


ROM_START( j6phxgld )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pg30cz05_1.b8", 0x000000, 0x020000, CRC(9eb4716a) SHA1(459db543d20e5ddd03d0be917d8d4e153cb97183) )
	ROM_LOAD16_BYTE( "pg30cz05_2.b8", 0x000001, 0x020000, CRC(36003add) SHA1(f806358ee1111c3c57b90b50f6db5935d0aa26a6) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "pg30cz07_1.b8", 0x000000, 0x020000, CRC(fe4270a9) SHA1(d8cbcc609c0a3af3de360504bc70757c981e4296) )
	ROM_LOAD16_BYTE( "pg30cz07_2.b8", 0x000000, 0x020000, CRC(0d4e4744) SHA1(9f1599859670b8973455ba6780ec4e2064ce29c2) )
	ROM_LOAD16_BYTE( "pg30sl_02_1.b8", 0x000000, 0x020000, CRC(0fc9de3d) SHA1(590f581f144344947b17ff05345f46ce240572bb) )
	ROM_LOAD16_BYTE( "pg30sl_02_2.b8", 0x000000, 0x020000, CRC(85702a38) SHA1(014ed2f9c25f6f9d35288cbe99b2cab1fac3569b) )
	ROM_LOAD16_BYTE( "pg75cz05_1.b8", 0x000000, 0x020000, CRC(92f9df5b) SHA1(2f817e065d0beb3c7a7acd0f4e4457f3ab4a80ad) )
	ROM_LOAD16_BYTE( "pg75cz07_1.b8", 0x000000, 0x020000, CRC(f20fde98) SHA1(a3f156031ef608a22111d4311b3be8aec4423ba0) )
	ROM_LOAD16_BYTE( "pg75cz1_1.b8", 0x000000, 0x020000, CRC(8f2b8818) SHA1(f4d1247f6b42741513dc698a19ff48032a05b8fb) )
	ROM_LOAD16_BYTE( "pg75cz1_2.b8", 0x000000, 0x020000, CRC(a6cacbd9) SHA1(bc253ce246d4fe6396efc1f163032e288da4bfec) )
	ROM_LOAD16_BYTE( "pgcz30_04_1.b8", 0x000000, 0x020000, CRC(94724991) SHA1(bac5bf431dd46cbf1d177ebecd0b51e043d4764a) )
	ROM_LOAD16_BYTE( "pgcz30_04_2.b8", 0x000000, 0x020000, CRC(8a4f924c) SHA1(50ab5f41c72162f64a575a7143496a5ab12bdaa0) )
	ROM_LOAD16_BYTE( "pgcz75_04_1.b8", 0x000000, 0x020000, CRC(983fe7a0) SHA1(ef078fe2b17c8214ff58181f52c13de78c6bee9b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "pgsnd2.b8", 0x000000, 0x080000, CRC(c63cf006) SHA1(f204da5e744dd2ade662ac8d9f7d1896513cb38a) )
ROM_END


ROM_START( j6pnxgd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pccz2_1.b8", 0x000000, 0x020000, CRC(7d801a34) SHA1(ca5b6685d92c2d5489ea27af2044c1f9d7bd365c) )
	ROM_LOAD16_BYTE( "pccz2_2.b8", 0x000001, 0x020000, CRC(de2bd9ae) SHA1(cd64e18a5b9c3bef9589015b85b1abcc41aaed45) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "pgsnd2.b8", 0x000000, 0x080000, CRC(c63cf006) SHA1(f204da5e744dd2ade662ac8d9f7d1896513cb38a) )
ROM_END


ROM_START( j6pnxmil )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "phoenixmill_93001.b8", 0x000000, 0x020000, CRC(a7889afc) SHA1(800570f97be625e7fb1067e2f85c252bfc66c796) )
	ROM_LOAD16_BYTE( "phoenixmill_93002.b8", 0x000001, 0x020000, CRC(533fe752) SHA1(912ae6eeb9ccf9f2ae829816350eb3913bfe485e) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "phoenixmill_97501.b8", 0x000000, 0x020000, CRC(abc534cd) SHA1(413dc4781d8770f5e9162a33e3656f75f38de142) )
	ROM_LOAD16_BYTE( "phoenixmill_97502.b8", 0x000000, 0x020000, CRC(533fe752) SHA1(912ae6eeb9ccf9f2ae829816350eb3913bfe485e) )
	ROM_LOAD16_BYTE( "pm30cz10_1.b8", 0x000000, 0x020000, CRC(fc475584) SHA1(bfb96597c5d648aea67253f9f12ced0434680931) )
	ROM_LOAD16_BYTE( "pm30cz10_2.b8", 0x000000, 0x020000, CRC(cb87457b) SHA1(1f7578f07bc9e05d370390f46998c0036985b328) )
	ROM_LOAD16_BYTE( "pm75cz10_1.b8", 0x000000, 0x020000, CRC(f00afbb5) SHA1(e73c5daed0692be444be9ff143c7bff5bb8089d0) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "phmill7_snd2", 0x000000, 0x014008, CRC(12086987) SHA1(ca6d74b844a0c042d66940f5d39fdef9d5591651) )// bad?
	ROM_LOAD( "phmill7_snd.bin", 0x000000, 0x080000, CRC(e7332f6d) SHA1(c2457be9a7a37184bacd4199a5c347896ecfeb1c) )
ROM_END


ROM_START( j6pinwzd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "piwi-4s1.bin", 0x000000, 0x020000, CRC(7a0c9ef2) SHA1(2f375cce448c97a3f5905c1c8110e0bd39051842) )
	ROM_LOAD16_BYTE( "piwi-4s2.bin", 0x000001, 0x020000, CRC(3b270013) SHA1(f2570d66210fe37810afb5417217b0f50048af76) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "piwi-4a1.bin", 0x000000, 0x020000, CRC(28c85e93) SHA1(3e4437cb2719b0648c8c6769cc867da836e3906b) )
	ROM_LOAD16_BYTE( "piwi-4p1.bin", 0x000000, 0x020000, CRC(9192caca) SHA1(333b2c75044aa846c4faeed1907aeb0dba52e7ab) )
	ROM_LOAD16_BYTE( "piwi-4wp.bin", 0x000000, 0x020000, CRC(f3ffbb42) SHA1(2ce69db92c408f4b43baa021a37a326e509c5c38) )
	ROM_LOAD16_BYTE( "piwi3p1", 0x000000, 0x020000, CRC(551c4596) SHA1(0a8aa7a0c17f4fb0e5e097677ec16e9f29b9f9e8) )
	ROM_LOAD16_BYTE( "piwi3p2", 0x000000, 0x020000, CRC(4dfcfdd5) SHA1(4e89e8d07771caaaa2f0c28c74db25af37ae634c) )
	ROM_LOAD16_BYTE( "pwiz15p1", 0x000000, 0x020000, CRC(ea4878e9) SHA1(0f61b9a971a9dfdb8cee9779d3e5b7dc705bba77) )
	ROM_LOAD16_BYTE( "pwiz15p2", 0x000000, 0x020000, CRC(561ee3df) SHA1(cfae78c0a9f448191aed16406fc46da208f5b62b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "pbwzsnd.bin", 0x000000, 0x020000, CRC(ee9df577) SHA1(bdc6ecba5b7ad9c7b012342c7710266ec6eeb0ab) ) // bad?
	ROM_LOAD( "piwi-snd.bin", 0x000000, 0x080000, CRC(ba98eecc) SHA1(39edb9524c23a78f89077215bef8f43a47605b47) )
ROM_END


ROM_START( j6pirgld )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pirat_113001.b8", 0x000000, 0x020000, CRC(58c488b8) SHA1(bd2cf3f604a1fd075d2077c8e38d75c0c3325cb8) )
	ROM_LOAD16_BYTE( "pirat_113002.b8", 0x000001, 0x020000, CRC(d38c52dd) SHA1(64f61a50c164d0d592bc566104a57a9630a78757) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "pirat_117501.b8", 0x000000, 0x020000, CRC(54892689) SHA1(9b87c38e89db690971142066113b78a97b34719f) )
	ROM_LOAD16_BYTE( "pirat_117502.b8", 0x000000, 0x020000, CRC(d38c52dd) SHA1(64f61a50c164d0d592bc566104a57a9630a78757) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "pirat6_snd.bin", 0x000000, 0x080000, CRC(5c60a3f8) SHA1(9d83aca9e5ecd230f6ca98f033f5274dbefe9feb) )
ROM_END


ROM_START( j6popoli )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7604.bin", 0x000000, 0x020000, CRC(7b44b69e) SHA1(8bbc3caa889d539646198c9b7f54cd31ab715c6d) )
	ROM_LOAD16_BYTE( "7605.bin", 0x000001, 0x020000, CRC(8b3cd10b) SHA1(92acf729c3bff02517f149dd80b18747d647bd2f) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7606.bin", 0x000000, 0x020000, CRC(90dae2a6) SHA1(38b790a3eb2aad3d2c039b27c18a1331b5c57b46) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6pog )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "potofgold1.bin", 0x000000, 0x020000, CRC(302e901e) SHA1(e88ded26da8b62b771eda0800e6e4afb1ae95ecf) )
	ROM_LOAD16_BYTE( "potofgold2.bin", 0x000001, 0x020000, CRC(40584378) SHA1(eeda580d65226feb642c541d1f16f2ff7b909098) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9417.bin", 0x000000, 0x020000, CRC(89740447) SHA1(8528bf5faac53a375cd34cc329439a5bf6029ecd) )
	ROM_LOAD16_BYTE( "9419.bin", 0x000000, 0x020000, CRC(008721f7) SHA1(e438dd70b3018898ad13d0888ce67b0bbb3ca526) )
	ROM_LOAD16_BYTE( "ace pog v3-2 (27c010)", 0x000000, 0x020000, CRC(28a2a74c) SHA1(c95a6a0a89cf942ba2460c46185e782f0445df2f) )
	ROM_LOAD16_BYTE( "ace pog v3a1 (27c010)", 0x000000, 0x020000, CRC(5b37fafc) SHA1(0aa7d0d813832f99150521d97ed754fa1f5f5fc6) )

	// these look like the roms in the 'impact video system' set? is this a video game? or are these misplaced / bad?
	ROM_LOAD( "pog_1.bin", 0x000000, 0x100000, CRC(2244020f) SHA1(e392605473e5d0ff7d00bfe48d275d78a3417ded) )
	ROM_LOAD( "pog_2.bin", 0x000000, 0x100000, CRC(165f1742) SHA1(6b7b992dfa0383a50b67c4528d8a461149a65d2b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "pog10snd.bin", 0x000000, 0x080000, CRC(00f6d1f6) SHA1(66581a6391e9ddc931cb102b00f38720ab125f5c) )
ROM_END


ROM_START( j6pogcls )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clpg-31.bin", 0x000000, 0x020000, CRC(07102281) SHA1(a7760c78f4848d7eec175027485d19d38307ccf9) )
	ROM_LOAD16_BYTE( "clpg-32.bin", 0x000001, 0x020000, CRC(8296488c) SHA1(8d3893fae9f2dde72a18e4b0a980814e3a3679ad) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "clpg-3a1.bin", 0x000000, 0x020000, CRC(55d4e2e0) SHA1(3d6d87c64392e863fc52a994c81a6f813cb06dfc) )
	ROM_LOAD16_BYTE( "clpg-3p1.bin", 0x000000, 0x020000, CRC(ec8e76b9) SHA1(53762b6d5f3d406b10bc5f9829826080cc590f40) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6pwrlin )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "powerl_63001.b8", 0x000000, 0x020000, CRC(28e5d1f3) SHA1(3acd26d88b19c61fce3286111d051dd0aaccd064) )
	ROM_LOAD16_BYTE( "powerl_63002.b8", 0x000001, 0x020000, CRC(789b3389) SHA1(4d497084489472da598d8d01811000a5ce14e0e8) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "powerl_67501.b8", 0x000000, 0x020000, CRC(24a87fc2) SHA1(3a80a7aa2defe0d419ba619c8f0e9502556fbfd1) )
	ROM_LOAD16_BYTE( "powerl_67502.b8", 0x000000, 0x020000, CRC(789b3389) SHA1(4d497084489472da598d8d01811000a5ce14e0e8) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6pwrspn )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "posp-5s1.bin", 0x000000, 0x020000, CRC(4184881b) SHA1(fb4dfb2e5b5c2cebd15b908a38014b56bb311eef) )
	ROM_LOAD16_BYTE( "posp-5s2.bin", 0x000001, 0x020000, CRC(7f888b32) SHA1(fed82966d74e9f8e0195b39a1ae267bff7c96677) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "posp-5a1.bin", 0x000000, 0x020000, CRC(1340487a) SHA1(c9f8c08ff4679f6a21dbb00d50f58e953a14716b) )
	ROM_LOAD16_BYTE( "posp-5p1.bin", 0x000000, 0x020000, CRC(aa1adc23) SHA1(a39b42de2ca2832e3c709f8b55bb15acc214bb73) )
	ROM_LOAD16_BYTE( "posp-5wp.bin", 0x000000, 0x020000, CRC(c877adab) SHA1(c4dcdeafd5c9d600f88e2b8d43c14bfcf7c0bbdd) )
	ROM_LOAD16_BYTE( "posp4p1", 0x000000, 0x020000, CRC(798af2ba) SHA1(912e6307a39239419b9e9295706e070632168ce0) )
	ROM_LOAD16_BYTE( "posp4p2", 0x000000, 0x020000, CRC(25883a02) SHA1(a5eb7c27e3e72e5609ee4c98a57e552f8feabffe) )
	ROM_LOAD16_BYTE( "pows15p1", 0x000000, 0x020000, CRC(a6d11f57) SHA1(489cd1cdd505ca4c9db87b0b8baf1cb0d43646ba) )
	ROM_LOAD16_BYTE( "pows15p2", 0x000000, 0x020000, CRC(e6f0e76d) SHA1(8138991e102f86b7f984c61a24f9255e726c807e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "posp-snd.bin", 0x000000, 0x080000, CRC(861a0d14) SHA1(ea5eef793ad682dbf660ed7e77f93a7b900c97cc) )
ROM_END


ROM_START( j6quantm )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20717.bin", 0x000000, 0x020000, CRC(31435fe1) SHA1(d42fe30367ded93562d4e1739307a47423f4dd51) )
	ROM_LOAD16_BYTE( "20718.bin", 0x000001, 0x020000, CRC(4d6f4b1f) SHA1(f562c677cf920fe2a0e5edec2e4f241855e005c3) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20719.bin", 0x000000, 0x020000, CRC(dadd0bd9) SHA1(6bbe782b46ed5fe0677845548077593df25d1c0d) )
	ROM_LOAD16_BYTE( "20720.bin", 0x000000, 0x020000, CRC(63879f80) SHA1(3c1b818378c8995fdadbec0aeda2d2b04db89e6a) )
	ROM_LOAD16_BYTE( "20721.bin", 0x000000, 0x020000, CRC(532e2e69) SHA1(70661489c4c16af0a59fcfc3f68e4182aa14a8be) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "qule-snd.bin", 0x000000, 0x080000, CRC(eb8c692d) SHA1(384b73573d64d67547d1c04f279bda6c02f78450) )
ROM_END


ROM_START( j6quick )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "quicksilver_a1.bin", 0x000000, 0x020000, CRC(ddca1835) SHA1(adb8a9320c8ab2c5c6d8ec83b1ed577a8eb4ba91) )
	ROM_LOAD16_BYTE( "quicksilver_a2.bin", 0x000001, 0x020000, CRC(efa8765d) SHA1(c1174f9ab8f687d1dd8b4d50ff519550a0643219) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "quicksilver_snd.bin", 0x000000, 0x080000, CRC(374d8892) SHA1(38918bbcb7d9117b21b6dafdb55cc5c36927fb4e) )
ROM_END


ROM_START( j6rager )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rag.p1", 0x000000, 0x020000, CRC(d8f3a090) SHA1(17b5bbf09d5a4f31d0fe4f6561fb03e97e3e4c9a) )
	ROM_LOAD16_BYTE( "rag.p2", 0x000001, 0x020000, CRC(5d176dc3) SHA1(c2641f01d57fd2eb7247252cc42a92e21e0f60a5) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "ragsnd.bin", 0x000000, 0x080000, CRC(5eadea62) SHA1(f1f6800a6479ae49a4774ee597e3d58e036f4100) )
ROM_END


ROM_START( j6ra )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "real-5s1.bin", 0x000000, 0x020000, CRC(b1894f2e) SHA1(dc77a2b40e9fee9bdc81697bf27ec81e420b06ea) )
	ROM_LOAD16_BYTE( "real-5s2.bin", 0x000001, 0x020000, CRC(01b3214d) SHA1(6a16c2e5045e85bfe26d805a1157eab0f0aa0cb0) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "real-5a1.bin", 0x000000, 0x020000, CRC(e34d8f4f) SHA1(4df257c8c8602035e6a174f4ed5698f2ef911021) )
	ROM_LOAD16_BYTE( "real-5p1.bin", 0x000000, 0x020000, CRC(5a171b16) SHA1(e5ddd23f1d3a062cd6b57dffbf88edb043ae9a6e) )
	ROM_LOAD16_BYTE( "real-5wp.bin", 0x000000, 0x020000, CRC(387a6a9e) SHA1(8f47ec2d58dfb85a27e2574f096ba3cc79036948) )
	ROM_LOAD16_BYTE( "realr3p1", 0x000000, 0x020000, CRC(ad9d92d3) SHA1(874a37e9db59ce8dd83fb96e8ae0ec1bd64aa1ae) )
	ROM_LOAD16_BYTE( "realr3p2", 0x000000, 0x020000, CRC(37ac2694) SHA1(b603f84146bea70794e98dab47d705b180e72b8d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "realsnd.bin", 0x000000, 0x080000, CRC(8bc92c90) SHA1(bcbbe270ce42d5960ac37a2324e3fb37ff513147) )
ROM_END


ROM_START( j6raclb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20953.bin", 0x000000, 0x020000, CRC(ed7afbad) SHA1(fdb2af1dedfaf1a5dfa543ba58bf4420b19c3454) )
	ROM_LOAD16_BYTE( "20950.bin", 0x000001, 0x020000, CRC(5c4659d9) SHA1(aa69f2c1d4ea6755ef1ed7e3d040598befcb6690) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "racl-ff1.bin", 0x000000, 0x020000, CRC(77bb4848) SHA1(4900a09cf0712d87aacf4772406a4d284c6e7d9e) )
	ROM_LOAD16_BYTE( "racl-fi1.bin", 0x000000, 0x020000, CRC(e1a56b39) SHA1(036850fcfd39b1194718c7bf575991514015d015) )
	ROM_LOAD16_BYTE( "racl-fp1.bin", 0x000000, 0x020000, CRC(58ffff60) SHA1(e68d9b8d8523732ced12eb7bfb8a749499ea01c1) )
	ROM_LOAD16_BYTE( "racl-fs1.bin", 0x000000, 0x020000, CRC(b361ab58) SHA1(129feeadc9f0026fa86c39f42e506027b6665dfa) )
	ROM_LOAD16_BYTE( "racl-fs2.bin", 0x000000, 0x020000, CRC(a1ad549a) SHA1(e33184262c9e76e8fecdcec9fa274baa16ba9d67) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "realsnd.bin", 0x000000, 0x080000, CRC(8bc92c90) SHA1(bcbbe270ce42d5960ac37a2324e3fb37ff513147) )
ROM_END


ROM_START( j6redarw )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rear-6_1.bin", 0x000000, 0x020000, CRC(19b81492) SHA1(2714248444c9dc800eb8cfed67106b33e1e070e3) )
	ROM_LOAD16_BYTE( "rear-6_2.bin", 0x000001, 0x020000, CRC(691cc832) SHA1(c68bee5abbec6d3b28030ee58991f0c4abe70d35) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rear-6a1.bin", 0x000000, 0x020000, CRC(4b7cd4f3) SHA1(1d8cb022b3d9ba07be6da0f8f068875386f6b73a) )
	ROM_LOAD16_BYTE( "rear-6n1.bin", 0x000000, 0x020000, CRC(288eac56) SHA1(1fbfa85cdfbf4bf23d2e2989eabf16e2536e9ae3) )
	ROM_LOAD16_BYTE( "rear-6np.bin", 0x000000, 0x020000, CRC(c310f86e) SHA1(28b2c2993e727dac1201323d89f3568704e1acdd) )
	ROM_LOAD16_BYTE( "rear-6p1.bin", 0x000000, 0x020000, CRC(f22640aa) SHA1(72793eb730ea970815a95202e4c44ebb3f15c124) )
	ROM_LOAD16_BYTE( "rear-6w1.bin", 0x000000, 0x020000, CRC(7bd5651a) SHA1(2d1caf1314bcbf95f2da98de0a6782a073cdbb4c) )
	ROM_LOAD16_BYTE( "rear-7_1.bin", 0x000000, 0x020000, CRC(6d36f687) SHA1(d07a2058fc0f7a7f1cac6398c46f6fc2fb676484) )
	ROM_LOAD16_BYTE( "rear-7_2.bin", 0x000000, 0x020000, CRC(962c6cac) SHA1(dc5fb331df921ef7b94d2c0d3e7d0bb299e728a3) )
	ROM_LOAD16_BYTE( "rear-7a1.bin", 0x000000, 0x020000, CRC(3ff236e6) SHA1(f0c42885b6f908ffdcf41ae3b6789bfa95c8f36f) )
	ROM_LOAD16_BYTE( "rear-7n1.bin", 0x000000, 0x020000, CRC(5c004e43) SHA1(eb4dfdb35efc5226df366cd3f2cdcd486098ba53) )
	ROM_LOAD16_BYTE( "rear-7p1.bin", 0x000000, 0x020000, CRC(86a8a2bf) SHA1(1c7301c3e5a3ff1d791b21e617d50875c06a7463) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 ) // which is correct?
	ROM_LOAD( "rear-snd.bin", 0x000000, 0x080000, CRC(7e8c05ce) SHA1(616b0f94b94331f86d7b1fec11dd05cf9b0968cf) )
	ROM_LOAD16_BYTE( "rear7np1.bin", 0x000000, 0x020000, CRC(b79e1a7b) SHA1(806ae7180dbee9b605bd8d923179a2323a7d38ee) )
ROM_END

ROM_START( j6redarww )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD( "redarrow.p1", 0x000000, 0x020000, CRC(e47fdd2c) SHA1(1cbce6e38cacad4c0ec617e38522ef81feaeb296) )
	ROM_LOAD( "redarrow.p2", 0x000001, 0x020000, CRC(afccd6c4) SHA1(5cfcb7132a169ea13fc0b48fc2d34071243a9046) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 ) // which is correct?
	ROM_LOAD( "rear-snd.bin", 0x000000, 0x080000, CRC(7e8c05ce) SHA1(616b0f94b94331f86d7b1fec11dd05cf9b0968cf) )
	ROM_LOAD16_BYTE( "rear7np1.bin", 0x000000, 0x020000, CRC(b79e1a7b) SHA1(806ae7180dbee9b605bd8d923179a2323a7d38ee) )
ROM_END


ROM_START( j6reddmn )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "reddemon1.bin", 0x000000, 0x020000, CRC(8f2aa8a7) SHA1(ea5b8c1418deaaf2bda58a40cacca3c77d6d5a08) )
	ROM_LOAD16_BYTE( "reddemon2.bin", 0x000001, 0x020000, CRC(2a7658ab) SHA1(4286a4a76b8d95a4da4e8aad2f81b091d2d2f96a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "reddemonsnd.bin", 0x000000, 0x080000, CRC(4fb1cbff) SHA1(27393e14af18f05df07bcbbab957a684de79dbb1) )
ROM_END


//68k crashes
ROM_START( j6rh6 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rhsx-a1.bin", 0x000000, 0x020000, CRC(fa5d4bb2) SHA1(22c896babcc052214a87e928006826ab6e8795bf) )
	ROM_LOAD16_BYTE( "rhsx-a2.bin", 0x000001, 0x020000, CRC(7d3435f0) SHA1(7d2b55d6b40fe069123fea16a92a4db2490bebe8) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rhsx-f1.bin", 0x000000, 0x020000, CRC(781c2139) SHA1(87f594ce9285142fd8e6553a9e24ca4f224e4fef) )
	ROM_LOAD16_BYTE( "rhsx-f2.bin", 0x000000, 0x020000, CRC(a9319b7f) SHA1(e8c393868fdf26fd30741f936b5ea708ea0c3fc7) )
	ROM_LOAD16_BYTE( "rhsx-fb1.bin", 0x000000, 0x020000, CRC(74518f08) SHA1(9d63ba9730e17ab13f7eab0ea49d833553017701) )
	ROM_LOAD16_BYTE( "rhsx-fe1.bin", 0x000000, 0x020000, CRC(60877d5b) SHA1(247421c1205dab12cb4e1701ecf667716dee7149) )
	ROM_LOAD16_BYTE( "rhsx-ff1.bin", 0x000000, 0x020000, CRC(bcc6c229) SHA1(7775b137649b66904bf38e9d480a70b9d62292bb) )
	ROM_LOAD16_BYTE( "rhsx-fp1.bin", 0x000000, 0x020000, CRC(93827501) SHA1(eac03990095f3ae2d00b6822b3cc8b0790f8507b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6rhchil )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rhcst.p1", 0x000000, 0x020000, CRC(334fdbcc) SHA1(194c226e1c41eb326841cf022e8a1b28088a7073) )
	ROM_LOAD16_BYTE( "rhcst.p2", 0x000001, 0x020000, CRC(e4a12747) SHA1(f6f0388dcf2713f4c289b4ae313cfccf1d308963) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "rhcs-3a1.bin", 0x000000, 0x020000, CRC(618b1bad) SHA1(3461d18d89694f4a28f1ae63dcea69a3413fef9d) )
	ROM_LOAD16_BYTE( "rhcs-3l1.bin", 0x000000, 0x020000, CRC(f79538dc) SHA1(6f6986b949a36229eb4a61b3dbfeb60a3d51cc6d) )
	ROM_LOAD16_BYTE( "rhcs-3p1.bin", 0x000000, 0x020000, CRC(d8d18ff4) SHA1(131caae565e480bf6d893591284b5f5f998deb32) )
	ROM_LOAD16_BYTE( "rhcs3lp1.bin", 0x000000, 0x020000, CRC(1c0b6ce4) SHA1(8713aae84f960d4fb121ce5552d1b14692b39c28) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "rhcssnd.bin", 0x000000, 0x080000, CRC(60b336b1) SHA1(53d04bec9cbba4a0e89d34329ed41f89945e283b) )
ROM_END


ROM_START( j6rh6cl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8edf", 0x000000, 0x020000, CRC(e0b6b3cb) SHA1(4c174f3a5ef0bd74877c2a769c60375cb34a3a6f) )
	ROM_LOAD16_BYTE( "f903.bin", 0x000001, 0x020000, CRC(ba024f9b) SHA1(f078d1ecbf8397f0cad99957c081acc44e40b5cb) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8ef7", 0x000000, 0x020000, CRC(155ae81f) SHA1(f03d04bd6b3b5f90be10e137984dda42c9d53c17) )
	ROM_LOAD16_BYTE( "8efb", 0x000000, 0x020000, CRC(3cf70cb9) SHA1(a31041b35310a5a10845ac2330df677fe2f0b95e) )
	ROM_LOAD16_BYTE( "8efe", 0x000000, 0x020000, CRC(cff204e3) SHA1(7b7c1558ea491fbb1fe61eeb932ccbd0e14e266d) )
	ROM_LOAD16_BYTE( "8eff.bin", 0x000000, 0x020000, CRC(246c50db) SHA1(b210d611ed4d0e410ebe65236b4acca3b2e07b7f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "rh6s1.bin", 0x000000, 0x080000, CRC(86b4c970) SHA1(0ce214ee406b1c325693b2b615498bdb2c3a16eb) )
ROM_END


ROM_START( j6reelmn )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9485.bin", 0x000000, 0x020000, CRC(1a729666) SHA1(d4389ff5a16b994ca1480fe0ff097c7601b2ef6b) )
	ROM_LOAD16_BYTE( "9486.bin", 0x000001, 0x020000, CRC(522b3e21) SHA1(caef56b3479ebe0007a420cbe1c0766e121b05c1) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9487.bin", 0x000000, 0x020000, CRC(f1ecc25e) SHA1(a5240f0fae7ddc056bfc974e37fabf5d51d7db53) )
	ROM_LOAD16_BYTE( "9488.bin", 0x000000, 0x020000, CRC(48b65607) SHA1(edeb98305907951c40a247a2f1337b77727c6d71) )
	ROM_LOAD16_BYTE( "9489.bin", 0x000000, 0x020000, CRC(781fe7ee) SHA1(f7799b5f74916aab64538b3467d2363a754f8af5) )
	ROM_LOAD16_BYTE( "remov7p1", 0x000000, 0x020000, CRC(55a48785) SHA1(7cea4fb4e9d521d6512e2ae0da36ee243d29e998) )
	ROM_LOAD16_BYTE( "remov7p2", 0x000000, 0x020000, CRC(8fd6fefc) SHA1(91d6d2f0915a20e2177992b360af763bcfe30cf7) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "1360.bin", 0x000000, 0x080000, CRC(3a1a5f09) SHA1(807cf2cf7a4738c1904990b281f7d4c9a86c78e7) )
ROM_END



ROM_START( j6reelth )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "reelthing.p1", 0x000000, 0x020000, CRC(74b90cef) SHA1(097fbbdd049a85e9a9251858fb80cf9cde2397fc) )
	ROM_LOAD16_BYTE( "reelthing.p2", 0x000001, 0x020000, CRC(c219443a) SHA1(742d9d7f8c1071c5860b34449909ca1d2decc053) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "reelthingsnd.bin", 0x000000, 0x080000, CRC(44d72e51) SHA1(f472e9effbd925ecc5db6dd47eddf3f5cea8fe46) )
ROM_END



ROM_START( j6richpk )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rich1p.bin", 0x000000, 0x020000, CRC(e99fd568) SHA1(ae71eb6a9871fa9856fb62d2a5158776d9c2bddf) )
	ROM_LOAD16_BYTE( "rich2.bin",  0x000001, 0x020000, CRC(39d5a254) SHA1(464696715769f1e15c80acf4116d7718490abf8c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6rico )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9523.bin", 0x000000, 0x020000, CRC(4eee01c3) SHA1(f2336b4732efa0b86ee8a0df6d975355c2f27367) )
	ROM_LOAD16_BYTE( "9524.bin", 0x000001, 0x020000, CRC(7d17a2ec) SHA1(eba6e5cdee844bdce4d9657009c64433307e49af) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9525.bin", 0x000000, 0x020000, CRC(a57055fb) SHA1(ed1572f8a0a40ea48a1bf4249a424780286f484a) )
	ROM_LOAD16_BYTE( "9526.bin", 0x000000, 0x020000, CRC(1c2ac1a2) SHA1(2aa10007b06bb5fb4e4220d99c5928bda3bb756f) )
	ROM_LOAD16_BYTE( "9527.bin", 0x000000, 0x020000, CRC(2c83704b) SHA1(34787e1c6664838476fd6f8085752e1647f3b765) )
	ROM_LOAD16_BYTE( "rico-51a.p1", 0x000000, 0x020000, CRC(6dc6ca21) SHA1(f0e38d90e0613a899aba520f3ecc746195a4b8fe) )
	ROM_LOAD16_BYTE( "ricov5p1", 0x000000, 0x020000, CRC(3f020a40) SHA1(98b00f4e5720135e71176bb5507f194468fd99ef) )
	ROM_LOAD16_BYTE( "ricov5p2", 0x000000, 0x020000, CRC(7510a81d) SHA1(8c46008bd8e8fbd9439370383c378ea19e0963c8) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "ricochet_snd.bin", 0x000000, 0x080000, CRC(b24522fe) SHA1(1546edee6cf483abdbc761c715dcbc696209d429) )
ROM_END



ROM_START( j6robin )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9425.bin", 0x000000, 0x020000, CRC(0b9f9c83) SHA1(2004b7fc3a0fd8ba424d0e293cd53b57a653f6e3) )
	ROM_LOAD16_BYTE( "9426.bin", 0x000001, 0x020000, CRC(a85c6c19) SHA1(eab10967642f801dcd25dcdd4dfe95ac3b3491bf) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9427.bin", 0x000000, 0x020000, CRC(e001c8bb) SHA1(084182d27ddd4146e75ec5cdc9c20e09f4bb390b) )
	ROM_LOAD16_BYTE( "9428.bin", 0x000000, 0x020000, CRC(595b5ce2) SHA1(276290e86454b2f1fc73b2cc3e545b6c60363535) )
	ROM_LOAD16_BYTE( "9429.bin", 0x000000, 0x020000, CRC(69f2ed0b) SHA1(dd759bd012f55fb94982c748f0cbc02178438abd) )

//  ROM_LOAD16_BYTE( "robin_a_wb.bin", 0x000000, 0x020000, CRC(69f2ed0b) SHA1(dd759bd012f55fb94982c748f0cbc02178438abd) )
//  ROM_LOAD16_BYTE( "robin_b_wb.bin", 0x000000, 0x020000, CRC(a85c6c19) SHA1(eab10967642f801dcd25dcdd4dfe95ac3b3491bf) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "robin_snd.bin", 0x000000, 0x080000, CRC(bbddccf9) SHA1(33f3d14d4898f9ba4ba2c2a88621cf3e2c828a8f) )
ROM_END


ROM_START( j6roller )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7368.bin", 0x000000, 0x020000, CRC(6fe1365b) SHA1(50271888ccd4395938721f2026ecfefd342c5209) )
	ROM_LOAD16_BYTE( "7369.bin", 0x000001, 0x020000, CRC(8ab98caa) SHA1(c9bb582917b4a6be477d592d6cbd28b5f7552a26) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7370.bin", 0x000000, 0x020000, CRC(847f6263) SHA1(b072b54ccea0a37cd4aa027bb869dd82db389360) )
	ROM_LOAD16_BYTE( "7371.bin", 0x000000, 0x020000, CRC(3d25f63a) SHA1(b37ef0c4c2cd8c948407a8767636b1a6115a1d94) )
	ROM_LOAD16_BYTE( "7372.bin", 0x000000, 0x020000, CRC(63ac986a) SHA1(2b4f7f59179ea9a239986c7a87e229a506ca006e) )
	ROM_LOAD16_BYTE( "8137.bin", 0x000000, 0x020000, CRC(ecc408ab) SHA1(12608c8645f02ea7d2d368907fd36be90ea5da56) )
	ROM_LOAD16_BYTE( "8138.bin", 0x000000, 0x020000, CRC(559e9cf2) SHA1(b93c69fed10b61a1b8d9988f69ac8a980b1ec57c) )
	ROM_LOAD16_BYTE( "8139.bin", 0x000000, 0x020000, CRC(65372d1b) SHA1(e4aa2350989b889e70785f368d834f422928d225) )
	ROM_LOAD16_BYTE( "9460.bin", 0x000000, 0x020000, CRC(28d13bcb) SHA1(1103b06830cbcd25e856b99beb307f69f0fee6fd) )
	ROM_LOAD16_BYTE( "9461.bin", 0x000000, 0x020000, CRC(b7515266) SHA1(d0b4d9b1dbc968f289e04398e81b585e71e3b358) )
	ROM_LOAD16_BYTE( "9462.bin", 0x000000, 0x020000, CRC(c34f6ff3) SHA1(fd93acc1b0f985b3fa4b822791a6c3864d8d8ad0) )
	ROM_LOAD16_BYTE( "9463.bin", 0x000000, 0x020000, CRC(7a15fbaa) SHA1(37fd6ddce0d2262df5ba32b90cf0fe664f9c7de1) )
	ROM_LOAD16_BYTE( "9464.bin", 0x000000, 0x020000, CRC(4abc4a43) SHA1(85b6379c356d02ba3ab2e7bd4bbc628d95f0b750) )
	ROM_LOAD16_BYTE( "rc.p1.bin", 0x000000, 0x020000, CRC(075a5c93) SHA1(ea1c6dac0af35d25c58fe8ca8e442a364cdb92e7) )
	ROM_LOAD16_BYTE( "rc.p2.bin", 0x000000, 0x020000, CRC(48cbbc99) SHA1(8bbb445d0e1defcac44d5637006d974aa268b8bf) )
	ROM_LOAD16_BYTE( "rc10v8-1.bin", 0x000000, 0x020000, CRC(8d0fac13) SHA1(44d9dbb5f9ea10068b5ffea972ee0e11ad3b6275) )
	ROM_LOAD16_BYTE( "rc10v8-2.bin", 0x000000, 0x020000, CRC(3c032987) SHA1(4fff2dd84c22ad10306d7712340857b5703b8f1b) )
	ROM_LOAD16_BYTE( "rc82.1ac", 0x000000, 0x020000, CRC(3ac2b6b6) SHA1(ea71c6acf5cd6cbacdc55df6897b0dc6560e92fe) )
	ROM_LOAD16_BYTE( "rc82.1p1", 0x000000, 0x020000, CRC(368f1887) SHA1(7b2523ab746fc46931895ca1eb81e7d5d9a1c864) )
	ROM_LOAD16_BYTE( "rc82.1p2", 0x000000, 0x020000, CRC(dc0f8c62) SHA1(3a4b1bd698c4e14c8c794b3bfb5fa9bc631475cc) )
	ROM_LOAD16_BYTE( "roco15p1.bin", 0x000000, 0x020000, CRC(ef1104f6) SHA1(5f61e4d22f17f761280eb8e932792d211d9928d3) )
	ROM_LOAD16_BYTE( "roco15p2.bin", 0x000000, 0x020000, CRC(9b3d316d) SHA1(6b66e458cd53c1527c2b295e898ead83720a7d99) )
	ROM_LOAD16_BYTE( "rollercoaster10_p1.bin", 0x000000, 0x020000, CRC(566a3d2c) SHA1(410e6ef59a3af9751e59b539affde95c8d94ba31) )
	ROM_LOAD16_BYTE( "rollercoaster10_p2.bin", 0x000000, 0x020000, CRC(4ea94876) SHA1(52be24fa61431ecf3f1a206ddfb5b5a52fc9ad0a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 ) // which is correct?
	ROM_LOAD( "rocosnd.bin", 0x000000, 0x080000, CRC(60236e81) SHA1(9546c92d8a42d911e5b462c598a0b42987c0ba03) )
	ROM_LOAD( "rcoastersoundalt.bin", 0x000000, 0x080000, CRC(e7e587c9) SHA1(fde7a7761253dc4133340b766d220873731c11c7) )
	ROM_LOAD( "rcstrsnd.bin", 0x000000, 0x080000, CRC(b0753c1d) SHA1(b111ca10c01ee2089cbc613ad91235d429272ab8) )
ROM_END


ROM_START( j6rccls )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "clrc-41.bin", 0x000000, 0x020000, CRC(6f312407) SHA1(c077276d18f50340989a20fe208ac84801895dc1) )
	ROM_LOAD16_BYTE( "clrc-42.bin", 0x000001, 0x020000, CRC(9d3c34b6) SHA1(6636bb33c79d2b8a2d570b76a60b195db57336ae) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "clrc-4a1.bin", 0x000000, 0x020000, CRC(3df5e466) SHA1(c2868ce1ac76feed5cce5f4304ce0b1e2814a763) )
	ROM_LOAD16_BYTE( "clrc-4p1.bin", 0x000000, 0x020000, CRC(84af703f) SHA1(e336f188bebe37dfa673af49913394eb59bf8a2d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6royfls )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20661.bin", 0x000000, 0x020000, CRC(f778f856) SHA1(7dd33fd41c81f6131f37d0d6fedccd36d6413eb8) )
	ROM_LOAD16_BYTE( "20662.bin", 0x000001, 0x020000, CRC(486f1e68) SHA1(ce1c3ba4d9031950db313e7179a1126a920e48e4) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20663.bin", 0x000000, 0x020000, CRC(1ce6ac6e) SHA1(0ae838c2f5629d167920ddcafe402b39b6ebd091) )
	ROM_LOAD16_BYTE( "20664.bin", 0x000000, 0x020000, CRC(33a21b46) SHA1(b555688000f13910c60b38016d2ce546cc6b97bc) )
	ROM_LOAD16_BYTE( "rfcl-4_2.bin", 0x000000, 0x020000, CRC(486f1e68) SHA1(ce1c3ba4d9031950db313e7179a1126a920e48e4) )
	ROM_LOAD16_BYTE( "rfcl-4g1.bin", 0x000000, 0x020000, CRC(951589de) SHA1(2f4379e5cb38909c492f7ec96fdcfd0f71cd055b) )
	ROM_LOAD16_BYTE( "rfcl-4gp.bin", 0x000000, 0x020000, CRC(7e8bdde6) SHA1(d9111880bec2e6d5b1a85c92f32c4e6599906700) )
	ROM_LOAD16_BYTE( "rflushc.1", 0x000000, 0x020000, CRC(c9a20e26) SHA1(34fbeb89c0b9a6d0f1cd6297245e6c56933d3981) )
	ROM_LOAD16_BYTE( "rflushc.2", 0x000000, 0x020000, CRC(02270768) SHA1(26853837b72d0529cca7dfae3488daec8d3bc998) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
//  ROM_LOAD( "rflushc.as1", 0x000000, 0x080000, CRC(c86bce52) SHA1(ba9b3f73e7418710737d3ace25ee2747d5685d8e) )
	ROM_LOAD( "rofl-so.bin", 0x000000, 0x080000, CRC(c86bce52) SHA1(ba9b3f73e7418710737d3ace25ee2747d5685d8e) )
ROM_END


ROM_START( j6samur )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "0bff", 0x000000, 0x020000, CRC(62e39cc6) SHA1(d739d3cbf74f7e6ef2323d120095eec316bcda9b) )
	ROM_LOAD16_BYTE( "3b6c", 0x000001, 0x020000, CRC(78541fe0) SHA1(a255beab55911cb14e54abe5357e1dc8c0232755) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "0bdf", 0x000000, 0x020000, CRC(a6397fd6) SHA1(1d470d69bdbf321bd74abe7410245f3c10530625) )
	ROM_LOAD16_BYTE( "0bf7", 0x000000, 0x020000, CRC(53d52402) SHA1(e038cf92b88a1f097235c358b3d6803748e54b8e) )
	ROM_LOAD16_BYTE( "0bfb", 0x000000, 0x020000, CRC(7a78c0a4) SHA1(6f2ae73fa0ee6794f2e5d2e179b82de1ba41f6c8) )
	ROM_LOAD16_BYTE( "0bfe", 0x000000, 0x020000, CRC(897dc8fe) SHA1(518c7cda31330725729cabb6bd76d2aea1a1a28b) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6sidewd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "siw-11s1.bin", 0x000000, 0x020000, CRC(d24e9e75) SHA1(22d21eb5bfe92fc61e1667345aafe7b3214c9218) )
	ROM_LOAD16_BYTE( "siw-11s2.bin", 0x000001, 0x020000, CRC(d4eee8cd) SHA1(0a3c2e19bd2202968344ca1204f7aed6250f2e34) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "siw-11a1.bin", 0x000000, 0x020000, CRC(808a5e14) SHA1(7219f838928cc1dc46c85b4c17c7b7a85345dfb7) )
	ROM_LOAD16_BYTE( "siw-11p1.bin", 0x000000, 0x020000, CRC(39d0ca4d) SHA1(3ae060b55108423662181262f1c6710787ba1975) )
	ROM_LOAD16_BYTE( "siw-11w1.bin", 0x000000, 0x020000, CRC(b023effd) SHA1(68ed43328656c05cb8bcb2053052c6b5f8e8856c) )
	ROM_LOAD16_BYTE( "siwi10p1", 0x000000, 0x020000, CRC(9ee164e0) SHA1(e1941810ce395f106b8f309af9e95d053d084232) )
	ROM_LOAD16_BYTE( "siwi10p2", 0x000000, 0x020000, CRC(e2751ca4) SHA1(9979fb71834f1b498628376d56cd86852276c585) )
	ROM_LOAD16_BYTE( "swin15p1", 0x000000, 0x020000, CRC(aa77639c) SHA1(7051b54f5890cfb7b602e96355d7c807dbe5c0f4) )
	ROM_LOAD16_BYTE( "swin15p2", 0x000000, 0x020000, CRC(77cfa4aa) SHA1(9b4b8498b92bc503c247473befec7836bea716a9) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "sidewindersnd.bin", 0x000000, 0x080000, CRC(6e49b83b) SHA1(cba9ce8cc5dbaa0b498b2314165d4cc64c0a3881) )
ROM_END


ROM_START( j6snakes )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20351.bin", 0x000000, 0x020000, CRC(7f9685fd) SHA1(b55c39ebbabd59a4c9aeea0f4337b8a629bad664) )
	ROM_LOAD16_BYTE( "20352.bin", 0x000001, 0x020000, CRC(c51c0fb2) SHA1(4eea976fe46b35f46553cd0645ff39d7ac62d988) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20353.bin", 0x000000, 0x020000, CRC(9408d1c5) SHA1(06b6fdba329f8b4854a92b61571359c8cde3fd26) )
	ROM_LOAD16_BYTE( "20354.bin", 0x000000, 0x020000, CRC(2d52459c) SHA1(740d0033174bfed153e17b0efc140e380c519654) )
	ROM_LOAD16_BYTE( "20355.bin", 0x000000, 0x020000, CRC(1dfbf475) SHA1(536fedcffc07c08f7214a140a4bcde4f4127af8e) )
	ROM_LOAD16_BYTE( "slad15p1", 0x000000, 0x020000, CRC(acb53439) SHA1(45e6ce2ef508655c75f836a5979740021bc93227) )
	ROM_LOAD16_BYTE( "slad15p2", 0x000000, 0x020000, CRC(3962b105) SHA1(e216abf66539e2bc19e5a67946fd3b9366a41b5d) )
	ROM_LOAD16_BYTE( "slad17p1", 0x000000, 0x020000, CRC(06f87c27) SHA1(52f25180524fcc18c11e039937ac53350af5e90e) )
	ROM_LOAD16_BYTE( "slad17p2", 0x000000, 0x020000, CRC(2cc8a1eb) SHA1(5762ff97a7dd5e8857d506cce333f197ac7a7925) )
	ROM_LOAD16_BYTE( "snla.p1", 0x000000, 0x020000, CRC(7f9685fd) SHA1(b55c39ebbabd59a4c9aeea0f4337b8a629bad664) )
	ROM_LOAD16_BYTE( "snla.p2", 0x000000, 0x020000, CRC(c51c0fb2) SHA1(4eea976fe46b35f46553cd0645ff39d7ac62d988) )
	ROM_LOAD16_BYTE( "snla10p1", 0x000000, 0x020000, CRC(2de13a66) SHA1(73d534348408648d678609c1821d97a8877788fa) )
	ROM_LOAD16_BYTE( "snla10p2", 0x000000, 0x020000, CRC(f0116e1d) SHA1(b53351142676b50484202bde180067eec0c592e6) )
	ROM_LOAD16_BYTE( "snladv~1", 0x000000, 0x020000, CRC(ced86202) SHA1(ba04b24f97a17f5d8aecd63515687e87f34029bb) )
	ROM_LOAD16_BYTE( "snladv~2", 0x000000, 0x020000, CRC(64b29222) SHA1(6e17a0ee68af644ef59abcacd5d3ad2412fdfad6) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "snla-snd.bin", 0x000000, 0x080000, CRC(7902d8ef) SHA1(222b0a18902619c9b4b29fa2485cb4e143c21bab) )
ROM_END


ROM_START( j6sonic )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9440.bin", 0x000000, 0x020000, CRC(384f931e) SHA1(12bddb3dc2c1bc6c51c5ef4002673c6f45fa335c) )
	ROM_LOAD16_BYTE( "9441.bin", 0x000001, 0x020000, CRC(cba4c367) SHA1(ad926216081797b81b93eb111bf8fd50b289c9e2) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9442.bin", 0x000000, 0x020000, CRC(d3d1c726) SHA1(4957542e0f451fe523ed2d19e9b3df3dec126a0a) )
	ROM_LOAD16_BYTE( "9443.bin", 0x000000, 0x020000, CRC(6a8b537f) SHA1(1e228e3e07dfb775add02510491523501f6d374b) )
	ROM_LOAD16_BYTE( "9444.bin", 0x000000, 0x020000, CRC(5a22e296) SHA1(372e3bd4a0e83adc01c8882dae3c392917b84613) )
	ROM_LOAD16_BYTE( "hdg6cp1", 0x000000, 0x010000, CRC(db73388e) SHA1(e253b4011b9e26e5634ae359924a417f437760d0) ) // ???
	ROM_LOAD16_BYTE( "shgv7p1a.bin", 0x000000, 0x020000, CRC(074da030) SHA1(69b5c256a36940503a7405eeba68e66865319c9d) )
	ROM_LOAD16_BYTE( "shgv7p1s.bin", 0x000000, 0x020000, CRC(55896051) SHA1(35f6856e076aead15819af7a537f8bbd6eabe3d4) )
	ROM_LOAD16_BYTE( "shgv7p2s.bin", 0x000000, 0x020000, CRC(8f128e3e) SHA1(a1c1c3ad091b4838450a151e74087453d47cf5d1) )
	ROM_LOAD16_BYTE( "sohe8p1", 0x000000, 0x020000, CRC(3659825e) SHA1(9cd23058b244d4d0ecaaeec351a2ed4e68671688) )
	ROM_LOAD16_BYTE( "sohe8p2", 0x000000, 0x020000, CRC(e9aa2dbc) SHA1(1e8d1ad35bb55a48223a10e11e51087a12379aae) )
	ROM_LOAD16_BYTE( "son15p1", 0x000000, 0x020000, CRC(3659825e) SHA1(9cd23058b244d4d0ecaaeec351a2ed4e68671688) )
	ROM_LOAD16_BYTE( "son15p2", 0x000000, 0x020000, CRC(e9aa2dbc) SHA1(1e8d1ad35bb55a48223a10e11e51087a12379aae) )
	ROM_LOAD16_BYTE( "sonic.p1", 0x000000, 0x020000, CRC(d4e65332) SHA1(60f0ef8fb5a21cfa3bfa2f8cf4f2ceebe672bb3a) )
	ROM_LOAD16_BYTE( "sonic.p2", 0x000000, 0x020000, CRC(4bf35499) SHA1(ea8ecfadc9f2c3b2000986735edb57699959f24c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "son_snd.bin", 0x000000, 0x080000, CRC(5cd8cf21) SHA1(82f875a59d678ef548173ee2c202e3963bc13116) )
ROM_END


ROM_START( j6spcinv )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spin-31.bin", 0x000000, 0x020000, CRC(e95af475) SHA1(0439bc798739fb2bfb2931917977f505c213ea93) )
	ROM_LOAD16_BYTE( "spin-32.bin", 0x000001, 0x020000, CRC(c688fb6e) SHA1(820bd78d3ebe0134e51b322ed348be1f1874b18c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "spaceinvaderssnd.bin", 0x000000, 0x080000, CRC(8ee10ee2) SHA1(959df6fe6b83d3671dbde4146fa2344d6a7b8b31) )
ROM_END


ROM_START( j6stards )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9318.bin", 0x000000, 0x020000, CRC(a39d0ad8) SHA1(d41b4686fc5492e257d5913a5d66160e9a8367a9) )
	ROM_LOAD16_BYTE( "9319.bin", 0x000001, 0x020000, CRC(f5c4c5d7) SHA1(837595acad74735bb82c9fd3623813bc59c56c86) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9320.bin", 0x000000, 0x020000, CRC(48035ee0) SHA1(0d513eb953aa951a776352dd5ddfebf1c5bc61b2) )
	ROM_LOAD16_BYTE( "9321.bin", 0x000000, 0x020000, CRC(f159cab9) SHA1(3e231a20768835b21aaecbfafe7093717bf376cf) )
	ROM_LOAD16_BYTE( "9322.bin", 0x000000, 0x020000, CRC(c1f07b50) SHA1(28cc1ce96bead7323b3dbdb0a97a9f4030e7ed8c) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "star-snd.bin", 0x000000, 0x080000, CRC(d2dcd6cc) SHA1(f5a290befd41014b6aabae9fdb601d5a9766f1ba) )
ROM_END


ROM_START( j6start )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "sttu-7s1.bin", 0x000000, 0x020000, CRC(7f60f498) SHA1(48070f9260cf3b5ff53613145acac6dc1511805f) )
	ROM_LOAD16_BYTE( "sttu-7_2.bin", 0x000001, 0x020000, CRC(829a2227) SHA1(d6819f029fe3778a3e9989cef1da658d7eb571ac) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "sttu-7p1.bin", 0x000000, 0x020000, CRC(94fea0a0) SHA1(64d8748865d650311f8b42aeac03cd38c4cdfaa1) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6strk10 )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20178.bin", 0x000000, 0x020000, CRC(35135814) SHA1(250c0835611be4eaabab4c59288d9fdeafdc6ca5) )
	ROM_LOAD16_BYTE( "20179.bin", 0x000001, 0x020000, CRC(c515abce) SHA1(0c8fb9d390f1d3f646b6d6b4177a5fa929c9067e) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20180.bin", 0x000000, 0x020000, CRC(de8d0c2c) SHA1(5c1bf58801ca9113f86aff5fa4c7f2e015fef2c0) )
	ROM_LOAD16_BYTE( "20181.bin", 0x000000, 0x020000, CRC(67d79875) SHA1(a5fbdb71487219e4432a0c94c00e341ebb124358) )
	ROM_LOAD16_BYTE( "9490.bin", 0x000000, 0x020000, CRC(847c50ae) SHA1(0d8127bc86889426662944c6b0afb1a165343ee2) )
	ROM_LOAD16_BYTE( "9491.bin", 0x000000, 0x020000, CRC(f9f8cbd8) SHA1(0975a5e1b778158e9b68a40abcfaa06647e61378) )
	ROM_LOAD16_BYTE( "9492.bin", 0x000000, 0x020000, CRC(6fe20496) SHA1(a538ba97595345fddb6936082fb5f97e6b1e9054) )
	ROM_LOAD16_BYTE( "9493.bin", 0x000000, 0x020000, CRC(d6b890cf) SHA1(9a9680026bdfa4965c3bb438c64f02802782b879) )
	ROM_LOAD16_BYTE( "prom1n.bin", 0x000000, 0x020000, CRC(125f3f20) SHA1(ca525936ada9a1fb218c1f5685c2adb58782d86c) )
	ROM_LOAD16_BYTE( "prom1n_0.bin", 0x000000, 0x020000, CRC(352c5751) SHA1(8d486cc24aba1d04ea50aeaf7b6004575c4d83e0) )
	ROM_LOAD16_BYTE( "prom1n_7.bin", 0x000000, 0x020000, CRC(f750b225) SHA1(d02216abd1bbc0b8010c7fcd1c29ca3949196ff6) )
	ROM_LOAD16_BYTE( "prom1p.bin", 0x000000, 0x020000, CRC(f9c16b18) SHA1(d8cc65743e16ee4f0fa7c0e01c250912ffddb51e) )
	ROM_LOAD16_BYTE( "prom1p_0.bin", 0x000000, 0x020000, CRC(deb20369) SHA1(14894481736f106c54050d63b243f8fb8c2d5142) )
	ROM_LOAD16_BYTE( "prom1p_7.bin", 0x000000, 0x020000, CRC(1ccee61d) SHA1(dcd4942bc66d3109ea540fed50148c0e355b803d) )
	ROM_LOAD16_BYTE( "prom2.bin", 0x000000, 0x020000, CRC(384b5865) SHA1(26f731b6b418fdc3bb33d27237c7e91fbf9f7aff) )
	ROM_LOAD16_BYTE( "prom2_0.bin", 0x000000, 0x020000, CRC(7109053a) SHA1(5381771e52f0333abbf3789492bd8fc6be53eea4) )
	ROM_LOAD16_BYTE( "prom2_7.bin", 0x000000, 0x020000, CRC(deab0a4e) SHA1(aeba0182906332996efe79dfc56a14fdd087940e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6supbrk )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "subr-2_1.bin", 0x000000, 0x020000, CRC(85222c75) SHA1(1fbed0f474a7ccfb523270b913a8adb41c8be388) )
	ROM_LOAD16_BYTE( "subr-2_2.bin", 0x000001, 0x020000, CRC(bd0c6f72) SHA1(df0daea8c89b1f1c7a12fd7e4e54ad6a562bb1df) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "subr-2a1.bin", 0x000000, 0x020000, CRC(d7e6ec14) SHA1(41b89a59e311bb532a3e1d6632a56e4a87105abf) )
	ROM_LOAD16_BYTE( "subr-2n1.bin", 0x000000, 0x020000, CRC(b41494b1) SHA1(6aa0d8cc14a6851e887600d8f9612ddca7a58917) )
	ROM_LOAD16_BYTE( "subr-2np.bin", 0x000000, 0x020000, CRC(5f8ac089) SHA1(b37e4d778d72720159f2f94afb0b3797b0282d10) )
	ROM_LOAD16_BYTE( "subr-2p1.bin", 0x000000, 0x020000, CRC(6ebc784d) SHA1(04282ac643adbf1051477595856d45329c5b0c08) )
	ROM_LOAD16_BYTE( "subr-2w1.bin", 0x000000, 0x020000, CRC(e74f5dfd) SHA1(6a67d02298abd442df81dac7a797f16b8389507a) )
	ROM_LOAD16_BYTE( "subr-3_1.bin", 0x000000, 0x020000, CRC(3915930d) SHA1(f3ddbd23123722ff0dd8abb9ec02ecf5a49e2387) )
	ROM_LOAD16_BYTE( "subr-3_2.bin", 0x000000, 0x020000, CRC(6fb4c56d) SHA1(356caaa4b9af3f273a875ead7ee62ec76f7e9602) )
	ROM_LOAD16_BYTE( "subr-3a1.bin", 0x000000, 0x020000, CRC(6bd1536c) SHA1(37679582bd1ea213b7a744b8da0ad5dd5e4d3cb5) )
	ROM_LOAD16_BYTE( "subr-3n1.bin", 0x000000, 0x020000, CRC(08232bc9) SHA1(4cf6d8b1c849b19030eec815c6e9a938dc7f3e91) )
	ROM_LOAD16_BYTE( "subr-3p1.bin", 0x000000, 0x020000, CRC(d28bc735) SHA1(3a7e55c05cd8618b8fe8d887d3c071054123e21e) )
	ROM_LOAD16_BYTE( "subr3np1.bin", 0x000000, 0x020000, CRC(e3bd7ff1) SHA1(3b6105fc57ba246837829e1e01a79372603287f3) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "subr-snd.bin", 0x000000, 0x080000, CRC(d2439c80) SHA1(8c80927e0d0c139293bd588fad15941bfb54674d) )
ROM_END


ROM_START( j6swpdrp )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "swoptillyadrop.p1", 0x000000, 0x020000, CRC(8e372c50) SHA1(7264eb0072f2863a301e522eea83fcc5585538dc) )
	ROM_LOAD16_BYTE( "swoptillyadrop.p2", 0x000001, 0x020000, CRC(c0d16c07) SHA1(a19894fdedf01e9b57e82c473e4adbf2ad95a27e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "swoptillyadropsnd.bin", 0x000000, 0x080000, CRC(2ed0d1de) SHA1(76a8ae25f4932d84a4bdb6e67c30d366e4850d0c) )
ROM_END


ROM_START( j6bags )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tbf.p1", 0x000000, 0x020000, CRC(36605488) SHA1(0b8895288122427ba552bf3a3f5ec123c44a146c) )
	ROM_LOAD16_BYTE( "tbf.p2", 0x000001, 0x020000, CRC(b368c27c) SHA1(d83ed79205ff2f499584ab62173f753e4038752a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "tbfsnd.bin", 0x000000, 0x080000, CRC(73c2460a) SHA1(af1aed4f56690f4f055b28c6bd5c0296b98f4f8d) )
ROM_END


ROM_START( j6roof )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prom1n.bin", 0x000000, 0x020000, CRC(c00d171c) SHA1(8cd2992aa7ccb6e7d064ae736652df76030cad7b) )
	ROM_LOAD16_BYTE( "prom2.bin", 0x000001, 0x020000, CRC(6782d773) SHA1(dae453b9b4672f66228551d0c74a0d6c6690e95d) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "prom1p.bin", 0x000000, 0x020000, CRC(2b934324) SHA1(13864411cb12a68a6794fd063af725d9c130bdfe) )

	// something else?
	ROM_LOAD16_BYTE( "034p1-2h.bin", 0x000000, 0x008000, CRC(2b0353fa) SHA1(5c9f06fdda33c4a4a09c69f1e969ae4041513fd9) )
	ROM_LOAD16_BYTE( "034p1-2i.bin", 0x000000, 0x008000, CRC(a64797fc) SHA1(7437dc2e203efc525aab251da5196d31b95d159a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6tbirds )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "thbi-3_1.bin", 0x000000, 0x020000, CRC(ecbdd85c) SHA1(626306a72d658af6ecfa4b65212cf8aa35630539) )
	ROM_LOAD16_BYTE( "thbi-3_2.bin", 0x000001, 0x020000, CRC(61e98f71) SHA1(315044637bde8ab862af08bf9825917c87fcc77d) )

	ROM_REGION( 0x1000000, "altrevs", 0 ) // from a set marked as 'crystal'
	ROM_LOAD16_BYTE( "thbi-21.bin", 0x000000, 0x020000, CRC(86c15e0f) SHA1(d4ad355162f66ca43d7a0e9081e7cf328f21a505) )
	ROM_LOAD16_BYTE( "thbi-22.bin", 0x000000, 0x020000, CRC(5588d052) SHA1(ee24e97b21d3b21571c619e823e047b646f1dfc4) )
	ROM_LOAD16_BYTE( "thbi-2a1.bin", 0x000000, 0x020000, CRC(d4059e6e) SHA1(fe086c163591f7ef81c77181fae32f4b0d780ceb) )
	ROM_LOAD16_BYTE( "thbi-2p1.bin", 0x000000, 0x020000, CRC(6d5f0a37) SHA1(710b7a197047867030159c2e9f3c450ccf236952) )
	ROM_LOAD16_BYTE( "thbi-31.bin", 0x000000, 0x020000, CRC(7cdd730d) SHA1(8e73e1a745b82bab0cf767bfdc42751f0f32d0d5) )
	ROM_LOAD16_BYTE( "thbi-32.bin", 0x000000, 0x020000, CRC(f02d61db) SHA1(0d2a59e2b3ec68999340732a57f69eb516b6c74e) )
	ROM_LOAD16_BYTE( "thbi-3a1.bin", 0x000000, 0x020000, CRC(25dbdeb8) SHA1(68f1e467cf12f711e639d19de1afdf18ebbffe3b) )
	ROM_LOAD16_BYTE( "thbi-3p1.bin", 0x000000, 0x020000, CRC(9c814ae1) SHA1(42047fd73bc02da2add5f45882e8977a542988c4) )

	ROM_LOAD16_BYTE( "thbi-31_bin", 0x0000, 0x020000, CRC(771f1ed9) SHA1(aea7208b3f5887ca2a25842ce18bac62cf37a955) )

	ROM_LOAD16_BYTE( "t_birds.p1", 0x0000, 0x080000, CRC(d1a616c7) SHA1(cb917146e30e39ef87dd32f6ac1d4254f402f293) )
	ROM_LOAD16_BYTE( "t_birds.p2", 0x0000, 0x080000, CRC(516c3d6b) SHA1(4aa36dde3840956374fab4c34071625f865370d5) )

	ROM_REGION( 0x80000, "upd", 0 )
	ROM_LOAD( "tbcl-snd.bin", 0x000000, 0x80000, CRC(1cc197be) SHA1(2247aa1a0e6aab7389b3222f373890f54e907361) )
ROM_END

ROM_START( j6tbirdsa )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tbirdsp.p1", 0x000000, 0x020000, CRC(e59977e3) SHA1(69d7628ec50691f9363685de49013c9303e9bcc6) )
	ROM_LOAD16_BYTE( "tbirdsp.p2", 0x000001, 0x020000, CRC(7932c4ee) SHA1(c90fc982c433429aeafc4c787905d950209189f7) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "tbirdsps.bin", 0x000000, 0x080000, CRC(a0158242) SHA1(e0aac98f493f229eca2dbf934f938c6fa250fa6b) )
ROM_END


ROM_START( j6tbirdsb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tbirds.b1", 0x000000, 0x020000, CRC(6a034cfd) SHA1(2b79072cd90b40f369f1f00379b6249d1f3578ed) )
	ROM_LOAD16_BYTE( "tbirds.b2", 0x000001, 0x020000, CRC(45d1343a) SHA1(afbb33aa21d2e9834bdfb7c21124adbe3222b48b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "thbi-2_1.bin", 0x000000, 0x020000, CRC(a1ca874a) SHA1(0a12f387a9a3e261c1017922b8d6a652c696855a) )
	ROM_LOAD16_BYTE( "thbi-2_2.bin", 0x000000, 0x020000, CRC(65320620) SHA1(3db5ccf4e69d85bb8b0270b12674c64ff674834c) )
	ROM_LOAD16_BYTE( "thbi-2a1.bin", 0x000000, 0x020000, CRC(f30e472b) SHA1(e30b4c5c04f31ebf677af5f7924efc8836e6a91f) )
	ROM_LOAD16_BYTE( "thbi-2n1.bin", 0x000000, 0x020000, CRC(90fc3f8e) SHA1(6b3e50482e8b779ad0cad8509ba76efd055d6987) )
	ROM_LOAD16_BYTE( "thbi-2p1.bin", 0x000000, 0x020000, CRC(4a54d372) SHA1(bd7924eaeab185f097283c14b3bb197403853050) )
	ROM_LOAD16_BYTE( "thbi-2w1.bin", 0x000000, 0x020000, CRC(c3a7f6c2) SHA1(c72a05ed21801401d5aec610d30482560b00baee) )
	ROM_LOAD16_BYTE( "thbi-3a1.bin", 0x000000, 0x020000, CRC(be79183d) SHA1(13936aafbb5420748d74b531cbef0a1c39f9be5d) )
	ROM_LOAD16_BYTE( "thbi-3n1.bin", 0x000000, 0x020000, CRC(dd8b6098) SHA1(79d2fcf128fbfe14bd62cc649825fefc10d386a5) )
	ROM_LOAD16_BYTE( "thbi-3p1.bin", 0x000000, 0x020000, CRC(07238c64) SHA1(8653eb78537c22a89bcd90e69ddc226dd25dbd76) )
	ROM_LOAD16_BYTE( "thbi2np1.bin", 0x000000, 0x020000, CRC(7b626bb6) SHA1(ed8eefb2207f908b0c5bc3f315d014faccdfb493) )
	ROM_LOAD16_BYTE( "thbi3np1.bin", 0x000000, 0x020000, CRC(361534a0) SHA1(e2ad00f1698a69b71084dba99b75f40a5f59aa20) )


	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "tbcl-snd.bin", 0x000000, 0x80000, CRC(1cc197be) SHA1(2247aa1a0e6aab7389b3222f373890f54e907361) )
ROM_END


ROM_START( j6tbrdcl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tbcl-b_1.bin", 0x000000, 0x020000, CRC(6a034cfd) SHA1(2b79072cd90b40f369f1f00379b6249d1f3578ed) )
	ROM_LOAD16_BYTE( "tbcl-b_2.bin", 0x000001, 0x020000, CRC(45d1343a) SHA1(afbb33aa21d2e9834bdfb7c21124adbe3222b48b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "c001", 0x000000, 0x020000, CRC(499d9ed8) SHA1(929ea1d7c2aa4749666002f54727cddb06ecc280) )
	ROM_LOAD16_BYTE( "f0df", 0x000000, 0x020000, CRC(ce3dec34) SHA1(5675ceeb8a3d8f856af59cbb5ea348acf68bb33a) )
	ROM_LOAD16_BYTE( "f0fe", 0x000000, 0x020000, CRC(e1795b1c) SHA1(7bb080be3a05e48852caf4ecd1f1b2500823d7c6) )
	ROM_LOAD16_BYTE( "f0ff", 0x000000, 0x020000, CRC(0ae70f24) SHA1(c8cf9c654b6eac5a59454f5c7db2e1bc53c37972) )
	ROM_LOAD16_BYTE( "fofb", 0x000000, 0x020000, CRC(127c5346) SHA1(77d3cafc157d6f47cfde6aa023932107af18c0fe) )
	ROM_LOAD16_BYTE( "tbcl-bf1.bin", 0x000000, 0x020000, CRC(aed9afed) SHA1(6c4cbbc9344448b1a5ac14dc680049a7999e7d68) )
	ROM_LOAD16_BYTE( "tbcl-bn1.bin", 0x000000, 0x020000, CRC(086e3d75) SHA1(de51cb464f4e6a3174d950e62bd0dbb8d004af72) )
	ROM_LOAD16_BYTE( "tbcl-bp1.bin", 0x000000, 0x020000, CRC(819d18c5) SHA1(af41757ddba761db7564ad9531f85fb852e6c0de) )
	ROM_LOAD16_BYTE( "tbcl-e1.bin", 0x000000, 0x020000, CRC(04ef59bd) SHA1(f6d07217535238c2e6c7c8a3e78d5680944fe850) )
	ROM_LOAD16_BYTE( "tbcl-e2.bin", 0x000000, 0x020000, CRC(dd05618b) SHA1(14f6aa577d5a33ec3b744aea94c46dcecc9ae810) )
	ROM_LOAD16_BYTE( "tbcl-ee1.bin", 0x000000, 0x020000, CRC(1c7405df) SHA1(2d1e5bb1af37699765d0b5d8ecc8e39058835523) )
	ROM_LOAD16_BYTE( "tbcl-ef1.bin", 0x000000, 0x020000, CRC(c035baad) SHA1(65ebaa609dd0b6251a7697f48571bd9bf934c8ca) )
	ROM_LOAD16_BYTE( "tbcl-ep1.bin", 0x000000, 0x020000, CRC(ef710d85) SHA1(befbd30fc8807ca80c5fa9963a8025860624ac51) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 ) // which is correct?
	ROM_LOAD( "tbcl-snd.bin", 0x000000, 0x80000, CRC(1cc197be) SHA1(2247aa1a0e6aab7389b3222f373890f54e907361) )
	ROM_LOAD16_BYTE( "tbclbnp1.bin", 0x000000, 0x020000, CRC(e3f0694d) SHA1(cdc3853aba0e226c24e3b7ad2acd9527405b5bb0) )
ROM_END


ROM_START( j6tomb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tora-41.bin", 0x000000, 0x020000, CRC(80956d40) SHA1(0864e5fa7bb41dca2128566ccd80c1208dea0157) )
	ROM_LOAD16_BYTE( "tora-42.bin", 0x000001, 0x020000, CRC(4f4feeb4) SHA1(7d267a395e5450f71d0788a7b493bab133a622a0) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "tora-3a1.bin", 0x000000, 0x020000, CRC(8bfbc0d5) SHA1(a473fa489f1a238790c240085aa7971b7e965c75) )
	ROM_LOAD16_BYTE( "tora-3n1.bin", 0x000000, 0x020000, CRC(e809b870) SHA1(f291b1b6b3926c584ea055f4459592d6cc24f5be) )
	ROM_LOAD16_BYTE( "tora-3np.bin", 0x000000, 0x020000, CRC(0397ec48) SHA1(ff26c4696e4852bd5c1c67ed89ef2c54f84c2c69) )
	ROM_LOAD16_BYTE( "tora-3p1.bin", 0x000000, 0x020000, CRC(32a1548c) SHA1(a365cae2a1e3ebfc9b2ca401ba44cbbc72d4b822) )
	ROM_LOAD16_BYTE( "tora-4a1.bin", 0x000000, 0x020000, CRC(d251ad21) SHA1(d9bdd1d569e2a76780d59b72d6416f5983f1102b) )
	ROM_LOAD16_BYTE( "tora-4p1.bin", 0x000000, 0x020000, CRC(6b0b3978) SHA1(a4543c0ea19af39acc7c9ffd498079fbc5374d54) )
	ROM_LOAD16_BYTE( "tr_1.bin", 0x000000, 0x020000, CRC(d93f00b4) SHA1(edd3136fd52fa97de4dc45ebf72d7eaa85a8687f) )
	ROM_LOAD16_BYTE( "tr_2.bin", 0x000000, 0x020000, CRC(78cdf764) SHA1(61643d684b63916eaef2cd54535bae2ed575545f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "tr_snd.bin", 0x000000, 0x080000, CRC(6cd91050) SHA1(6818e59fc52b9776ee40bf7f2a8fca2f74343335) )
ROM_END


ROM_START( j6topflg )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "topflight.p1", 0x000000, 0x020000, CRC(f7e45700) SHA1(35034fa03ffb1c1c46becb6b4f194a5226d2db8d) )
	ROM_LOAD16_BYTE( "topflight.p2", 0x000001, 0x020000, CRC(98d1c5d5) SHA1(37497b0abd1586fda092bfabedd022a304dc00b0) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "topflightsnd.bin", 0x000000, 0x080000, CRC(c8e5b96e) SHA1(af62461a8279c2c09e5b5f93b5dce7ef0e973de6) )
ROM_END



ROM_START( j6tutti )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tutti_np_v240.p1", 0x000000, 0x020000, CRC(693d469e) SHA1(995e2278cb2570aae05166b640184ddcf4e12114) )
	ROM_LOAD16_BYTE( "tutti_np_v240.p2", 0x000001, 0x020000, CRC(23c46b1d) SHA1(ac0ca5ec2f2d3aa75de27d629bd3a19c02165d3d) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "61000040.bin", 0x000000, 0x080000, CRC(64963d14) SHA1(6ed36e371477ad924031a783a5adbe83acdfd7f9) )
ROM_END


ROM_START( j6twst )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "twister.p1", 0x000000, 0x020000, CRC(d86b391b) SHA1(cc6154c48d408295dbfbdb96e1cc364a6d93ec5f) )
	ROM_LOAD16_BYTE( "twister.p2", 0x000001, 0x020000, CRC(52e24bd7) SHA1(72c7b1e482e61b554c36f57f9aca3cf6208e4b5f) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "bb7e", 0x000000, 0x020000, CRC(b61680c6) SHA1(4dec5a6ac843dc2b085f27aacdc961d3b2c80044) )
	ROM_LOAD16_BYTE( "bb9d", 0x000000, 0x020000, CRC(cb96f78f) SHA1(8847620e5feebaa413ffc3623bd7d6cac9ccb1b7) )
	ROM_LOAD16_BYTE( "bbb5", 0x000000, 0x020000, CRC(3e7aac5b) SHA1(0452a3464672033effa85e7265a0f440205a26da) )
	ROM_LOAD16_BYTE( "bbbd", 0x000000, 0x020000, CRC(0f4c149f) SHA1(56b0bb49fc2b4115b19bf989c160129daf7806f0) )
	ROM_LOAD16_BYTE( "bbbe", 0x000000, 0x020000, CRC(e4d240a7) SHA1(a9e4f343b9f6a273de3a2748870ff7e6041e48a0) )
	ROM_LOAD16_BYTE( "ef39", 0x000000, 0x020000, CRC(39fbc848) SHA1(080925d9d1cf061dcbd11dd5a104ad4decaca20d) )
	ROM_LOAD16_BYTE( "twst-b1.bin", 0x000000, 0x020000, CRC(63d2a2a0) SHA1(91352ffa2ab0cbe46ad2ab192b6c155bec24a40b) )
	ROM_LOAD16_BYTE( "twst-b1a.bin", 0x000000, 0x020000, CRC(311662c1) SHA1(87029504817d24ac54bb786231f0d17e3b54c5ed) )
	ROM_LOAD16_BYTE( "twst-b1p.bin", 0x000000, 0x020000, CRC(884cf698) SHA1(a74f19d5910f35fc0c1196afb6f568517f1adb8b) )
	ROM_LOAD16_BYTE( "twst-b2.bin", 0x000000, 0x020000, CRC(a8665dc2) SHA1(f113d584a4449924fdf3753a7e0dafe6fb27f3b8) )
	ROM_LOAD16_BYTE( "twstb1kp.bin", 0x000000, 0x020000, CRC(b97a4e5c) SHA1(d6ebd9fa928c7f642a1bf1eb1a1b74d7862dc82c) )
	ROM_LOAD16_BYTE( "twstb1lp.bin", 0x000000, 0x020000, CRC(4c961588) SHA1(bf678dcf91378dcbd35df00134cd745a922ee0ee) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "twistersnd.bin", 0x000000, 0x080000, CRC(421a7a81) SHA1(d47caeb209eb6cfc47f82162b03563b25bbdf017) )
ROM_END


ROM_START( j6twstd )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "twister1.bin", 0x000000, 0x080000, CRC(7adb2a40) SHA1(fece9a0e8c20f988afb61f9bf34bb53d34ff379a) )
	ROM_LOAD16_BYTE( "twister2.bin", 0x000001, 0x080000, CRC(df09763b) SHA1(4691d050dc2adfc41fa8fef36dbc5299e0c921bf) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "twistersound.bin", 0x000000, 0x080000, CRC(66510699) SHA1(464fcd0a4279a911cf80a1e64453a477fb0b6fd4) )
ROM_END



ROM_START( j6untch )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7121.bin", 0x000000, 0x020000, CRC(ad5c67f1) SHA1(aabb433c8490c7672cf70157ab45f12292563291) )
	ROM_LOAD16_BYTE( "7122.bin", 0x000001, 0x020000, CRC(17a560d1) SHA1(295d7de024acde88780afa98a689337855da24cd) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD( "unto3bi.1", 0x0000, 0x020000, CRC(a111c9c0) SHA1(8a48fc16614231e8a9e1ea1f237f0baa13658dd9) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END





ROM_START( j6pompay )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20270.bin", 0x000000, 0x020000, CRC(c214a067) SHA1(9e4d26727b32051b188c361b8ad8922cbd7a10b2) )
	ROM_LOAD16_BYTE( "20271.bin", 0x000001, 0x020000, CRC(0a45e0e2) SHA1(a2fd22b732801db08739bae88a6df549546f62d8) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
//  ROM_LOAD16_BYTE( "up_1.bin", 0x000000, 0x020000, CRC(a079d1ef) SHA1(9097b639e457b9cc1e8333df40e6512abff3c6d4) )
//  ROM_LOAD16_BYTE( "up_2.bin", 0x000000, 0x020000, CRC(0a45e0e2) SHA1(a2fd22b732801db08739bae88a6df549546f62d8) )
	ROM_LOAD16_BYTE( "20272.bin", 0x000000, 0x020000, CRC(298af45f) SHA1(6b77c3357e3108d9d328c64e7e091aa81946c7de) )
	ROM_LOAD16_BYTE( "20273.bin", 0x000000, 0x020000, CRC(90d06006) SHA1(b48876e518b15499ac8d1fa19a4a0c56f624a3ce) )
	ROM_LOAD16_BYTE( "20274.bin", 0x000000, 0x020000, CRC(a079d1ef) SHA1(9097b639e457b9cc1e8333df40e6512abff3c6d4) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "uppo-so1.bin", 0x000000, 0x080000, CRC(60e370f8) SHA1(6b528f64ee5d00491655169bc108a7a6d383eaa5) )
ROM_END



ROM_START( j6vindal )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vindaloot.p1", 0x000000, 0x020000, CRC(141e55d6) SHA1(5472b19321fa7fbfa92e74f1849b772b593d69af) )
	ROM_LOAD16_BYTE( "vindaloot.p2", 0x000001, 0x020000, CRC(de043fa1) SHA1(1bde5d6a0da6f70ab8dcf70e549d40a0e8994b05) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "vindalootsnd.bin", 0x000000, 0x080000, CRC(2151baa1) SHA1(999915cb549e1bdae3079dea2895124becb96e2a) )
ROM_END



ROM_START( j6vivark )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vrve-2_1.bin", 0x000000, 0x020000, CRC(fea54f1c) SHA1(d5fcaa6417f0961bf370c90a468b5d59a17b62ba) )
	ROM_LOAD16_BYTE( "vrve-2_2.bin", 0x000001, 0x020000, CRC(eaca9d39) SHA1(3e41362fb3780a09a7647972d6df624fd8a73dba) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vrcl-e1.bin", 0x000000, 0x020000, CRC(f817a3c3) SHA1(e419939f22c28cbbe4c8e59398e0e7610abdbb17) ) // club set?
	ROM_LOAD16_BYTE( "vrcl-e2.bin", 0x000001, 0x020000, CRC(7b283bb4) SHA1(4841065fa9c4720184fa11227411e60eb40410bc) ) // club set?
	ROM_LOAD16_BYTE( "vrve-2n1.bin", 0x000000, 0x020000, CRC(cf93f7d8) SHA1(67cf78c7aed9f3d8929f05326134e353d34a066b) )
	ROM_LOAD16_BYTE( "vrve-2np.bin", 0x000000, 0x020000, CRC(240da3e0) SHA1(e4b7a68059602d8fdfd773e932815266db5cf2ac) )
	ROM_LOAD16_BYTE( "vrve-2p1.bin", 0x000000, 0x020000, CRC(153b1b24) SHA1(43d52f3cc68874a557844a6a045eb65beecf94aa) )
	ROM_LOAD16_BYTE( "vrve-2w1.bin", 0x000000, 0x020000, CRC(9cc83e94) SHA1(b14b5dc34320edcdacfe2f6a4955c0989b4013ec) )
	ROM_LOAD16_BYTE( "vrve-4_2.bin", 0x000000, 0x020000, CRC(bacf1474) SHA1(541a3afeba486d44eaf8063f729ab186bf75ec3d) )
	ROM_LOAD16_BYTE( "vrve-4a1.bin", 0x000000, 0x020000, CRC(3459e107) SHA1(93559fdd19371d1d72f40b476a90a849e71e1320) )
	ROM_LOAD16_BYTE( "vrve-6_1.bin", 0x000000, 0x020000, CRC(d96dafad) SHA1(68f3229fe0e2be6a258a9c0741c2f8a6f89313f5) )
	ROM_LOAD16_BYTE( "vrve-6_2.bin", 0x000000, 0x020000, CRC(524d7cd4) SHA1(3c24edd2725cd65487e74ca881c0f33c858e8095) )
	ROM_LOAD16_BYTE( "vrve-6a1.bin", 0x000000, 0x020000, CRC(8ba96fcc) SHA1(6aad6ff060fc5db98a5c52860424c79fd26dbc72) )
	ROM_LOAD16_BYTE( "vrve-6n1.bin", 0x000000, 0x020000, CRC(e85b1769) SHA1(abbfe158731e5c0531bf63e134be020335fa0c48) )
	ROM_LOAD16_BYTE( "vrve-6p1.bin", 0x000000, 0x020000, CRC(32f3fb95) SHA1(60ea60794db49af8b4ae1647042a3722c500caac) )
	ROM_LOAD16_BYTE( "vrve6np1.bin", 0x000000, 0x020000, CRC(03c54351) SHA1(a4c59f2c37104074ca4ede43bcdb275aacb8d9c5) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "vrve-snd.bin", 0x000000, 0x080000, CRC(40374f0b) SHA1(607eac4d3caee022e61531655ded137644602939) )
ROM_END



ROM_START( j6vivrkc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "vrcl-d_1s.bin", 0x000000, 0x020000, CRC(ba6a1f04) SHA1(4fbb326386d4d5dfc16e64b824b042932e0497ee) )
	ROM_LOAD16_BYTE( "vrcl-d_2s.bin", 0x000001, 0x020000, CRC(73cd7f94) SHA1(19854f8223e1614237686d42c7cbbd7853a83c62) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "vrcl-df1s.bin", 0x000000, 0x020000, CRC(7eb0fc14) SHA1(8801d6ae1ab321fe9757c9bd3021db52fea8c3f1) )
	ROM_LOAD16_BYTE( "vrcl-dp1s.bin", 0x000000, 0x020000, CRC(51f44b3c) SHA1(389b356dfd922f0e6fe258310166dc11fabe4432) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "vrcl-snds.bin", 0x000000, 0x080000, CRC(c4267ccf) SHA1(f299b63f762b420eaa5ddb024f357d7abb9fc21e) )
ROM_END



ROM_START( j6wldkng )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8472.bin", 0x000000, 0x020000, CRC(69d0a138) SHA1(b06e09dc55927b2032cfa62b9c65e4f16dfc0e7a) )
	ROM_LOAD16_BYTE( "8473.bin", 0x000001, 0x020000, CRC(5a39c96c) SHA1(b48a56b4cc252b51a1c02e8d04fcd79ede6e597c) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
//  ROM_LOAD16_BYTE( "8472s.bin", 0x000000, 0x020000, CRC(69d0a138) SHA1(b06e09dc55927b2032cfa62b9c65e4f16dfc0e7a) )
//  ROM_LOAD16_BYTE( "8473s.bin", 0x000000, 0x020000, CRC(5a39c96c) SHA1(b48a56b4cc252b51a1c02e8d04fcd79ede6e597c) )
	ROM_LOAD16_BYTE( "8474s.bin", 0x000000, 0x020000, CRC(824ef500) SHA1(0629bd346eaf4b7c76d31b8d82fef9ee37ca98fe) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "wiki-snds.bin", 0x000000, 0x080000, CRC(2ba0529c) SHA1(c4b4b80fcbb867650649a42a4abe7675eea8f848) )
ROM_END



ROM_START( j6wthing )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "wt1_1.p1", 0x000000, 0x020000, CRC(9c01e3c7) SHA1(b8a195c56fa3c3d14940525c177cbad60582196d) )
	ROM_LOAD16_BYTE( "wt1_1.p2", 0x000001, 0x020000, CRC(1dfd6f1b) SHA1(94146203997e9107dd08db164699e3669a31454b) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "wt1_0.p1", 0x000000, 0x020000, CRC(5f8db0ee) SHA1(0dfc26b846c347ec513f789ecef479f2de289193) )
	ROM_LOAD16_BYTE( "wt1_0.p2", 0x000000, 0x020000, CRC(8757a772) SHA1(b5a5fc7b16f2b696935db00f962b7d663119ea38) )
	ROM_LOAD16_BYTE( "w_thing.p1", 0x0000, 0x080000, CRC(e44d4b1b) SHA1(ff897fd60e6005cdbcee95c8621d1a1fc2be3442) )
	ROM_LOAD16_BYTE( "w_thing.p2", 0x0000, 0x080000, CRC(fd6c7571) SHA1(46dc3d8d76d70876b278050f2af4ae716205f3e7) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "wildsnd.bin", 0x000000, 0x080000, CRC(a2c08185) SHA1(9ee589df284f1b803ca015fff599d229358530d4) )
ROM_END



ROM_START( j6wildw )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9379.bin", 0x000000, 0x020000, CRC(533dcf84) SHA1(b1b973fc4dc0601da7587d9079e24319ecca5f1b) )
	ROM_LOAD16_BYTE( "9378.bin", 0x000001, 0x020000, CRC(2deb0db1) SHA1(a215b2d3d6dd9d2aa6c5be2ad5e0fdedafe3f557) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9380.bin", 0x000000, 0x020000, CRC(ea675bdd) SHA1(f328fd95c242bae08fb162b3b39f9498915639fe) )
	ROM_LOAD16_BYTE( "9381.bin", 0x000000, 0x020000, CRC(daceea34) SHA1(f6af9e91d482ea47a73e1392b017a6262652271e) )
	ROM_LOAD16_BYTE( "wildwest15-p1.bin", 0x000000, 0x020000, CRC(b8a39bbc) SHA1(bea3823a27e1d9b16bd9fd1a36603dfaedc8e428) )
	ROM_LOAD16_BYTE( "wildwest15-p2.bin", 0x000000, 0x020000, CRC(2deb0db1) SHA1(a215b2d3d6dd9d2aa6c5be2ad5e0fdedafe3f557) )
	ROM_LOAD16_BYTE( "wwe30.2", 0x000000, 0x020000, CRC(c1e36826) SHA1(15e80e9bc175610ec5840a94c17dd3472422e30b) )
	ROM_LOAD16_BYTE( "wwe30dsk.1", 0x000000, 0x020000, CRC(c6581dcd) SHA1(3f7c8c45574da4dab3b927e1e6fc5d1c8a942ddf) )
	ROM_LOAD16_BYTE( "wwe30exh.1", 0x000000, 0x020000, CRC(884fc937) SHA1(d51966519f609b2d2daf52594ef46ffc02b600ba) )
	ROM_LOAD16_BYTE( "wwe30nsk.1", 0x000000, 0x020000, CRC(2dc649f5) SHA1(0b792229a81a31c162d09cc35e3b05bb49e7e5c0) )
	ROM_LOAD16_BYTE( "wwe30wht.1", 0x000000, 0x017000, CRC(3fa65aca) SHA1(87055c50517e20d8f9ec90539c9bd701af244951) ) // bad?

	ROM_LOAD16_BYTE( "wiwe4p1", 0x000000, 0x020000, CRC(b8a39bbc) SHA1(bea3823a27e1d9b16bd9fd1a36603dfaedc8e428) ) // from a different set, was on it's own, maybe wrong game

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "wwe10snd.bin", 0x000000, 0x080000, CRC(00c8c428) SHA1(2e10b10093acd4c2f7051aff28a8ae976bb1425b) )
ROM_END


ROM_START( j6wizard )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "20418.bin", 0x000000, 0x020000, CRC(3a90026b) SHA1(2e766b97b1e1ad8cff6f5146a45e0f063cdbb0f7) )
	ROM_LOAD16_BYTE( "20419.bin", 0x000001, 0x020000, CRC(fb11230b) SHA1(13319192825d054034b3f3af8fc0d1020925b88f) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "20420.bin", 0x000000, 0x020000, CRC(d10e5653) SHA1(9da581cd6e23c0cea95ea4858725e0b5b6a987c8) )
	ROM_LOAD16_BYTE( "20421.bin", 0x000000, 0x020000, CRC(6854c20a) SHA1(36dc0d70691b659b0b1a9d1cad522decd3a1155c) )
	ROM_LOAD16_BYTE( "20422.bin", 0x000000, 0x020000, CRC(58fd73e3) SHA1(679760acf795c9c02ff1b9bb72a7947afc2aee52) )
	ROM_LOAD16_BYTE( "wizo15p1", 0x000000, 0x020000, CRC(75117690) SHA1(7ae6ef2360dd892da6eed345a2c5b5351a39b7a0) )
	ROM_LOAD16_BYTE( "wizo15p2", 0x000000, 0x020000, CRC(8954b864) SHA1(63445cfd59e64d1dcdee52185c41c32f037bf176) )
	ROM_LOAD16_BYTE( "wizod5p1", 0x000000, 0x020000, CRC(9c43b23d) SHA1(fac2327d5d41adc3af905642f33f541096467a2b) )
	ROM_LOAD16_BYTE( "wizod5p2", 0x000000, 0x020000, CRC(62460b0a) SHA1(89ba2bb4a9b7ea15462a86d5ccee5622850800af) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "woosnd.bin", 0x000000, 0x080000, CRC(f72bd4f4) SHA1(ef8651fe7bb5f5340e41d35ca0669cba7d9c1372) )
ROM_END



ROM_START( j6knight )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "yourluckynight.p1", 0x000000, 0x020000, CRC(fbf7d7d9) SHA1(885a8a5acd06bafe7df01d9e36c5315f9a7f518c) )
	ROM_LOAD16_BYTE( "yourluckynight.p2", 0x000001, 0x020000, CRC(996c1d3e) SHA1(f5127baee641619d7fb6ff66996eaa5aa4f45a88) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "278e", 0x000000, 0x020000, CRC(d3fe21b8) SHA1(a0813c4007b3058d0a23e3f658466e30bd91cc51) )
	ROM_LOAD16_BYTE( "6898", 0x000000, 0x020000, CRC(e6445233) SHA1(0f27531d823a5d67b8e963956313abd61163edcf) )
	ROM_LOAD16_BYTE( "68b7", 0x000000, 0x020000, CRC(9bc4257a) SHA1(aa4ed152aa1431272c48950526eb24f01dd9db12) )
	ROM_LOAD16_BYTE( "68d0", 0x000000, 0x020000, CRC(85b62a96) SHA1(994d2cc800efc426ccfe08a9b492278010cd64c5) )
	ROM_LOAD16_BYTE( "68d7", 0x000000, 0x020000, CRC(5f1ec66a) SHA1(b11b591c55e440befb07ab562ff3436bd2523499) )
	ROM_LOAD16_BYTE( "68d8", 0x000000, 0x020000, CRC(b4809252) SHA1(cdfd32a02be11f0e546e54036e995d072bd9ca49) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "yourluckynightsnd.bin", 0x000000, 0x080000, CRC(53c20160) SHA1(6295797d384fd00b4d982c924a4cfbaa079e93a1) )
ROM_END



ROM_START( j6svndb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ace.evn", 0x000000, 0x020000, CRC(0078e610) SHA1(545f3aa30154e7a75d72bc4621010177f7b1b441) )
	ROM_LOAD16_BYTE( "ace.odd", 0x000001, 0x020000, CRC(422268c4) SHA1(defaab4476aad6ad17ff917fa795a6a5a1828090) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD( "ace.bin", 0x000000, 0x040000, CRC(272070fd) SHA1(cd5f06106f0379b2769515193aee61dea04c1f1e) ) // just combined?

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END



ROM_START( j6reeltha )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "reel-31.bin", 0x000000, 0x020000, CRC(3e8020f8) SHA1(479f9e157eba570d0fa670f0f9ea7dbd3d94ca1f) )
	ROM_LOAD16_BYTE( "reel-32.bin", 0x000001, 0x020000, CRC(49b1cb81) SHA1(5f39416d4a74a7af36909bc2afc7a568957fc8c1) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "reel-3p1.bin", 0x000000, 0x020000, CRC(d51e74c0) SHA1(d0f7c4c30a6943c7e429e57b16b9eef989b253ff) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "reel-snd.bin", 0x000000, 0x080000, CRC(3178ddb0) SHA1(60be12e6198bd8b7cf021b54c3cd858ff5bac127) )
ROM_END




ROM_START( j6ewn )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ewn.p1", 0x000000, 0x020000, CRC(c1b3fa48) SHA1(fc250876b99af70562473e9d03c233dbf53a82c9) )
	ROM_LOAD16_BYTE( "ewn.p2", 0x000001, 0x020000, CRC(60f66b8d) SHA1(7ac2b741ecddd379c86a95bddcc9e0a82a5272b6) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "ewnsnd.bin", 0x000000, 0x020000, CRC(6d91e98c) SHA1(4acf46758089f2519027db148665aa75789d2d68) )
ROM_END



ROM_START( j6hikar )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "hikarate-p1.bin", 0x000000, 0x020000, CRC(de24d4d2) SHA1(9cebc0fe9476e6c555845f9ed42f13c52fd3486b) )
	ROM_LOAD16_BYTE( "hikarate-p2.bin", 0x000001, 0x020000, CRC(aaf75168) SHA1(0407464768fcd8b260926efbba3fd727df78a4f7) )



	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "hk4a1.b8", 0x000000, 0x020000, CRC(de24d4d2) SHA1(9cebc0fe9476e6c555845f9ed42f13c52fd3486b) )
	ROM_LOAD16_BYTE( "hk4a2.b8", 0x000000, 0x020000, CRC(aaf75168) SHA1(0407464768fcd8b260926efbba3fd727df78a4f7) )

	ROM_LOAD16_BYTE( "hika-51.bin", 0x00000, 0x020000, CRC(533bba89) SHA1(4abfb9fc925f47e65dc45511289a966acd88c200) )
	ROM_LOAD16_BYTE( "hika-52.bin", 0x00001, 0x020000, CRC(930351ba) SHA1(40705df03e858ffbef684606ce2d667859f40e05) )
	ROM_LOAD16_BYTE( "hika-51a.bin", 0x0000, 0x020000, CRC(01ff7ae8) SHA1(70073a34e8f0e01ceab7e8afb88c68282006080e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END



ROM_START( j6hisprt )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spir0_1e_demo.p1", 0x000000, 0x020000, CRC(016e68db) SHA1(efb9da76b16352588ba9a831210f135b13c0fec9) )
	ROM_LOAD16_BYTE( "spir0_1e_demo.p2", 0x000001, 0x020000, CRC(49b62046) SHA1(e07db0ce27896af4f508993d935135264cfe0ba1) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "spirsnd_demo.bin", 0x000000, 0x080000, CRC(15c6771f) SHA1(a99f142f53637af361699a73e229dcce224b117f) )
ROM_END



ROM_START( j6rcclub )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8478.bin", 0x000000, 0x020000, CRC(59ea4a54) SHA1(a5ec5a7e8dad51e6bed210ffacb9a7ee64046fa9) )
	ROM_LOAD16_BYTE( "8479.bin", 0x000001, 0x020000, CRC(8f45065a) SHA1(ee0ff3d4154879a5e77a438d68f6e482116fe235) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8480.bin", 0x000000, 0x020000, CRC(b2741e6c) SHA1(37aee7860e7f17262c003f0fc3ba0f87ce6c376a) )
	ROM_LOAD16_BYTE( "8481.bin", 0x000000, 0x020000, CRC(9d30a944) SHA1(76d83d29f0d83a187a1b0cc4c8c9fc3da51501c3) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6slvgst )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "9122.bin", 0x000000, 0x020000, CRC(6d3d9e98) SHA1(377adca43f1624b5f779132cbc833ff883531aa3) )
	ROM_LOAD16_BYTE( "9123.bin", 0x000001, 0x020000, CRC(8347b12f) SHA1(c2ed622aaf3e4de3ff8b2854eded793aed662c56) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "9124.bin", 0x000000, 0x020000, CRC(86a3caa0) SHA1(793cba57e6849b4bb4b821cee26a164a2911ff9d) )
	ROM_LOAD16_BYTE( "9125.bin", 0x000000, 0x020000, CRC(3ff95ef9) SHA1(df4ddeace9cb37c68ff70ff6302933f2f447c6c6) )
	ROM_LOAD16_BYTE( "9126.bin", 0x000000, 0x020000, CRC(0f50ef10) SHA1(3bc83d627b4f54ee39fcfa3aeb2f3b75517f7f00) )
	ROM_LOAD16_BYTE( "9150.bin", 0x000000, 0x020000, CRC(ea1de415) SHA1(7c2844e6e166fc045aaef6b1157205d63895901c) )
	ROM_LOAD16_BYTE( "9151.bin", 0x000000, 0x020000, CRC(372fd0b0) SHA1(2f186c95d0b5dd15385c0d6404a8a105400226b5) )
	ROM_LOAD16_BYTE( "9152.bin", 0x000000, 0x020000, CRC(0183b02d) SHA1(d8ddb012a141697abd1a198ea991ff33f7257a23) )
	ROM_LOAD16_BYTE( "9153.bin", 0x000000, 0x020000, CRC(b8d92474) SHA1(1ae7eb663a620fab8e7bf2f67eeea35607f081b1) )
	ROM_LOAD16_BYTE( "9154.bin", 0x000000, 0x020000, CRC(8870959d) SHA1(1d073d7ac82a1dd47473cce9aa238d23e45a90ac) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6footy )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ff1-03p1.bin", 0x000000, 0x020000, CRC(95f6f955) SHA1(1e7427d442a00f57f83fa16c8a485b110e3a01b8) )
	ROM_LOAD16_BYTE( "ff1-03p2.bin", 0x000001, 0x020000, CRC(ba2369ad) SHA1(9ba7bd782f1a287c9dc0a29e26c77fd82ad45f59) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "ff1-03i2.bin", 0x000000, 0x020000, CRC(542d0881) SHA1(e4ce44ec47ed1f5cc6c2f36b7d4ced75e3f9dfdb) )
	ROM_LOAD16_BYTE( "ff1-03x2.bin", 0x000000, 0x020000, CRC(bd4eadb4) SHA1(e3875456c67c3ea813bf13421089be4383731d4a) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "ffsnd.bin", 0x000000, 0x080000, CRC(99b8f4bd) SHA1(afc8e24db67f841a570b4cdd780a759a8fa13055) )
ROM_END



ROM_START( j6bbankr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
//  ROM_LOAD16_BYTE(  "bb21.bin", 0x0000, 0x020000, CRC(313ef240) SHA1(aa88ddb383c4c292ab610854407665842fedd374) )
//  ROM_LOAD16_BYTE(  "bb22.bin", 0x0000, 0x020000, CRC(3f246acd) SHA1(bd6813d1477da9ee8e7c289123ba3aed5b3bb076) )
	ROM_LOAD16_BYTE(  "bigbanker-crystal-p1.bin", 0x00001, 0x020000, CRC(3f246acd) SHA1(bd6813d1477da9ee8e7c289123ba3aed5b3bb076) )
	ROM_LOAD16_BYTE(  "bigbanker-crystal-p2.bin", 0x00000, 0x020000, CRC(313ef240) SHA1(aa88ddb383c4c292ab610854407665842fedd374) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END



ROM_START( j6bmc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "bimo-91.bin", 0x00000, 0x020000, CRC(691a15b8) SHA1(d48d80ed59b7d20a4910a5b0554e64f8fe324bef) )
	ROM_LOAD16_BYTE(  "bimo-92.bin", 0x00001, 0x020000, CRC(3a3503d2) SHA1(6e6bc2c07677b0a2416b084fb3204cc47373cc6a) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "bm71.b8", 0x0000, 0x020000, CRC(7bcca348) SHA1(9b4ac6b48c0ed258c5fd257c452661e664662866) )
	ROM_LOAD16_BYTE(  "bm72.b8", 0x0000, 0x020000, CRC(d28d8990) SHA1(cf92b5310db7ede40dd6c0e8d3f2a11b5bcb0745) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "bigmoneysnd.bin", 0x0000, 0x080000, CRC(dd0a88c6) SHA1(22206fcba097a4f7dc6ae84d496d149a4206e0f0) )
ROM_END




ROM_START( j6bno )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "bignightout.p1", 0x00000, 0x020000, CRC(d9a9858f) SHA1(103a31682eb28f0585e948ad8f59887ad2976f40) )
	ROM_LOAD16_BYTE(  "bignightout.p2", 0x00001, 0x020000, CRC(d17a7e13) SHA1(f40e46070a59e10cb08ec47f7ce53694ef13b311) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "bignite.g1", 0x0000, 0x020000, CRC(b06a2513) SHA1(36012ad32a8ad824028866fdee86847a042dc8c5) )
	ROM_LOAD16_BYTE(  "bignite.g2", 0x0000, 0x020000, CRC(d929f64e) SHA1(ed027f97adee2188a342908d8224a796b6b0991b) )
	ROM_LOAD16_BYTE(  "bnol1.b8", 0x0000, 0x020000, CRC(8dc17175) SHA1(7af502eb90f2d7ab14d747ef9b96f2eade70463a) )
	ROM_LOAD16_BYTE(  "bnol2.b8", 0x0000, 0x020000, CRC(2170044b) SHA1(f985bb42b84f5729ec3ce9d5b8c9ec47c6380e52) )
	ROM_LOAD16_BYTE(  "bnolp.b8", 0x0000, 0x020000, CRC(665f254d) SHA1(65ea616d46664882d84b9945cca384e175a18f3f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "bignightoutsnd.bin", 0x0000, 0x080000, CRC(e4da3e2e) SHA1(9751dfa7f9ca11b7073742b0ba2bf90fb483452b) )
	//ROM_LOAD(  "bignite.as1", 0x0000, 0x080000, CRC(e4da3e2e) SHA1(9751dfa7f9ca11b7073742b0ba2bf90fb483452b) )
ROM_END




ROM_START( j6btbw )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "a7df", 0x00000, 0x020000, CRC(9620c56e) SHA1(be1818cb5f5c9ed88fa250e7470ca18724a0bc0b) )
	ROM_LOAD16_BYTE(  "2c84", 0x00001, 0x020000, CRC(37e242d4) SHA1(66c7327272ef469c44937137e86406edf0ebd5e1) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "a7f7", 0x0000, 0x020000, CRC(63cc9eba) SHA1(32396f0f64fbcfbf3e824b1557ff8b34c5f1e041) )
	ROM_LOAD16_BYTE(  "a7fb", 0x0000, 0x020000, CRC(4a617a1c) SHA1(14cbb644ffd8b28fb1856e0e61133511a7a91671) )
	ROM_LOAD16_BYTE(  "a7fe", 0x0000, 0x020000, CRC(b9647246) SHA1(daad9b7a3b2ab9899671c07ee2cd2af106fe612c) )
	ROM_LOAD16_BYTE(  "a7ff", 0x0000, 0x020000, CRC(52fa267e) SHA1(b9f63e50a0b2fe39c3ce43914c05136e11536200) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "borntobewildsnd.bin", 0x0000, 0x080000, CRC(2c590926) SHA1(d5f56624d1f8f9692004937f98cadde78c2606bc) )
ROM_END




ROM_START( j6cpal )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "cpa1.bin", 0x00000, 0x020000, CRC(84f4c191) SHA1(bc5466b03818b653e7286ab95cfda0d2e9f64251) )
	ROM_LOAD16_BYTE(  "cpa2.bin", 0x00001, 0x020000, CRC(9b0ca91a) SHA1(86e49c02966ee62e030d526a6329d3be715fb3db) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "cpa1p.bin", 0x0000, 0x020000, CRC(6f6a95a9) SHA1(96d54598abcf887afeb1c878c6b4acbce7d24039) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6cpala )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "capa-b1.bin", 0x00000, 0x020000, CRC(03855437) SHA1(92a6f9feeed7b0a0c1c420561bc74892ec0a5c89) )
	ROM_LOAD16_BYTE( "capa-b2.bin", 0x00001, 0x020000, CRC(74b590bc) SHA1(0c11b97b15eee082133340bc685e4257d769b88f) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "capa-7p1.bin", 0x0000, 0x020000, CRC(5718b588) SHA1(2233a9848c7520e3158e4b69d2253a5c023e7015) )
	ROM_LOAD16_BYTE( "capa-7s1.bin", 0x0000, 0x020000, CRC(bc86e1b0) SHA1(51a00a30fb9fac86f2790fb6904298561f8f4183) )
	ROM_LOAD16_BYTE( "capa-7w1.bin", 0x0000, 0x020000, CRC(deeb9038) SHA1(167ad8e3984c13791db15b53194fa0e3b5f79b76) )
	ROM_LOAD16_BYTE( "capa1.bin", 0x0000, 0x020000, CRC(ee4221d1) SHA1(d79440ddbb7083c95d15a00e3e9395b43a1af376) )
	ROM_LOAD16_BYTE( "capa2.bin", 0x0000, 0x020000, CRC(775f6d12) SHA1(03559f8f7830b84cb718e0d005d842ddf515b6a1) )

	ROM_LOAD16_BYTE( "fm519d11.bin", 0x0000, 0x010000, CRC(0272325e) SHA1(2f632ea7246c2afd485b11a03afeef4c9e30f5cf) ) // this is some other game on different hw by the looks of it0

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "cpalace22-06-98 68d0a91.bin", 0x0000, 0x080000, CRC(56d581e0) SHA1(d22796ec6d96f4d4ea10dfdb925ceaff032fe7d0) )
	ROM_LOAD( "capa-snd.bin", 0x0000, 0x080000, CRC(56d581e0) SHA1(d22796ec6d96f4d4ea10dfdb925ceaff032fe7d0) )
ROM_END

ROM_START( j6cpalb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "ceas_pal.p1", 0x00000, 0x020000, CRC(e8df4b09) SHA1(c04fcede76eff727aed979cd7bed5401be2480e3) )
	ROM_LOAD16_BYTE( "ceas_pal.p2", 0x00001, 0x020000, CRC(4569254c) SHA1(f058dd5b97bd94e59fd952f42696f1f0aaede016) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( j6cdivr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "fsd.p1", 0x00000, 0x020000, CRC(e9266e02) SHA1(407b99a329b91864d6552d4ae2e13d8a5880930b) )
	ROM_LOAD16_BYTE(  "fsd.p2", 0x00001, 0x020000, CRC(9be40086) SHA1(7b24c3e42299f09a2ad66785673a28ff326537cf) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6ccc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "ccb1.bin", 0x00000, 0x020000, CRC(38a9b16e) SHA1(2ee0e1c67064e537b4459a69dc9ebf8be89f9051) )
	ROM_LOAD16_BYTE(  "ccb2.bin", 0x00001, 0x020000, CRC(5628a3e9) SHA1(79506828905e215fca410554a1dd1bac2050f11a) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "ccbp1.bin", 0x0000, 0x020000, CRC(d337e556) SHA1(945461142b7e0b39f12c29c64713483aeed073ba) )
	ROM_LOAD16_BYTE(  "cce1.bin", 0x0000, 0x020000, CRC(0906747f) SHA1(ad9bb38843b81d0ef948b140448de1dab2058786) )
	ROM_LOAD16_BYTE(  "cce2.bin", 0x0000, 0x020000, CRC(33186e30) SHA1(8290c3128596a6e3d11e1af9c120f80defed5f97) )
	ROM_LOAD16_BYTE(  "ccep1.bin", 0x0000, 0x020000, CRC(e2982047) SHA1(dc4991bb74358f3022e00ce81812687fa71d01ee) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6colic )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "coli-11.bin", 0x00000, 0x020000, CRC(1dbfc333) SHA1(264b2ad65d58ad7c6b569d2896af69973709d5dc) )
	ROM_LOAD16_BYTE(  "coli-12.bin", 0x00001, 0x020000, CRC(1d938c67) SHA1(1a6e7a472d55f17f92d29bd99fcd6a753eb856d9) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "coli-1a1.bin", 0x0000, 0x020000, CRC(4f7b0352) SHA1(b98791c5af9262e7a389525178b4270146a9a825) )
	ROM_LOAD16_BYTE(  "coli-1n1.bin", 0x0000, 0x020000, CRC(2c897bf7) SHA1(b7418506de2600b1eeb155849cce821eba629d74) )
	ROM_LOAD16_BYTE(  "coli-1p1.bin", 0x0000, 0x020000, CRC(f621970b) SHA1(e772093135321e256483dd4a6e72a1366d7e28c0) )
	ROM_LOAD16_BYTE(  "coli1np1.bin", 0x0000, 0x020000, CRC(c7172fcf) SHA1(fee2f9023b651e4c2a799f413060536a775f43c5) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6crakr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "crac-11.bin", 0x00000, 0x020000, CRC(facb09cd) SHA1(3f70f3616a2201d25d3712012b4c56cfa09c1411) )
	ROM_LOAD16_BYTE(  "crac-12.bin", 0x00001, 0x020000, CRC(16472d00) SHA1(cea574e7c8e42b8832f9849899e56d9b73d97ed1) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "crac-1a1.bin", 0x0000, 0x020000, CRC(a80fc9ac) SHA1(2ad7d1e18c1ad1560039cda63014d17a5151379f) )
	ROM_LOAD16_BYTE(  "crac-1p1.bin", 0x0000, 0x020000, CRC(11555df5) SHA1(7a1bb63b40cb3169bf0f4c45a032beba911f2d30) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6ewndg )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "ewnc1.bin", 0x00000, 0x020000, CRC(9ee88773) SHA1(4b0772ddf7061300a914fe1a47d696e50be8c04b) )
	ROM_LOAD16_BYTE(  "ewnc2.bin", 0x00001, 0x020000, CRC(791b2108) SHA1(3f8f12e2e3941d95f0332eb828a3f4be60a81742) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "ewn8_8.b16", 0x0000, 0x040000, CRC(674fd826) SHA1(34b17cdc4b8eb67c1100e62dee8bf13c66a5fa82) )
	ROM_LOAD16_BYTE(  "ewnc1ss.bin", 0x0000, 0x020000, CRC(cc2c4712) SHA1(15729b5bccdb5f0e71bdedbe8817988c186a9d7e) )
	ROM_LOAD16_BYTE(  "ewncp1.bin", 0x0000, 0x020000, CRC(27b2132a) SHA1(144f491eeaed2b6e0203e31fe5f78280edc41c1e) )
ROM_END




ROM_START( j6easy )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "emb1.b8", 0x00000, 0x020000, CRC(edeab2f6) SHA1(5fa20c2323049a1f92284fb331d836de7fc907c5) )
	ROM_LOAD16_BYTE(  "emb2.b8", 0x00001, 0x020000, CRC(b8344d42) SHA1(864d28960112095f23b62648e296ea068cd824ce) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "embp1.b8", 0x0000, 0x020000, CRC(0674e6ce) SHA1(698130fae4a40f289407ac56a221c77a493ee16b) )
	ROM_LOAD16_BYTE(  "emc1.b8", 0x0000, 0x020000, CRC(95a1c269) SHA1(d0502c3408a285523f718de54df8ef1a265a0221) )
	ROM_LOAD16_BYTE(  "emc2.b8", 0x0000, 0x020000, CRC(5b4ecf73) SHA1(327b9a517e1a2c22e1da6c10fb32eb16bc182d5c) )
	ROM_LOAD16_BYTE(  "emcp.b8", 0x0000, 0x020000, CRC(7e3f9651) SHA1(ad14b3b7a14c1b86fae94f44f3dfc52cfbfd136f) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "easymoneysnd.bin", 0x0000, 0x080000, CRC(aee5bc30) SHA1(071038e7fc7767b7b11c9b97b41e079fbbe11291) )
ROM_END





ROM_START( j6ffc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "eedf", 0x00000, 0x020000, CRC(cba081fa) SHA1(bdb547c2922e9b8bccd9c12cc6aa8b12ef4bc3b2) )
	ROM_LOAD16_BYTE(  "c0f2", 0x00001, 0x020000, CRC(fb68bbba) SHA1(4978d305409d5f024ed2ef18b32aa5cc582ce83d) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "eef7", 0x0000, 0x020000, CRC(3e4cda2e) SHA1(e5d2d57d114230356afd50e0a854449c30b84bf3) )
	ROM_LOAD16_BYTE(  "eefb", 0x0000, 0x020000, CRC(17e13e88) SHA1(9f51f0d32370e5bb9d3efe60c6a83dcffaff586b) )
	ROM_LOAD16_BYTE(  "eefe", 0x0000, 0x020000, CRC(e4e436d2) SHA1(e8c3bf1ef1737bb4bc59a7e1c2be33acec9db56b) )
	ROM_LOAD16_BYTE(  "eeff", 0x0000, 0x020000, CRC(0f7a62ea) SHA1(9867488b8ceeb24b5c7c7f5824603f6310b1b97d) )
	ROM_LOAD16_BYTE(  "frfo-d1.bin", 0x0000, 0x020000, CRC(711ac380) SHA1(d85298372fc6fd473fc84038c3c05733436bbb9f) )
	ROM_LOAD16_BYTE(  "frfo-d2.bin", 0x0000, 0x020000, CRC(1744b355) SHA1(1c5f07f1b43ff189e9618bf6012485cab7894712) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6grc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "fqb1.bin", 0x00000, 0x020000, CRC(e479c554) SHA1(397a36e12ebe47d67ad209c66a2da302191e16e2) )
	ROM_LOAD16_BYTE(  "fqb2.bin", 0x00001, 0x020000, CRC(19c653df) SHA1(f5fd7bc0d68eadfa241a01d5c5d7bd91642e0376) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "fqbp1.bin", 0x0000, 0x020000, CRC(0fe7916c) SHA1(f251f5fd07ceeb7aa9e09cb95a07f28f80cd59ce) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6hdc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "club-hotdogs.p1", 0x00000, 0x020000, CRC(791a59d5) SHA1(5653638d9c138471afcbee9b0e81246d76d7d57f) )
	ROM_LOAD16_BYTE(  "club-hotdogs.p2", 0x00001, 0x020000, CRC(4b993b62) SHA1(dd0d6f289bbbc28b83a803be822293f2d2125347) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "29a7", 0x0000, 0x020000, CRC(23b61c92) SHA1(f0fb308f93ec3534ce583da5af5e772a219a0505) )
	ROM_LOAD16_BYTE(  "c9f7", 0x0000, 0x020000, CRC(067976c6) SHA1(cf18b659ed6c372b5a61f6aca991f23897144953) )
	ROM_LOAD16_BYTE(  "c9fe", 0x0000, 0x020000, CRC(dcd19a3a) SHA1(5d39070d9d8dbb0c1f6c544ea51a0f8428fc50c4) )
	ROM_LOAD16_BYTE(  "c9ff", 0x0000, 0x020000, CRC(374fce02) SHA1(4e5d7d452f6345388a5989d0059344000c691161) )
	ROM_LOAD16_BYTE(  "chd4d_1f", 0x0000, 0x020000, CRC(bdc0bac5) SHA1(875336aac8a3b456aa24e2f683617d76c415d0ea) )
	ROM_LOAD16_BYTE(  "chd4d_1j", 0x0000, 0x020000, CRC(482ce111) SHA1(0d82973f8123b2174d8d2df9ca7f3929da449dd3) )
	ROM_LOAD16_BYTE(  "chd4d_1p", 0x0000, 0x020000, CRC(92840ded) SHA1(31b4161226a771747b488ebd81e9539928ae5dcb) )
	ROM_LOAD16_BYTE(  "chd4d_1s", 0x0000, 0x020000, CRC(dc93d917) SHA1(1f4f0c33fa6bfd5f4070a154983447e8e5c75fa9) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "club-hotdogssnd.bin", 0x0000, 0x080000, CRC(2bd7871f) SHA1(dc129f64f7186c02f4283229b579275ecb3a1165) )
ROM_END




ROM_START( j6impls )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "impc1.b8", 0x00000, 0x020000, CRC(b4b75883) SHA1(95e5e87df6eca95d3317a2b0a4ab487c6ec2d0cb) )
	ROM_LOAD16_BYTE(  "impc2.b8", 0x00001, 0x020000, CRC(0cffbc0d) SHA1(c8f4dc280526cb2ab23c6fe82939c1c228948acf) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6kapang )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "kapa-41.bin", 0x00000, 0x020000, CRC(34509f12) SHA1(b7de8c6004b9638365dceed79d9a829587c45ab4) )
	ROM_LOAD16_BYTE(  "kapa-42.bin", 0x00001, 0x020000, CRC(5d03b7eb) SHA1(a5e7a0674eb0d5798cdcd467f76aed92d15f9df8) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "kapa-4p1.bin", 0x0000, 0x020000, CRC(dfcecb2a) SHA1(fd8930a1ec80159358acf0a4ea65fd0addaa9d23) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6kfc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "kfcl-e1.bin", 0x00000, 0x020000, CRC(f16857a0) SHA1(5cff87bf3857593c7d032315b7555e16c3849d08) )
	ROM_LOAD16_BYTE(  "kfcl-e2.bin", 0x00001, 0x020000, CRC(9da62c89) SHA1(24d35a4c4e45d0a9bd6d0557c373b7d63f6a83aa) )

	ROM_LOAD16_BYTE( "kfcl-f1.bin", 0x00000, 0x020000, CRC(4edfa088) SHA1(e23d18c94c19edaadb73e31f0526c4a3f40c5e7e) )
	ROM_LOAD16_BYTE( "kfcl-f2.bin", 0x00001, 0x020000, CRC(88d37b10) SHA1(8954e7f4a7401dc93f9f4e16d0ae9ff6d2b911d8) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "kfcl-ep1.bin", 0x0000, 0x020000, CRC(1af60398) SHA1(45b26b983f82298c9cb14eeb23ea30f24b8ab0e7) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "kungfuclubsound.bin", 0x0000, 0x080000, CRC(659dfb1a) SHA1(0094cdee97c82a05358e8fcc6157f761c51c3655) )
ROM_END




ROM_START( j6lucklo )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "llb1.bin", 0x00000, 0x020000, CRC(e63a2c97) SHA1(b9e02de1c82d761209ee185b29bd248785e07cbe) )
	ROM_LOAD16_BYTE(  "llb2.bin", 0x00001, 0x020000, CRC(0d6de39d) SHA1(eb696d7cbecef4033a7f539430b9e970a82e757c) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "llbp1.bin", 0x0000, 0x020000, CRC(0da478af) SHA1(76a7f84eea5fdde8c08d5f56a612097235c4d1b8) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6monst )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "monster.p1", 0x00000, 0x020000, CRC(979ba29d) SHA1(44258fc5c41794c84d0b034cc5bc789d2365f641) )
	ROM_LOAD16_BYTE(  "monster.p2", 0x00001, 0x020000, CRC(5871a333) SHA1(eb2e1dbd8c2acc7717a3178c6fe20bcde34cf2ff) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "f57f", 0x0000, 0x020000, CRC(3212225f) SHA1(7b76db4cf9cb0a656cbc43671bf453c9834b71c5) )
	ROM_LOAD16_BYTE(  "f5df", 0x0000, 0x020000, CRC(5341418d) SHA1(fc744d7508b4d0d67748bc135733244eec623758) )
	ROM_LOAD16_BYTE(  "f5f7", 0x0000, 0x020000, CRC(a6ad1a59) SHA1(df7f71f73d0c0dd318df17c6ec3b3788dbcfe488) )
	ROM_LOAD16_BYTE(  "f5fe", 0x0000, 0x020000, CRC(7c05f6a5) SHA1(96b5c0c21119a343a94c608690766fdf8f0fc300) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "bad1snd.bin", 0x0000, 0x080000, CRC(1b61dcf9) SHA1(3ba4c8d9b77c86fbb931af0c0d9808ac68d0aa25) )
ROM_END




ROM_START( j6pinfvr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "pife-31.bin", 0x00000, 0x020000, CRC(0ce84585) SHA1(ef5781f1bed3e47169a7f10145557906b2b401b1) )
	ROM_LOAD16_BYTE(  "pife-32.bin", 0x00001, 0x020000, CRC(b5e9a22c) SHA1(4694a30a11d18fedb526085353e6f784cc3ec3f9) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "pfversnd.bin", 0x0000, 0x080000, CRC(27ab8a98) SHA1(afaf064d31a080a5a4c03e6141e062e33ec23353) )
ROM_END





ROM_START( j6pinwzdc )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "pwc1.b8", 0x00000, 0x020000, CRC(f6099987) SHA1(0aabc7ba43b9144cf4a15354c4a9a17d54ebec0d) )
	ROM_LOAD16_BYTE(  "pwc2.b8", 0x00001, 0x020000, CRC(ea96ce13) SHA1(8897eb2decd2521dda2194a3be25d2b03e286d94) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "pwd1.b8", 0x0000, 0x020000, CRC(b55c9ba9) SHA1(f53d68079378229f2311b190f8eda4b35b1d9913) )
	ROM_LOAD16_BYTE(  "pwd2.b8", 0x0000, 0x020000, CRC(ffe32710) SHA1(33ab7c95575587d036b3fc94d1f7363164748a97) )
	ROM_LOAD16_BYTE(  "pwp.b8", 0x0000, 0x020000, CRC(5ec2cf91) SHA1(0d8a63c884a33f1d46e27e2ce5b1a77b140b4062) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END





ROM_START( j6potg )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "fxd.p1", 0x00000, 0x020000, CRC(113ddbe2) SHA1(492e320c296604ebd7bd5a55356698bfa36c1d0d) )
	ROM_LOAD16_BYTE(  "fxd.p2", 0x00001, 0x020000, CRC(c31eea57) SHA1(9b2a5cccbc7254c9a6b51f7ba5cc43c06578ed6e) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "fxd_a.p1", 0x0000, 0x020000, CRC(43f91b83) SHA1(eaa1c1aa414ff378a7cb6820cfce7cc88fbfc559) )
	ROM_LOAD16_BYTE(  "fxd_p.p1", 0x0000, 0x020000, CRC(faa38fda) SHA1(437c1a34a044d2d4e17d1e94e145f3733d15bcf0) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6ramese )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "rari-b1.bin", 0x00000, 0x020000, CRC(a9d6f938) SHA1(ab0f9fb178708ff8660f37495f1c2579c9370b27) )
	ROM_LOAD16_BYTE(  "rari-b2.bin", 0x00001, 0x020000, CRC(99483b27) SHA1(734c68d6dab75cf2c8aae7e943f4feb49c592918) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "73ae", 0x0000, 0x020000, CRC(74cc2c21) SHA1(94827e336d273207e8550ca775be7fb086a39566) )
	ROM_LOAD16_BYTE(  "7fdf", 0x0000, 0x020000, CRC(df11398e) SHA1(ea26644faec4ee3138af5b784c31c3548792899c) )
	ROM_LOAD16_BYTE(  "7ffb", 0x0000, 0x020000, CRC(035086fc) SHA1(e5d0457048e6866f743054efefdac03d9cd3620b) )
	ROM_LOAD16_BYTE(  "7ffe", 0x0000, 0x020000, CRC(f0558ea6) SHA1(2fe1918200126faae7bcfdb9ff8722ccc3fe4f57) )
	ROM_LOAD16_BYTE(  "7fff", 0x0000, 0x020000, CRC(1bcbda9e) SHA1(916945650feacfd5c476d01837dc78efafde3c75) )
	ROM_LOAD16_BYTE(  "rari-a1.bin", 0x0000, 0x020000, CRC(b7f68ceb) SHA1(29bf459564c1f33ea4987c6d2e979489f0837a9c) )
	ROM_LOAD16_BYTE(  "rari-a2.bin", 0x0000, 0x020000, CRC(2d50dc66) SHA1(5511b12d1cb1f5e28b1cbb74981f57fe9f955a60) )
	ROM_LOAD16_BYTE(  "rari-bp1.bin", 0x0000, 0x020000, CRC(4248ad00) SHA1(a5c0c3e924f52f1b6e9a072dc5c7f5de288b44f6) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "ramesesrichessnd.bin", 0x0000, 0x080000, CRC(0173169e) SHA1(ccba7f6d41193f556af8ef6c827b482277ee0ee2) )
ROM_END




ROM_START( j6r2rum )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "rrb1.b8", 0x00000, 0x020000, CRC(0970162e) SHA1(5e83f7b6ca1e2b33a5adbbb8ad69c60c213d2ed3) )
	ROM_LOAD16_BYTE(  "rrb2.b8", 0x00001, 0x020000, CRC(17d03879) SHA1(d5d569435a3e131f71dcd4e4dd56c227801fe898) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "rrba.b8", 0x0000, 0x020000, CRC(5bb4d64f) SHA1(6b61181a73dc08a9e7fe6206a339dedd311505cf) )
	ROM_LOAD16_BYTE(  "rrbp.b8", 0x0000, 0x020000, CRC(e2ee4216) SHA1(d9a29b04ca6018bfd03bb09c2391d69a1a27e271) )
	ROM_LOAD16_BYTE(  "rtr-11.bin", 0x0000, 0x020000, CRC(979246a1) SHA1(78666f916bced21a6057f2c96fad8dde70df14bd) )
	ROM_LOAD16_BYTE(  "rtr-12.bin", 0x0000, 0x020000, CRC(1bc680a3) SHA1(43cc3e3b24de205345350ae6ac773969f698ae09) )
	ROM_LOAD16_BYTE(  "rtr-1a1.bin", 0x0000, 0x020000, CRC(c55686c0) SHA1(48d4a24585a6d5735c07f9df16270d12b6f74030) )
	ROM_LOAD16_BYTE(  "rtr-1p1.bin", 0x0000, 0x020000, CRC(7c0c1299) SHA1(78f96af3f8c91c14f4341c74d8b89b7486bfaa56) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "rtrsnd.bin", 0x0000, 0x080000, CRC(ba72e377) SHA1(99e123eebb8e7ceb2fb36fd17f1c23d3ce04d2d6) )
ROM_END




ROM_START( j6redal )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "real-c1.bin", 0x00000, 0x020000, CRC(eabec7ae) SHA1(5722dc489b3cf5ff90d9688d52fd8489d80b9c96) )
	ROM_LOAD16_BYTE(  "real-c2.bin", 0x00001, 0x020000, CRC(104bbeee) SHA1(1f4a3d05c729e03f91b9fa09c0d09b952ea2ce9d) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "raa1a.bin", 0x0000, 0x020000, CRC(839e7773) SHA1(57dcd6fcf933ea3f2d903a62f673a1c71d028748) )
	ROM_LOAD16_BYTE(  "raa2.bin", 0x0000, 0x020000, CRC(bbfa6d4d) SHA1(2a1e1df72acb95e1e8eaa498727aafd1734bb5d1) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6reelb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "fg81.bin", 0x00000, 0x020000, CRC(570cd8b3) SHA1(91563b30d2ff229a000dfb3299a9cf343517fc72) )
	ROM_LOAD16_BYTE(  "fg82.bin", 0x00001, 0x020000, CRC(952a7743) SHA1(908e03279f0b98921b417e4c85117e0ef7f5c8b3) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "fg81p.bin", 0x0000, 0x020000, CRC(bc928c8b) SHA1(6411994eebdfaac494725f64446cb711eb54c2b7) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6sl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "s&l31.b8", 0x00000, 0x020000, CRC(e8cadadb) SHA1(e7edfa79e1648658e200ddbbb1d280641e03b93e) )
	ROM_LOAD16_BYTE(  "s&l32.b8", 0x00001, 0x020000, CRC(b2251a43) SHA1(024c82509026561022ebcd3b2654d5beaa838d24) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "s&l3p.b8", 0x0000, 0x020000, CRC(03548ee3) SHA1(f4727303725b005ae04d9e4572a9249c07b705f3) )
	ROM_LOAD16_BYTE(  "sal111.bin", 0x0000, 0x020000, CRC(e82a1072) SHA1(5175e39ad471f2fa9240364bf893a9f493b44f63) )
	ROM_LOAD16_BYTE(  "sal111a.bin", 0x0000, 0x020000, CRC(baeed013) SHA1(0366f27b47bae80d3f51f66baf24e2b1faad8d57) )
	ROM_LOAD16_BYTE(  "sal111p.bin", 0x0000, 0x020000, CRC(03b4444a) SHA1(fcd77cd44fb1e6c9094296bf21c561acb27f5ea9) )
	ROM_LOAD16_BYTE(  "sal112.bin", 0x0000, 0x020000, CRC(a5cc9b6a) SHA1(facd8fcd8ae61b73d5b42809619bc27d8bd6ef9a) )
	ROM_LOAD16_BYTE(  "salft1p1.bin", 0x0000, 0x020000, CRC(cf0540b1) SHA1(f225195c663e5b081af21e4e212d776a0bb08b7a) )
	ROM_LOAD16_BYTE(  "salft1p2.bin", 0x0000, 0x020000, CRC(e7ce4c21) SHA1(b6571c028710f5797607f871c7c2cb87df749131) )
	ROM_LOAD16_BYTE(  "snakes&ladders-crystal-p1.bin", 0x0000, 0x020000, CRC(4f27b6c7) SHA1(cd457f4a4e7d518d4d19f6860d6e07207afff4c6) )
	ROM_LOAD16_BYTE(  "snakes&ladders-crystal-p2.bin", 0x0000, 0x020000, CRC(3b3f5d04) SHA1(1f190296667aa599af1cc0b001be40a7b212e5d8) )
	ROM_LOAD16_BYTE(  "snla-41.bin", 0x0000, 0x020000, CRC(bb964fe0) SHA1(451dd54d9bb9e066dd650551861f3ee818f81d7c) )
	ROM_LOAD16_BYTE(  "snla-42.bin", 0x0000, 0x020000, CRC(7af3cc94) SHA1(34111cff7dcf61c5d3c14f58456d88e7eda2e0c1) )
	ROM_LOAD16_BYTE(  "snla-4p1.bin", 0x0000, 0x020000, CRC(50081bd8) SHA1(ebbf1d1b4c7ec03667ff617ee83065764d3f89db) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




ROM_START( j6sla )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "s&l31.b8", 0x00000, 0x020000, CRC(e8cadadb) SHA1(e7edfa79e1648658e200ddbbb1d280641e03b93e) )
	ROM_LOAD16_BYTE(  "s&l32.b8", 0x00001, 0x020000, CRC(b2251a43) SHA1(024c82509026561022ebcd3b2654d5beaa838d24) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "s&l3p.b8", 0x0000, 0x020000, CRC(03548ee3) SHA1(f4727303725b005ae04d9e4572a9249c07b705f3) )
	ROM_LOAD16_BYTE(  "sal111.bin", 0x0000, 0x020000, CRC(e82a1072) SHA1(5175e39ad471f2fa9240364bf893a9f493b44f63) )
	ROM_LOAD16_BYTE(  "sal111a.bin", 0x0000, 0x020000, CRC(baeed013) SHA1(0366f27b47bae80d3f51f66baf24e2b1faad8d57) )
	ROM_LOAD16_BYTE(  "sal111p.bin", 0x0000, 0x020000, CRC(03b4444a) SHA1(fcd77cd44fb1e6c9094296bf21c561acb27f5ea9) )
	ROM_LOAD16_BYTE(  "sal112.bin", 0x0000, 0x020000, CRC(a5cc9b6a) SHA1(facd8fcd8ae61b73d5b42809619bc27d8bd6ef9a) )
	ROM_LOAD16_BYTE(  "salft1p1.bin", 0x0000, 0x020000, CRC(cf0540b1) SHA1(f225195c663e5b081af21e4e212d776a0bb08b7a) )
	ROM_LOAD16_BYTE(  "salft1p2.bin", 0x0000, 0x020000, CRC(e7ce4c21) SHA1(b6571c028710f5797607f871c7c2cb87df749131) )
	ROM_LOAD16_BYTE(  "snla-41.bin", 0x0000, 0x020000, CRC(bb964fe0) SHA1(451dd54d9bb9e066dd650551861f3ee818f81d7c) )
	ROM_LOAD16_BYTE(  "snla-42.bin", 0x0000, 0x020000, CRC(7af3cc94) SHA1(34111cff7dcf61c5d3c14f58456d88e7eda2e0c1) )
	ROM_LOAD16_BYTE(  "snla-4p1.bin", 0x0000, 0x020000, CRC(50081bd8) SHA1(ebbf1d1b4c7ec03667ff617ee83065764d3f89db) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "snlsasnd.bin", 0x0000, 0x080000, CRC(288e4ce1) SHA1(6690eccb1af94731af160d73b4d8903a98fa27bb) )
ROM_END




ROM_START( j6thril )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "fpe1.bin", 0x00000, 0x020000, CRC(d2e60e6f) SHA1(525022907858021a2a274a870336de6b414c0b05) )
	ROM_LOAD16_BYTE(  "fpe2.bin", 0x00001, 0x020000, CRC(08700f25) SHA1(aa1b80a7805a80899c3a852866e93fbce9b0ad7c) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "fpep1.bin", 0x0000, 0x020000, CRC(39785a57) SHA1(df748f3c92584c8549d4e60cb61f3deadf9a516e) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "thlr-snd.bin", 0x0000, 0x020000, CRC(03771692) SHA1(2f42859d09e4354689887f1e40f9fce1eb858e3e) ) // probably bad
	ROM_LOAD(  "thrillersnd.bin", 0x0000, 0x080000, CRC(c93bc625) SHA1(ecceddd90a721c39ef3da8cef442c0d78dacaed2) )
ROM_END



ROM_START( j6tqust )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE(  "tq21.b8", 0x00000, 0x020000, CRC(b114c904) SHA1(bf4d2f13525aa9b91bd5985eb62e022751bba596) )
	ROM_LOAD16_BYTE(  "tq22.b8", 0x00001, 0x020000, CRC(aa62d195) SHA1(a9ba505245580444d03c7aa5f6aa3b54cb6b6fbc) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE(  "tq2p.b8", 0x0000, 0x020000, CRC(5a8a9d3c) SHA1(cee5dec688f12ea0c7477c12493324e2f0162a6a) )
	ROM_LOAD16_BYTE(  "trqu-31.bin", 0x0000, 0x020000, CRC(d6fe4bdc) SHA1(ee95e7f1bc5f48691f1f0dfdb57a9ea38c58c557) )
	ROM_LOAD16_BYTE(  "trqu-32.bin", 0x0000, 0x020000, CRC(4fabc6ba) SHA1(eb643359e687c5b33847b782b0d3e5b33c891fca) )
	ROM_LOAD16_BYTE(  "trqu-3p1.bin", 0x0000, 0x020000, CRC(3d601fe4) SHA1(84b3c53165e4e416173861f9bf9f1be1c74888a4) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD(  "tresquestsnd.bin", 0x0000, 0x080000, CRC(c7254d62) SHA1(9b97c7d1a844cd39c3dcfd984e1908ece47db00e) )
ROM_END




ROM_START( j6fbcrz )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "fc.p1", 0x00000, 0x020000, CRC(6493c4c6) SHA1(ae4c18b35a55ef08b02fdabb8021eb008660543e) )
	ROM_LOAD16_BYTE( "fc.p2", 0x00001, 0x020000, CRC(2563c918) SHA1(7b5a58892019d59aff02f982cafd8617f116a812) )

	ROM_REGION( 0x1000000, "encrypted", ROMREGION_ERASE00 )
	// these both seem encrypted with the same scheme, the data becomes the same in the place where the normal
	// program roms have a 0xff fill.  Encryption is atypical of Impact tho, so what's going on here??

	// edit: Apparently the public release of JPeMu v1.45 required the author to specifically encrypt any ROMs
	//       before they would run on the emulator, as a way of 'controlling' the users, and what they could run.
	//       These are therefore worthless garbage.
//  ROM_LOAD16_BYTE( "fce.p1", 0x00000, 0x020000, CRC(57220618) SHA1(7bd717e438e2bf230179b0f5bb358888a3501c59) )0
//  ROM_LOAD16_BYTE( "fce.p2", 0x00001, 0x020000, CRC(16d20bc6) SHA1(0e8ac586ccf3d02189e24bdd2ed88052491aceb6) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	ROM_LOAD( "fcsnd.bin", 0x0000, 0x080000, CRC(53806516) SHA1(e5689759ccba30ac974eee4361330ad503a29909) )
ROM_END

ROM_START( j6h5clb )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8872.bin", 0x00000, 0x020000, CRC(3a3be2e9) SHA1(9078fd612cb5f195f0d9bddc5f04e1389b4f7233) )
	ROM_LOAD16_BYTE( "8873.bin", 0x00001, 0x020000, CRC(6fbe7ca4) SHA1(d7fbd1d83f165f899c5ae8ec3243ec3d8be5563d) )

	ROM_REGION( 0x1000000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8874.bin", 0x0000, 0x020000, CRC(d1a5b6d1) SHA1(54300e459cc3ea756528424f08da5505f629abd4) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( j6milln )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "pljm-f_1.bin", 0x00000, 0x020000, CRC(a4a9c5b8) SHA1(2fa9f4a7ef05352498b91c2b7bcf4d9ca20614a5) )
	ROM_LOAD16_BYTE( "pljm-f_2.bin", 0x00001, 0x020000, CRC(71f303b2) SHA1(7fba05cb6107296cea9b5be575569daf98f2b9c1) )

	ROM_LOAD16_BYTE( "ljc-g1.bin", 0x00000, 0x020000, CRC(518088da) SHA1(d144ab37d17a8ea2174a7ed2dd32b37f3d112dcd) )
	ROM_LOAD16_BYTE( "ljc-g2.bin", 0x00001, 0x020000, CRC(e42c5cc0) SHA1(612973ba3dac2b1098dd746fd4fd6e7bb0246949) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( j6gldpl )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "60000304.p1", 0x00000, 0x020000, CRC(b1a32f63) SHA1(48484bfc7e8dc2937c926ed8c5c4c3f4ebafc908) )
	ROM_LOAD16_BYTE( "60000304.p2", 0x00001, 0x020000, CRC(a7fbc9c7) SHA1(2b2db6a7619011bf02a6b03733387577c0c50cc6) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

ROM_START( j6shoot )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "shoot out p1", 0x00000, 0x020000, CRC(d1797023) SHA1(6a75451b0e5954c87f65f333a5fc73d3ac43d1f4) )
	//ROM_LOAD( "shoot ou", 0x0000, 0x020000, CRC(d1797023) SHA1(6a75451b0e5954c87f65f333a5fc73d3ac43d1f4) )
	ROM_LOAD16_BYTE( "shootp2", 0x00001, 0x020000, CRC(c345c0a9) SHA1(8e9dfb58b3295b2d44bfa810f5502872cf7111d1) )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END




DRIVER_INIT( j6fbcrz )
{
	#if 0

	int i;

	UINT8 *src1 = machine.region( "maincpu" )->base();
	UINT8 *src2 = machine.region( "encrypted" )->base();

	for (i=0;i<0x040000;i++)
	{
		src2[i] = src2[i]^src1[i];
	}

	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"%s", machine.system().name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(src2, 0x040000, 1, fp);
			fclose(fp);
		}
	}
	#endif
}


/************************************
 *
 *  Game driver(s)
 *
 *************************************/

/* Video */

GAME( 1995, cluedo,   0,       jpmimpct, cluedo,   0, ROT0, "JPM", "Cluedo (prod. 2D)",           GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1995, cluedod,  cluedo,  jpmimpct, cluedo,   0, ROT0, "JPM", "Cluedo (prod. 2D) (Protocol)",GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1995, cluedo2c, cluedo,  jpmimpct, cluedo,   0, ROT0, "JPM", "Cluedo (prod. 2C)",           GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1995, cluedo2,  cluedo,  jpmimpct, cluedo,   0, ROT0, "JPM", "Cluedo (prod. 2)",      	 GAME_IMPERFECT_GRAPHICS | GAME_SUPPORTS_SAVE )
GAME( 1996, trivialp, 0,       jpmimpct, trivialp, 0, ROT0, "JPM", "Trivial Pursuit (New Edition) (prod. 1D)",  GAME_SUPPORTS_SAVE )
GAME( 1996, trivialpd,trivialp,jpmimpct, trivialp, 0, ROT0, "JPM", "Trivial Pursuit (New Edition) (prod. 1D) (Protocol)",GAME_SUPPORTS_SAVE )
GAME( 1996, trivialpo,trivialp,jpmimpct, trivialp, 0, ROT0, "JPM", "Trivial Pursuit",  GAME_SUPPORTS_SAVE )
GAME( 1997, scrabble, 0,       jpmimpct, scrabble, 0, ROT0, "JPM", "Scrabble (rev. F)",           GAME_SUPPORTS_SAVE )
GAME( 1997, scrabbled,scrabble,jpmimpct, scrabble, 0, ROT0, "JPM", "Scrabble (rev. F) (Protocol)",GAME_SUPPORTS_SAVE )
GAME( 1998, hngmnjpm, 0,       jpmimpct, hngmnjpm, 0, ROT0, "JPM", "Hangman (JPM)",               GAME_SUPPORTS_SAVE )
GAME( 1998, hngmnjpmd,hngmnjpm,jpmimpct, hngmnjpm, 0, ROT0, "JPM", "Hangman (JPM) (Protocol)",    GAME_SUPPORTS_SAVE )
GAME( 1999, coronatn, 0,       jpmimpct, coronatn, 0, ROT0, "JPM", "Coronation Street Quiz Game", GAME_SUPPORTS_SAVE )
GAME( 1999, coronatnd,coronatn,jpmimpct, coronatn, 0, ROT0, "JPM", "Coronation Street Quiz Game (Protocol)", GAME_SUPPORTS_SAVE )
GAME( 199?, tqst,	  0,	   jpmimpct, cluedo	 , 0, ROT0, "JPM", "Treasure Quest"			   , GAME_NOT_WORKING) // seems to be a video set (different to the games below), but probably incomplete, was marked as 'ACE' ?
GAME( 199?, snlad,	  0,	   jpmimpct, cluedo	 , 0, ROT0, "JPM", "Snake & Ladders"			   , GAME_NOT_WORKING) // probably incomplete

/* Mechanical Below */

GAME( 199?, j6fifth		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "5th Dimension (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6aceclb	, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Ace Of Clubs (Crystal) (IMPACT, set 1)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6aceclba	, j6aceclb	, impctawp, tbirds, 0, ROT0, "Crystal", "Ace Of Clubs (Crystal) (IMPACT, set 2)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6acehi		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Aces High (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6amdrm		, 0			, impctawp, tbirds, 0, ROT0, "Mdm", "American Dream (Mdm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6arcade	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Arcadia (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bnkrcl	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Banker Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6big50		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Big 50 (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bigbnk	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Big Banker (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bigbuk	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Big Bucks (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bigcsh	, 0			, impctawp, tbirds, 0, ROT0, "Empire", "Big Cash Machine (Empire) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bigpct	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Big Picture (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bigtop	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Big Top Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bigwhl	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Big Wheel (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bnza		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Bonanza (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6brkout	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Breakout (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bucks		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Bucks Fizz (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6buzz		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Buzzundrum (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cpclb		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Caesar's Palace Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6camelt	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Camelot (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6scarlt	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Captain Scarlet (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cshbox	, 0			, impctawp, tbirds, 0, ROT0, "Empire", "Cash Box Club (Empire) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cshbeu	, 0			, impctawp, tbirds, 0, ROT0, "Empire", "Cash Box Club (Empire) (Euro) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cshbst	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Cash Buster (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cshcnt	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Cash Countdown (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cshrd		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Cash Raider (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cshtwr	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Cash Towers (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cshvgs	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Cash Vegas Strip (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cas5		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Casino 5ive Liner (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cascz		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Casino Crazy (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cccla		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Casino Crazy Classic (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cascla	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Casino Crazy Classic Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6casclb	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Casino Crazy Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6caslas	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Casino Las Vegas (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cheque	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Cheque Mate (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cluclb	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Cluedo Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6col		, 0			, impctawp, tbirds, 0, ROT0, "Mdm", "Coliseum (Mdm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6colcsh	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Coliseum Cash (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6colmon	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Colour Of Money (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6coprob	, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Cops 'n' Robbers (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6crack		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Cracker (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6crzclb	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Crazy Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6crsfir	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Cross Fire (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6daygld	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Daytona Gold (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6dayml		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Daytona Millennium (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6dmnjkr	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Demon Jokers (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6dmngz		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Diamond Geezer (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6dyfl		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Do You Feel Lucky (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6drdogh	, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Dr Dough (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6euphor	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Euphoria (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6fastfr	, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Fast Fruits Club (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6fasttk	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Fast Trak (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6filth		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Filthy Rich Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6firbl		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Fireball (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6fireck	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Firecracker (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6firclb	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Firecracker Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6fivalv	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Five Alive Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6fiveln	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Five Liner (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6frc10		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Force 10 (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6framft	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Frame & Fortune Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6frtmch	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "The Fruit Machine (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6frtpot	, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Fruitpots (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6gforce	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "G Force (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6gforceb	, j6gforce	, impctawp, tbirds, 0, ROT0, "JPM", "G Force (Jpm) (IMPACT) (15GBP Jackpot)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6gidogh	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "G.I. Dough (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6guab		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Give Us A Break (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6guabcl	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Give Us A Break Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6gldclb	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Gladiator Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6gogold	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Go For Gold (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6gldmin	, 0			, impctawp, tbirds, 0, ROT0, "Empire", "Gold Mine (Empire) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6gldday	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Golden Day (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6golddm	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Golden Demons (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6goldgl	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Golden Goal (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hapyhr	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Happy Hour (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hifly		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Hi Flyer (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6impact	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Hi Impact (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6impactb	, j6impact	, impctawp, tbirds, 0, ROT0, "JPM", "Hi Impact (Jpm) (IMPACT) (15GBP Jackpot)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hilosv	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Hi Lo Silver (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hirol		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Hi Roller (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hirlcl	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Hi Roller Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6histk		, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Hi Stakes (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hiphop	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Hip Hopper (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hotsht	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Hot Shot (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6impuls	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Impulse (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6indy		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Indiana Jones (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6showtm	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "It's Showtime (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6jackjs	, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Jackpot Justice (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6jkrgld	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Jokers Gold (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6jkrpls	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Jokers Plus (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6jkpldx	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Jokers Plus Deluxe (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6jkwld		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Jokers Wild (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6jungfv	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Jungle Fever (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6kamel		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Kameleon (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6kungfu	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Kung Fu (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6luckla	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Lucky Las Vegas (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6magcir	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Magic Circle Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6mavrk		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Maverick (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6maxod		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Maximum Overdrive (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6maxcsh	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Maximus Cash (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6medal		, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Medallion Job (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6megbck	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Mega Bucks (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6monmad	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Money Madness (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6monspd	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Money Spider (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6montlk	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Money Talks (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6mono60	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Monopoly 60th Anniversary Edition (Jpm) (IMPACT, set 1)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6mono60a	, j6mono60	, impctawp, tbirds, 0, ROT0, "JPM", "Monopoly 60th Anniversary Edition (Jpm) (IMPACT, set 2)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6monobn	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Monopoly Bingo (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6outlaw	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Outlaw (Jpm) (IMPACT, v3)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6outlawd	, j6outlaw	, impctawp, tbirds, 0, ROT0, "JPM", "Outlaw (Jpm) (IMPACT, v3) (Protocol)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6outlawc	, j6outlaw	, impctawp, tbirds, 0, ROT0, "JPM", "Outlaw (Jpm) (IMPACT, Club?)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6oxo		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Oxo (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6oxobin	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Oxo Bingo (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pacman	, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Pac Man Plus (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6papa		, 0			, impctawp, tbirds, 0, ROT0, "Empire", "Paparazzi (Empire) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6phxgld	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Phoenix Gold (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pnxgd		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Phoenix Gold De Luxe (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pnxmil	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Phoenix Millennium (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pinwzd	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Pinball Wizard (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pirgld	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Pirates Gold (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6popoli	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Popeye & Olive (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pog		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Pot Of Gold (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pogcls	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Pot Of Gold Classic (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pwrlin	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Power Lines (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pwrspn	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Powerspin (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6quantm	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Quantum Leap (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6quick		, 0			, impctawp, tbirds, 0, ROT0, "RAL", "Quicksilver (RAL) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6rager		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Red Alert (Jpm) [German] (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6ra		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Red Alert (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6raclb		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Red Alert Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6redarw	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Red Arrow (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6redarww	, j6redarw	, impctawp, tbirds, 0, ROT0, "Whitbread / JPM", "Red Arrow (Whitbread / Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6reddmn	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Red Demon (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 2002, j6rh6		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Red Hot 6 (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6rhchil	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Red Hot Chili Stepper (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6rh6cl		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Red Hot Six Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6reelmn	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Reel Money (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6reelth	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Reel Thing (Ace) (IMPACT, set 1)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6reeltha	, j6reelth	, impctawp, tbirds, 0, ROT0, "Ace", "Reel Thing (Ace) (IMPACT, set 2)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6richpk	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Rich Pickings (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6rico		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Ricochet (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6robin		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Robin Hood (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6roller	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Roller Coaster (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6rccls		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Roller Coaster Classic (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6royfls	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Royal Flush Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6samur		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Samurai Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6sidewd	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Sidewinder (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6snakes	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Snakes & Ladders (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6sonic		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Sonic The Hedgehog (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6spcinv	, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Space Invaders (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6stards	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Stardust (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6start		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Starturn (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6strk10	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Strike 10 (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6supbrk	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Super Breakout (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6swpdrp	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Swop Till Ya Drop (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bags		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Three Bags Full (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6roof		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Thru' The Roof (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6tbirds	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Thunderbirds (Jpm) (IMPACT, set 1)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6tbirdsa	, j6tbirds	, impctawp, tbirds, 0, ROT0, "JPM", "Thunderbirds (Jpm) (IMPACT, set 2)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6tbirdsb	, j6tbirds	, impctawp, tbirds, 0, ROT0, "JPM", "Thunderbirds (Jpm) (IMPACT, set 3)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6tbrdcl	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Thunderbirds Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6tomb		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Tomb Raider (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6topflg	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Top Flight (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6tutti		, 0			, impctawp, tbirds, 0, ROT0, "Qps", "Tutti Frutti (Qps) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6twst		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Twister (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6twstd		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Twister (Jpm) [Dutch] (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6untch		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Untouchables (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pompay	, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Up Pompay (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6vindal	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Vindaloot (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6vivark	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Viva Rock Vegas (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6vivrkc	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Viva Rock Vegas Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6wldkng	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Wild King Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6wthing	, 0			, impctawp, tbirds, 0, ROT0, "Empire", "Wild Thing (Empire) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6wildw		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "Wild West (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6wizard	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Wizard Of Odds (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6knight	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Your Lucky Knight (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6svndb		, 0			, impctawp, tbirds, 0, ROT0, "Ace", "7 Deadly Bins (Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6ewn		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Each Way Nudger (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hikar		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Hi Karate (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hisprt	, 0			, impctawp, tbirds, 0, ROT0, "Empire", "High Spirits (Empire) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6rcclub	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Roller Coaster Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6slvgst	, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Silver Ghost (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6footy		, 0			, impctawp, tbirds, 0, ROT0, "Empire", "Football Fever (Empire) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6fbcrz		, 0			, impctawp, tbirds, j6fbcrz, ROT0, "JPM", "Football Crazy (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6h5clb		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "High Five Club (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )




GAME( 199?, j6bbankr	, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Big Banker (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bmc		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Big Money Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6bno		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Big Nite Out (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6btbw		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Born To Be Wild Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cpal		, 0			, impctawp, tbirds, 0, ROT0, "Whitbread / Crystal", "Caesar's Palace (Whitbread / Crystal) (IMPACT, set 1)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cpala		, 0			, impctawp, tbirds, 0, ROT0, "Whitbread / Crystal", "Caesar's Palace (Whitbread / Crystal) (IMPACT, set 2)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND ) // marked as ACE
GAME( 199?, j6cpalb		, 0			, impctawp, tbirds, 0, ROT0, "Whitbread / Ace", "Caesar's Palace (Whitbread / Ace) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6cdivr		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Cash Diver (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6ccc		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Casino Crazy Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6colic		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Coliseum (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6crakr		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Cracker (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6ewndg		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Each Way Nudger (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6easy		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Easy Money (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6ffc		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Frame & Fortune Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6grc		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Gold Rush Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6hdc		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Hot Dogs Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6impls		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Impulse (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6kapang	, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Kapang (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6kfc		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Kung Fu Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6lucklo	, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Lucky Lottery Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6monst		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Monster Cash Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pinfvr	, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Pinball Fever (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6pinwzdc	, j6pinwzd	, impctawp, tbirds, 0, ROT0, "Crystal", "Pinball Wizard (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6potg		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Pot Of Gold (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6ramese	, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Rameses' Riches Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6r2rum		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Ready To Rumble (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6redal		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Red Alert (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6reelb		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Reel Bingo Classic Club (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6sl		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Snakes & Ladders (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6sla		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Snakes & Ladders Slides Again (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6thril		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Thriller (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6tqust		, 0			, impctawp, tbirds, 0, ROT0, "Crystal", "Treasure Quest (Crystal) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )
GAME( 199?, j6gldpl		, 0			, impctawp, tbirds, 0, ROT0, "Qps / Mazooma", "Golden Palace (Qps / Mazooma) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND ) // Mazooma rebuild? only has QPS strings tho
GAME( 199?, j6shoot		, 0			, impctawp, tbirds, 0, ROT0, "JPM / Whitbread", "ShootOut (Jpm / Whitbread) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND ) // Mazooma rebuild? only has QPS strings tho


// was marked as SWP, should this be a video game? - Apparently it's just a link box using the same hardware, but for 3 PC based units which aren't dumped, and probably can't really be emulated :-/
GAME( 199?, j6milln		, 0			, impctawp, tbirds, 0, ROT0, "JPM", "Millionaire (Jpm) (IMPACT)", GAME_NOT_WORKING|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL|GAME_NO_SOUND )










