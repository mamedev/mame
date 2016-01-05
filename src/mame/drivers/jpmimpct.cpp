// license:BSD-3-Clause
// copyright-holders:Philip Bennett
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
        For now, the MACHINE_IMPERFECT_GRAPHICS flag remains.

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

/**************************************************************************

IMPACT Games

IMPACT apparently stands for Interactive Moving Picture Amusement Control
Technology, and is intended as a replacement for the JPM System 5 board.
Large sections of the processing were moved to two identical custom ASICs
(U1 and U2), only half of each is used.

Thanks to Tony Friery and JPeMU for I/O routines and documentation.

***************************************************************************/


#include "emu.h"
#include "cpu/m68000/m68000.h"

#include "includes/jpmimpct.h"
#include "machine/nvram.h"
#include "jpmimpct.lh"
#include "video/awpvid.h"
#include "machine/i8255.h"

/*************************************
 *
 *  MC68681 DUART (TODO)
 *
 *************************************/

#define MC68681_1_CLOCK     3686400
#define MC68681_2_CLOCK     3686400


/*************************************
 *
 *  68000 IRQ handling
 *
 *************************************/

void jpmimpct_state::update_irqs()
{
	m_maincpu->set_input_line(2, m_tms_irq ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(5, m_duart_1_irq ? ASSERT_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  Initialisation
 *
 *************************************/

MACHINE_START_MEMBER(jpmimpct_state,jpmimpct)
{
	save_item(NAME(m_tms_irq));
	save_item(NAME(m_duart_1_irq));
	save_item(NAME(m_touch_cnt));
	save_item(NAME(m_touch_data));

	/* TODO! */
	save_item(NAME(m_duart_1.ISR));
	save_item(NAME(m_duart_1.IMR));
	save_item(NAME(m_duart_1.CT));
}


MACHINE_RESET_MEMBER(jpmimpct_state,jpmimpct)
{
	memset(&m_duart_1, 0, sizeof(m_duart_1));

	/* Reset states */
	m_duart_1_irq = m_tms_irq = 0;
	m_touch_cnt = 0;

//  m_duart_1.IVR=0x0f;
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

TIMER_DEVICE_CALLBACK_MEMBER(jpmimpct_state::duart_1_timer_event)
{
	m_duart_1.tc = 0;
	m_duart_1.ISR |= 0x08;

	m_duart_1_irq = 1;
	update_irqs();
}

READ16_MEMBER(jpmimpct_state::duart_1_r)
{
	struct duart_t &duart_1 = m_duart_1;
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
			val = ioport("TEST/DEMO")->read();
			break;
		}
		case 0xe:
		{
			attotime rate = attotime::from_hz(MC68681_1_CLOCK) * (16 * duart_1.CT);
			timer_device *duart_timer = machine().device<timer_device>("duart_1_timer");
			duart_timer->adjust(rate, 0, rate);
			break;
		}
		case 0xf:
		{
			m_duart_1_irq = 0;
			update_irqs();
			duart_1.ISR |= ~0x8;
			break;
		}
	}

	return val;
}

WRITE16_MEMBER(jpmimpct_state::duart_1_w)
{
	struct duart_t &duart_1 = m_duart_1;
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
			//osd_printf_debug("%c", data);
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
			//osd_printf_debug("%c",data);
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
READ16_MEMBER(jpmimpct_state::duart_2_r)
{
	switch (offset)
	{
		case 0x9:
		{
			if (m_touch_cnt == 0)
			{
				if ( ioport("TOUCH")->read() & 0x1 )
				{
					m_touch_data[0] = 0x2a;
					m_touch_data[1] = 0x7 - (ioport("TOUCH_Y")->read() >> 5) + 0x30;
					m_touch_data[2] = (ioport("TOUCH_X")->read() >> 5) + 0x30;

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
			UINT16 val = m_touch_data[m_touch_cnt];

			if (m_touch_cnt++ == 3)
				m_touch_cnt = 0;

			return val;
		}
		default:
			return 0;
	}
}

/*
    Nothing important here?
*/
WRITE16_MEMBER(jpmimpct_state::duart_2_w)
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

READ16_MEMBER(jpmimpct_state::inputs1_r)
{
	UINT16 val = 0x00ff;

	switch (offset)
	{
		case 0:
		{
			val = ioport("DSW")->read();
			break;
		}
		case 2:
		{
			val = ioport("SW2")->read();
			break;
		}
		case 4:
		{
			val = ioport("SW1")->read();
			break;
		}
		case 9:
		{
			val = ioport("COINS")->read();
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
WRITE16_MEMBER(jpmimpct_state::volume_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_upd7759->set_bank_base(0x20000 * ((data >> 1) & 3));
		m_upd7759->reset_w(data & 0x01);
	}
}

WRITE16_MEMBER(jpmimpct_state::upd7759_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_upd7759->port_w(space, 0, data);
		m_upd7759->start_w(0);
		m_upd7759->start_w(1);
	}
}

READ16_MEMBER(jpmimpct_state::upd7759_r)
{
	if (ACCESSING_BITS_0_7)
	{
		return m_upd7759->busy_r();
	}

	return 0xffff;
}

/*************************************
 *
 *  Mysterious stuff
 *
 *************************************/

READ16_MEMBER(jpmimpct_state::unk_r)
{
	return 0xffff;
}

WRITE16_MEMBER(jpmimpct_state::unk_w)
{
}

void jpmimpct_state::jpm_draw_lamps(int data, int lamp_strobe)
{
	int i;
	for (i=0; i<16; i++)
	{
		m_Lamps[16*(m_lamp_strobe+i)] = data & 1;
		output_set_lamp_value((16*lamp_strobe)+i, (m_Lamps[(16*lamp_strobe)+i]));
		data = data >> 1;
	}
}

READ16_MEMBER(jpmimpct_state::jpmio_r)
{
	return 0xffff;
}

WRITE16_MEMBER(jpmimpct_state::jpmio_w)
{
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
			m_meters->update(0, data >> 10);
			m_duart_1.IP &= ~0x10;
			break;
		}

		case 0x08:
		{
			jpm_draw_lamps(data, m_lamp_strobe);
			break;
		}

		case 0x0b:
		{
			output_set_digit_value(m_lamp_strobe,data);
			break;
		}
		case 0x0f:
		{
			if (data & 0x10)
			{
				m_lamp_strobe = (data +1) & 0x0f;
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
static ADDRESS_MAP_START( m68k_program_map, AS_PROGRAM, 16, jpmimpct_state )
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
	AM_RANGE(0x00480080, 0x00480081) AM_WRITE(upd7759_w)
	AM_RANGE(0x00480082, 0x00480083) AM_WRITE(volume_w)
	AM_RANGE(0x00480084, 0x00480085) AM_READ(upd7759_r)
	AM_RANGE(0x004801e0, 0x004801ff) AM_READWRITE(duart_2_r, duart_2_w)
	AM_RANGE(0x00800000, 0x00800007) AM_DEVREADWRITE("dsp", tms34010_device, host_r, host_w)
	AM_RANGE(0x00c00000, 0x00cfffff) AM_ROM
	AM_RANGE(0x00d00000, 0x00dfffff) AM_ROM
	AM_RANGE(0x00e00000, 0x00efffff) AM_ROM
	AM_RANGE(0x00f00000, 0x00ffffff) AM_ROM
ADDRESS_MAP_END

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/
static ADDRESS_MAP_START( awp68k_program_map, AS_PROGRAM, 16, jpmimpct_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM // most games are 0x00000000 - 0x0003ffff, but some QPS ones go up to fffff, check for any mirroring etc.
	AM_RANGE(0x00400000, 0x00403fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x00480000, 0x0048001f) AM_READWRITE(duart_1_r, duart_1_w)
	AM_RANGE(0x00480020, 0x00480033) AM_READ(inputs1awp_r)
	AM_RANGE(0x00480034, 0x00480035) AM_READ(ump_r)
	AM_RANGE(0x00480040, 0x00480041) AM_READ(optos_r)
	AM_RANGE(0x00480060, 0x00480067) AM_DEVREADWRITE8("ppi8255", i8255_device, read, write,0x00ff)
	AM_RANGE(0x00480080, 0x00480081) AM_WRITE(upd7759_w)
	AM_RANGE(0x00480082, 0x00480083) AM_WRITE(volume_w)
	AM_RANGE(0x00480084, 0x00480085) AM_READ(upd7759_r)
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


/*************************************
 *
 *  Video CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( tms_program_map, AS_PROGRAM, 16, jpmimpct_state )
	AM_RANGE(0xc0000000, 0xc00001ff) AM_DEVREADWRITE("dsp", tms34010_device, io_register_r, io_register_w)
	AM_RANGE(0x00000000, 0x003fffff) AM_MIRROR(0xf8000000) AM_RAM AM_SHARE("vram")
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
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 1")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW 3")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW 4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 5")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW 6")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW 7")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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

WRITE_LINE_MEMBER(jpmimpct_state::tms_irq)
{
	m_tms_irq = state;
	update_irqs();
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( jpmimpct, jpmimpct_state )
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(m68k_program_map)

	MCFG_CPU_ADD("dsp", TMS34010, 40000000)
	MCFG_CPU_PROGRAM_MAP(tms_program_map)
	MCFG_TMS340X0_HALT_ON_RESET(TRUE) /* halt on reset */
	MCFG_TMS340X0_PIXEL_CLOCK(40000000/16) /* pixel clock */
	MCFG_TMS340X0_PIXELS_PER_CLOCK(4) /* pixels per clock */
	MCFG_TMS340X0_SCANLINE_RGB32_CB(jpmimpct_state, scanline_update)   /* scanline updater (rgb32) */
	MCFG_TMS340X0_OUTPUT_INT_CB(WRITELINE(jpmimpct_state, tms_irq))
	MCFG_TMS340X0_TO_SHIFTREG_CB(jpmimpct_state, to_shiftreg)       /* write to shiftreg function */
	MCFG_TMS340X0_FROM_SHIFTREG_CB(jpmimpct_state, from_shiftreg)      /* read from shiftreg function */

	MCFG_QUANTUM_TIME(attotime::from_hz(30000))
	MCFG_MACHINE_START_OVERRIDE(jpmimpct_state,jpmimpct)
	MCFG_MACHINE_RESET_OVERRIDE(jpmimpct_state,jpmimpct)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_TIMER_DRIVER_ADD("duart_1_timer", jpmimpct_state, duart_1_timer_event)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(40000000/4, 156*4, 0, 100*4, 328, 0, 300)
	MCFG_SCREEN_UPDATE_DEVICE("dsp", tms34010_device, tms340x0_rgb32)
	MCFG_PALETTE_ADD("palette", 256)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_VIDEO_START_OVERRIDE(jpmimpct_state,jpmimpct)
	
	MCFG_DEVICE_ADD("meters", METERS, 0)
	MCFG_METERS_NUMBER(5)
MACHINE_CONFIG_END




/*************************************
 *
 *  Initialisation
 *
 *************************************/

READ8_MEMBER(jpmimpct_state::hopper_b_r)
{
	int retval;
	// B0 = 100p Hopper Out Verif
	// B1 = Hopper High
	// B2 = Hopper Low
	// B3 = 20p Hopper Opto

	// Always return hoppers full
	retval=0xed; // 1110 1101

	if (!m_hopinhibit)//if inhibited, we don't change these flags
	{
		if (m_hopper[0] && m_motor[0]) //&& ((m_hopflag1 & 0x20)==0x20))
		{//100p
			retval &= ~0x01;
		}
		if (((m_hopper[1] && m_motor[1]) || (m_hopper[2] && m_slidesout))) //&& ((m_hopflag2 & 0x20)==0x20))
		{
			retval &= ~0x08;
		}
	}

	return retval;
}

READ8_MEMBER(jpmimpct_state::hopper_c_r)
{
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
	if (m_hopper[0])
	{
		retval &= ~0x40;
	}
	if (m_hopper[1])
	{
		retval &= ~0x10;
	}

	if (!m_hopinhibit)
	{
		if ((m_slidesout==1) && ((m_hopper[2]==0)))
		{
			m_slidesout=0;
			retval &= ~0x80;
		}
	}

	return retval;
}

WRITE8_MEMBER(jpmimpct_state::payen_a_w)
{
	m_motor[0] = (data & 0x01);
	m_payen = (data & 0x10);
	m_slidesout = (data & 0x10);
	m_motor[1] = (data & 0x40);
	m_hopinhibit = (data & 0x80);
}

WRITE8_MEMBER(jpmimpct_state::display_c_w)
{
	//Reset 0x04, data 0x02, clock 0x01
	m_vfd->por(data & 0x04);
	m_vfd->data(data & 0x02);
	m_vfd->sclk(data & 0x01);
}

MACHINE_START_MEMBER(jpmimpct_state,impctawp)
{
	save_item(NAME(m_duart_1_irq));
	save_item(NAME(m_touch_cnt));
	save_item(NAME(m_touch_data));

	/* TODO! */
	save_item(NAME(m_duart_1.ISR));
	save_item(NAME(m_duart_1.IMR));
	save_item(NAME(m_duart_1.CT));
}

MACHINE_RESET_MEMBER(jpmimpct_state,impctawp)
{
	memset(&m_duart_1, 0, sizeof(m_duart_1));

	/* Reset states */
	m_duart_1_irq = 0;
	m_vfd->reset();
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
READ16_MEMBER(jpmimpct_state::inputs1awp_r)
{
	UINT16 val = 0x00;

	{
		switch (offset)
		{
			case 0:
			{
				val = ioport("DSW")->read();
				break;
			}
			case 1:
			{
				val = ioport("PERCENT")->read();
				break;
			}
			case 2:
			{
				val = ioport("KEYS")->read();
				break;
			}
			case 3:
			{
				val = ioport("SW2")->read();
				break;
			}
			case 4:
			{
				val = ioport("SW1")->read();
				break;
			}
			case 5:
			{
				val = (ioport("SW3")->read() );
				break;
			}
			case 6:
			{
				val = (ioport("SW4")->read() );
				break;
			}
			case 7://5
			{
				val = (ioport("SW5")->read() );
				break;
			}
			case 9:
			{
				val = ioport("COINS")->read();
				break;
			}
		}
	return val & 0xff00;
	}
}

READ16_MEMBER(jpmimpct_state::optos_r)
{
	return m_optic_pattern;
}

READ16_MEMBER(jpmimpct_state::prot_1_r)
{
	return 0x01;
}

READ16_MEMBER(jpmimpct_state::prot_0_r)
{
	return 0x00;
}

WRITE16_MEMBER(jpmimpct_state::jpmioawp_w)
{
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
			m_reel0->update((data >> 0)& 0x0F);
			m_reel1->update((data >> 1)& 0x0F);
			m_reel2->update((data >> 2)& 0x0F);
			m_reel3->update((data >> 3)& 0x0F);
			awp_draw_reel("reel1", m_reel0);
			awp_draw_reel("reel2", m_reel1);
			awp_draw_reel("reel3", m_reel2);
			awp_draw_reel("reel4", m_reel3);
			break;
		}
		case 0x04:
		{
			m_reel4->update((data >> 4)& 0x0F);
			m_reel5->update((data >> 5)& 0x0F);
			awp_draw_reel("reel5", m_reel4);
			awp_draw_reel("reel6", m_reel5);
			break;
		}
		case 0x06:
		{
			//Slides
			if ((data & 0xff)!=0x00)
			{
				m_slidesout=2;
			}
			if (((data & 0xff)==0x00) && (m_slidesout==2))
			{
				m_slidesout=1;
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
							m_meters->update(i, 0);
						}
						break;
					}
					default:
					{
						m_meters->update(((metno <<2) - 1), 1);
					}
					break;
				}
			}
			int combined_meter = m_meters->GetActivity(0) | m_meters->GetActivity(1) |
			m_meters->GetActivity(2) | m_meters->GetActivity(3) |
			m_meters->GetActivity(4);

			if(combined_meter)
			{
				m_duart_1.IP &= ~0x10;
			}
			else
			{
				m_duart_1.IP |= 0x10;
			}
			break;
		}

		case 0x08:
		{
			jpm_draw_lamps(data, m_lamp_strobe);
			break;
		}

		case 0x0b:
		{
			output_set_digit_value(m_lamp_strobe,data);
			break;
		}
		case 0x0f:
		{
			if (data & 0x10)
			{
				m_lamp_strobe = (data & 0x0f);
			}
			break;
		}
	}
}

READ16_MEMBER(jpmimpct_state::ump_r)
{
	return 0xff;//0xffff;
}



INPUT_PORTS_START( tbirds )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW 0 (toggle to stop alarm)")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW 1")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW 2")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW 3")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW 4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW 5")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW 6")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW 7")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

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

MACHINE_CONFIG_START( impctawp, jpmimpct_state )
	MCFG_CPU_ADD("maincpu",M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(awp68k_program_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(30000))
	MCFG_S16LF01_ADD("vfd",0)

	MCFG_MACHINE_START_OVERRIDE(jpmimpct_state,impctawp)
	MCFG_MACHINE_RESET_OVERRIDE(jpmimpct_state,impctawp)
	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("ppi8255", I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(jpmimpct_state, payen_a_w))
	MCFG_I8255_IN_PORTB_CB(READ8(jpmimpct_state, hopper_b_r))
	MCFG_I8255_IN_PORTC_CB(READ8(jpmimpct_state, hopper_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(jpmimpct_state, display_c_w))

	MCFG_TIMER_DRIVER_ADD("duart_1_timer", jpmimpct_state, duart_1_timer_event)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("upd",UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_DEFAULT_LAYOUT(layout_jpmimpct)

	MCFG_STARPOINT_48STEP_ADD("reel0")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(jpmimpct_state, reel0_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel1")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(jpmimpct_state, reel1_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel2")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(jpmimpct_state, reel2_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel3")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(jpmimpct_state, reel3_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel4")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(jpmimpct_state, reel4_optic_cb))
	MCFG_STARPOINT_48STEP_ADD("reel5")
	MCFG_STEPPER_OPTIC_CALLBACK(WRITELINE(jpmimpct_state, reel5_optic_cb))
	
	MCFG_DEVICE_ADD("meters", METERS, 0)
	MCFG_METERS_NUMBER(5)

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
	ROM_COPY( "user1", 0x00000, 0x100000, 0x100000 )

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
	ROM_COPY( "user1", 0x00000, 0x100000, 0x100000 )

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
	ROM_COPY( "user1", 0x00000, 0x100000, 0x100000 )

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
	ROM_COPY( "user1", 0x00000, 0x100000, 0x100000 )

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


ROM_START( buzzundr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "prom1n.bin", 0x000000, 0x080000, CRC(2b47efd8) SHA1(bc96a5ea2511081f73a120e025249018c517c638) )
	ROM_LOAD16_BYTE( "prom2.bin",  0x000001, 0x080000, CRC(3a1c38a3) SHA1(cb85e1a9535ba646724db5e3dfbdb81384ada918) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gr1.bin", 0x000000, 0x100000, NO_DUMP )
	ROM_LOAD16_BYTE( "gr2.bin", 0x000001, 0x100000, NO_DUMP )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END


ROM_START( monspdr )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "msp10dsk.1", 0x000000, 0x080000, CRC(892aa085) SHA1(cfb8d4edbf22a88906b3b1fa52156be201d81b44) )
	ROM_LOAD16_BYTE( "msp10.2",    0x000001, 0x080000, CRC(3db5e13e) SHA1(79eb1f17a8e1b3220cd7c5f46212b8a2e1a112cb) )

	ROM_REGION16_LE( 0x200000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gr1.bin", 0x000000, 0x100000, NO_DUMP )
	ROM_LOAD16_BYTE( "gr2.bin", 0x000001, 0x100000, NO_DUMP )

	ROM_REGION( 0x80000, "upd", ROMREGION_ERASE00 )
	/* missing? */
ROM_END

/************************************
 *
 *  Game driver(s)
 *
 *************************************/

/* Video */

GAME( 1995, cluedo,   0,       jpmimpct, cluedo, driver_device,   0, ROT0, "JPM", "Cluedo (prod. 2D)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, cluedod,  cluedo,  jpmimpct, cluedo, driver_device,   0, ROT0, "JPM", "Cluedo (prod. 2D) (Protocol)",MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, cluedo2c, cluedo,  jpmimpct, cluedo, driver_device,   0, ROT0, "JPM", "Cluedo (prod. 2C)",           MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1995, cluedo2,  cluedo,  jpmimpct, cluedo, driver_device,   0, ROT0, "JPM", "Cluedo (prod. 2)",        MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1996, trivialp, 0,       jpmimpct, trivialp, driver_device, 0, ROT0, "JPM", "Trivial Pursuit (New Edition) (prod. 1D)",  MACHINE_SUPPORTS_SAVE )
GAME( 1996, trivialpd,trivialp,jpmimpct, trivialp, driver_device, 0, ROT0, "JPM", "Trivial Pursuit (New Edition) (prod. 1D) (Protocol)",MACHINE_SUPPORTS_SAVE )
GAME( 1996, trivialpo,trivialp,jpmimpct, trivialp, driver_device, 0, ROT0, "JPM", "Trivial Pursuit",  MACHINE_SUPPORTS_SAVE )
GAME( 1997, scrabble, 0,       jpmimpct, scrabble, driver_device, 0, ROT0, "JPM", "Scrabble (rev. F)",           MACHINE_SUPPORTS_SAVE )
GAME( 1997, scrabbled,scrabble,jpmimpct, scrabble, driver_device, 0, ROT0, "JPM", "Scrabble (rev. F) (Protocol)",MACHINE_SUPPORTS_SAVE )
GAME( 1998, hngmnjpm, 0,       jpmimpct, hngmnjpm, driver_device, 0, ROT0, "JPM", "Hangman (JPM)",               MACHINE_SUPPORTS_SAVE )
GAME( 1998, hngmnjpmd,hngmnjpm,jpmimpct, hngmnjpm, driver_device, 0, ROT0, "JPM", "Hangman (JPM) (Protocol)",    MACHINE_SUPPORTS_SAVE )
GAME( 1999, coronatn, 0,       jpmimpct, coronatn, driver_device, 0, ROT0, "JPM", "Coronation Street Quiz Game", MACHINE_SUPPORTS_SAVE )
GAME( 1999, coronatnd,coronatn,jpmimpct, coronatn, driver_device, 0, ROT0, "JPM", "Coronation Street Quiz Game (Protocol)", MACHINE_SUPPORTS_SAVE )
GAME( 199?, tqst,     0,       jpmimpct, cluedo  , driver_device, 0, ROT0, "JPM", "Treasure Quest"             , MACHINE_NOT_WORKING) // incomplete (ACE?)
GAME( 199?, snlad,    0,       jpmimpct, cluedo  , driver_device, 0, ROT0, "JPM", "Snake & Ladders"            , MACHINE_NOT_WORKING) // incomplete
GAME( 199?, buzzundr, 0,       jpmimpct, cluedo  , driver_device, 0, ROT0, "Ace", "Buzzundrum (Ace)", MACHINE_NOT_WORKING )
GAME( 199?, monspdr , 0,       jpmimpct, cluedo  , driver_device, 0, ROT0, "Ace", "Money Spider (Ace)", MACHINE_NOT_WORKING )
