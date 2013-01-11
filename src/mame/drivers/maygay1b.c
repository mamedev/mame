/*****************************************************************************************

    Maygay M1 A/B driver, (under heavy construction !!!)

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team,
    used under license from http://mamedev.org

    This only loads the basic stuff - there needs to be more done to make this run.

    The sound ROM + OKIM6376 is on the game plug-in board, so not all games have it
    (although in some cases it is just missing)


    Gladiators
    ----------

    Produttore
        MayGay

    N.revisione
        M1A

    CPU
        on main board:
            1x TMP82C79P-2
            1x S22EA-EF68B21P
            1x EP840034.A-P-80C51AVW
            1x MC68681P
            1x S22EB-EF68B09P
            1x YM2149F
            2x oscillator 8.000MHz
            1x oscillator 12.000MHz

        on piggyback (SA5-029D):
            1x OKIM6376

    ROMs
        on main board:
            1x GAL16V8

        on piggyback (SA5-029D):
            1x AM27C512
            2x M27C4001
            1x GAL16V8

    Note
        on main board:
            1x 26 pins dual line connector (serial pot?)
            1x 2 legs connector (speaker)
            2x 15 legs connector (coin mech, switch matrix)
            3x 10 legs connector (meters, reel index, triacs)
            1x 11 legs connector (spare stepper motors)
            1x 20 legs connector (stepper motors)
            1x 19 legs connector (aux display)
            1x 9 legs connector (lamps)
            1x 17 legs connector (P3)
            1x 24 legs connector (lamps)
            1x 14 legs connector (power supply)
            1x 8 legs connector (control port)
            1x trimmer (volume)
            1x battery (2.4V 100mAh)
            9x red leds
            1x pushbutton
            2x 8 switches dip

        on piggyback (SA5-029D):
            1x 5 legs connector
            3x trimmer


******************************************************************************************/
#include "emu.h"
#include "includes/maygay1b.h"

#include "maygay1b.lh"


void maygay1b_state::m1_draw_lamps(int data,int strobe, int col)
{
	int i;

	for ( i = 0; i < 8; i++ )
	{
		m_lamppos = (strobe*8) + col + i;

		if ((data>>i)&1)
			m_Lamps[m_lamppos] = 1;
		else
			m_Lamps[m_lamppos] = 0;

		output_set_lamp_value(m_lamppos, m_Lamps[m_lamppos]);
	}
}


/*************************************
 *
 *  8279 display/keyboard driver
 *
 *************************************/

static void update_outputs(i8279_state *chip, UINT16 which)
{
	static const UINT8 ls48_map[16] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };
	int i;

	/* update the items in the bitmask */
	for (i = 0; i < 16; i++)
		if (which & (1 << i))
		{
			int val;

			val = chip->ram[i] & 0x0f;
			if (chip->inhibit & 0x01)
				val = chip->clear & 0x0f;
			output_set_digit_value(i * 2 + 0, ls48_map[val]);

			val = chip->ram[i] >> 4;
			if (chip->inhibit & 0x02)
				val = chip->clear >> 4;
			output_set_digit_value(i * 2 + 1, ls48_map[val]);
		}
}

READ8_MEMBER(maygay1b_state::m1_8279_r)
{
	i8279_state *chip = m_i8279 + 0;
	static const char *const portnames[] = { "SW1","STROBE5","STROBE7","STROBE3","SW2","STROBE4","STROBE6","STROBE2" };
	UINT8 result = 0xff;
	UINT8 addr;

	/* read data */
	if ((offset & 1) == 0)
	{
		switch (chip->command & 0xe0)
		{
			/* read sensor RAM */
			case 0x40:
				addr = chip->command & 0x07;
				result = ioport("SW1")->read();
				/* handle autoincrement */
				if (chip->command & 0x10)
					chip->command = (chip->command & 0xf0) | ((addr + 1) & 0x0f);

				break;


			/* read display RAM */
			case 0x60:

				/* set the value of the corresponding outputs */
				addr = chip->command & 0x0f;
				result = chip->ram[addr];

				/* handle autoincrement */
				if (chip->command & 0x10)
					chip->command = (chip->command & 0xf0) | ((addr + 1) & 0x0f);
				break;
		}
	}

	/* read status word */
	else
	{
		if ( chip->read_sensor )
		{
			result = ioport(portnames[chip->sense_address])->read();
//          break
		}
		if ( chip->sense_auto_inc )
		{
			chip->sense_address = (chip->sense_address + 1 ) & 7;
		}
		else
		{
			result = chip->ram[chip->disp_address];
			if ( chip->disp_auto_inc )
			chip->disp_address++;
		}
	}
	return result;
}

WRITE8_MEMBER(maygay1b_state::m1_8279_w)
{
	i8279_state *chip = m_i8279 + 0;
	UINT8 addr;

	/* write data */
	if ((offset & 1) == 0)
	{
		switch (chip->command & 0xe0)
		{
			/* write display RAM */
			case 0x80:

				/* set the value of the corresponding outputs */
				addr = chip->command & 0x0f;
				if (!(chip->inhibit & 0x04))
					chip->ram[addr] = (chip->ram[addr] & 0xf0) | (data & 0x0f);
				if (!(chip->inhibit & 0x08))
					chip->ram[addr] = (chip->ram[addr] & 0x0f) | (data & 0xf0);
				update_outputs(chip, 1 << addr);

				/* handle autoincrement */
				if (chip->command & 0x10)
					chip->command = (chip->command & 0xf0) | ((addr + 1) & 0x0f);
				break;
		}
	}

	/* write command */
	else
	{
		chip->command = data;

		switch (data & 0xe0)
		{
			/* command 0: set mode */
			/*
			    Display modes:

			    00 = 8 x 8-bit character display -- left entry
			    01 = 16 x 8-bit character display -- left entry
			    10 = 8 x 8-bit character display -- right entry
			    11 = 16 x 8-bit character display -- right entry

			    Keyboard modes:

			    000 = Encoded scan keyboard -- 2 key lockout
			    001 = Decoded scan keyboard -- 2 key lockout
			    010 = Encoded scan keyboard -- N-key rollover
			    011 = Decoded scan keyboard -- N-key rollover
			    100 = Encoded scan sensor matrix
			    101 = Decoded scan sensor matrix
			    110 = Strobed input, encoded display scan
			    111 = Strobed input, decoded display scan
			*/
			case 0x00:
				logerror("8279A: display mode = %d, keyboard mode = %d\n", (data >> 3) & 3, data & 7);
				chip->mode = data & 0x1f;
				break;

			/* command 1: program clock */
			case 0x20:
				logerror("8279A: clock prescaler set to %02X\n", data & 0x1f);
				chip->prescale = data & 0x1f;
				break;

			/* command 2: read FIFO/sensor RAM */
			case 0x40:
				chip->sense_address = data & 0x07;
				chip->sense_auto_inc = data & 0x10;
				chip->read_sensor = 1;
				break;
			/* command 3: read display RAM */
			case 0x60:
				chip->disp_address = data & 0x0f;
				chip->disp_auto_inc = data & 0x10;
				chip->read_sensor = 0;
				break;
			/* command 4: write display RAM */
			case 0x80:
				chip->disp_address = data & 0x0f;
				chip->disp_auto_inc = data & 0x10;
				chip->write_display = 1;
				break;

			/* command 5: display write inhibit/blanking */
			case 0xa0:
				chip->inhibit = data & 0x0f;
				update_outputs(chip, 0);
				logerror("8279: clock prescaler set to %02X\n", data & 0x1f);
				break;

				break;

			/* command 6: clear */
			case 0xc0:
				chip->clear = (data & 0x08) ? ((data & 0x04) ? 0xff : 0x20) : 0x00;
				if (data & 0x11)
					memset(chip->ram, chip->clear, sizeof(chip->ram));
				break;

			/* command 7: end interrupt/error mode set */
			case 0xe0:
				break;
		}
	}
	if ( chip->write_display )
	{  // Data
		if ( chip->ram[chip->disp_address] != data )
		{
			m1_draw_lamps(chip->ram[chip->disp_address],chip->disp_address, 0);
		}
		chip->ram[chip->disp_address] = data;

		if ( chip->disp_auto_inc )
			chip->disp_address ++;
	}
}

READ8_MEMBER(maygay1b_state::m1_8279_2_r)
{
	i8279_state *chip = m_i8279 + 1;
	UINT8 result = 0xff;
	UINT8 addr;

	/* read data */
	if ((offset & 1) == 0)
	{
		switch (chip->command & 0xe0)
		{
			/* read sensor RAM */
			case 0x40:
				//result = ~ioport("DSW1")->read();  /* DSW 1 - inverted! */
				break;

			/* read display RAM */
			case 0x60:

				/* set the value of the corresponding outputs */
				addr = chip->command & 0x0f;
				result = chip->ram[addr];

				/* handle autoincrement */
				if (chip->command & 0x10)
					chip->command = (chip->command & 0xf0) | ((addr + 1) & 0x0f);
				break;
		}
	}

	/* read status word */
	else
	{
		logerror("read 0xfc%02x\n", offset);
		result = 0x10;
	}
	return result;
}


WRITE8_MEMBER(maygay1b_state::m1_8279_2_w)
{
	i8279_state *chip = m_i8279 + 1;
	UINT8 addr;

	/* write data */
	if ((offset & 1) == 0)
	{
		switch (chip->command & 0xe0)
		{
			/* write display RAM */
			case 0x80:

				/* set the value of the corresponding outputs */
				addr = chip->command & 0x0f;
				if (!(chip->inhibit & 0x04))
					chip->ram[addr] = (chip->ram[addr] & 0xf0) | (data & 0x0f);
				if (!(chip->inhibit & 0x08))
					chip->ram[addr] = (chip->ram[addr] & 0x0f) | (data & 0xf0);
				update_outputs(chip, 1 << addr);

				/* handle autoincrement */
				if (chip->command & 0x10)
					chip->command = (chip->command & 0xf0) | ((addr + 1) & 0x0f);
				break;
		}
	}

	/* write command */
	else
	{
		chip->command = data;

		switch (data & 0xe0)
		{
			/* command 0: set mode */
			/*
			    Display modes:

			    00 = 8 x 8-bit character display -- left entry
			    01 = 16 x 8-bit character display -- left entry
			    10 = 8 x 8-bit character display -- right entry
			    11 = 16 x 8-bit character display -- right entry

			    Keyboard modes:

			    000 = Encoded scan keyboard -- 2 key lockout
			    001 = Decoded scan keyboard -- 2 key lockout
			    010 = Encoded scan keyboard -- N-key rollover
			    011 = Decoded scan keyboard -- N-key rollover
			    100 = Encoded scan sensor matrix
			    101 = Decoded scan sensor matrix
			    110 = Strobed input, encoded display scan
			    111 = Strobed input, decoded display scan
			*/
			case 0x00:
				logerror("8279A: display mode = %d, keyboard mode = %d\n", (data >> 3) & 3, data & 7);
				chip->mode = data & 0x1f;
				break;

			/* command 1: program clock */
			case 0x20:
				logerror("8279A: clock prescaler set to %02X\n", data & 0x1f);
				chip->prescale = data & 0x1f;
				break;

			/* command 2: read FIFO/sensor RAM */
			case 0x40:
				chip->sense_address = data & 0x07;
				chip->sense_auto_inc = data & 0x10;
				chip->read_sensor = 1;
				break;
			/* command 3: read display RAM */
			case 0x60:
				chip->disp_address = data & 0x0f;
				chip->disp_auto_inc = data & 0x10;
				chip->read_sensor = 0;
				break;
			/* command 4: write display RAM */
			case 0x80:
				chip->disp_address = data & 0x0f;
				chip->disp_auto_inc = data & 0x10;
				chip->write_display = 1;
				break;

			/* command 5: display write inhibit/blanking */
			case 0xa0:
				break;

			/* command 6: clear */
			case 0xc0:
				break;

			/* command 7: end interrupt/error mode set */
			case 0xe0:
				break;
		}
	}
	if ( chip->write_display )
	{  // Data
		if ( chip->ram[chip->disp_address] != data )
		{
			m1_draw_lamps(chip->ram[chip->disp_address],chip->disp_address, 128);
		}
		chip->ram[chip->disp_address] = data;
		if ( chip->disp_auto_inc )
			chip->disp_address ++;
	}

}

///////////////////////////////////////////////////////////////////////////
// called if board is reset ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

static void m1_stepper_reset(running_machine &machine)
{
	maygay1b_state *state = machine.driver_data<maygay1b_state>();
	int pattern = 0,i;
	for ( i = 0; i < 6; i++)
	{
		stepper_reset_position(i);
		if ( stepper_optic_state(i) ) pattern |= 1<<i;
	}
	state->m_optic_pattern = pattern;
}

void maygay1b_state::machine_reset()
{
	m_vfd->reset(); // reset display1
	m_duart68681 = machine().device( "duart68681" );
	m1_stepper_reset(machine());
}

///////////////////////////////////////////////////////////////////////////

// IRQ from Duart (hopper?)
static void duart_irq_handler(device_t *device, int state, UINT8 vector)
{
	device->machine().device("maincpu")->execute().set_input_line(M6809_IRQ_LINE,  state?ASSERT_LINE:CLEAR_LINE);
	LOG(("6809 irq%d \n",state));
}

// FIRQ, related to the sample playback?
READ8_MEMBER( maygay1b_state::m1_firq_trg_r )
{
	static int i = 0xff;
	i ^= 0xff;
	space.machine().device("maincpu")->execute().set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	LOG(("6809 firq\n"));
	return i;
}

// NMI is periodic? or triggered by a write?
TIMER_DEVICE_CALLBACK_MEMBER( maygay1b_state::maygay1b_nmitimer_callback )
{
	if (m_NMIENABLE)
	{
		LOG(("6809 nmi\n"));
		machine().device("maincpu")->execute().set_input_line(INPUT_LINE_NMI, HOLD_LINE);
	}
}



/***************************************************************************
    6821 PIA
***************************************************************************/

// some games might differ..
WRITE8_MEMBER(maygay1b_state::m1_pia_porta_w)
{
//  printf("m1_pia_porta_w %02x\n",data);

	if((data & 0x40))
	{
		if (m_alpha_clock != (data & 0x20))
		{
			if (!m_alpha_clock)
			{
				m_vfd->shift_data((data & 0x10)?0:1);
			}
		}
		m_alpha_clock = (data & 0x20);
	}
	else
	{
		m_vfd->reset();
	}
}

WRITE8_MEMBER(maygay1b_state::m1_pia_portb_w)
{
	int i;
	for (i=0; i<8; i++)
		if ( data & (1 << i) )      output_set_indexed_value("triac", i, data & (1 << i));
}

static const pia6821_interface m1_pia_intf =
{
	DEVCB_NULL,     /* port A in */
	DEVCB_NULL,     /* port B in */
	DEVCB_NULL,     /* line CA1 in */
	DEVCB_NULL,     /* line CB1 in */
	DEVCB_NULL,     /* line CA2 in */
	DEVCB_NULL,     /* line CB2 in */
	DEVCB_DRIVER_MEMBER(maygay1b_state,m1_pia_porta_w),     /* port A out */
	DEVCB_DRIVER_MEMBER(maygay1b_state,m1_pia_portb_w),     /* port B out */
	DEVCB_NULL,     /* line CA2 out */
	DEVCB_NULL,     /* port CB2 out */
	DEVCB_NULL,     /* IRQA */
	DEVCB_NULL      /* IRQB */
};

// input ports for M1 board ////////////////////////////////////////

INPUT_PORTS_START( maygay_m1 )
	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "SW101" ) PORT_DIPLOCATION("SW1:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "SW102" ) PORT_DIPLOCATION("SW1:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "SW103" ) PORT_DIPLOCATION("SW1:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "SW104" ) PORT_DIPLOCATION("SW1:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "SW105" ) PORT_DIPLOCATION("SW1:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "SW106" ) PORT_DIPLOCATION("SW1:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "SW107" ) PORT_DIPLOCATION("SW1:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "AntiFraud Protection" ) PORT_DIPLOCATION("SW1:08")
	PORT_DIPSETTING(    0x80, DEF_STR( Off  ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x00, "SW201" ) PORT_DIPLOCATION("SW2:01")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On  ) )
	PORT_DIPNAME( 0x02, 0x00, "SW202" ) PORT_DIPLOCATION("SW2:02")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On  ) )
	PORT_DIPNAME( 0x04, 0x00, "SW203" ) PORT_DIPLOCATION("SW2:03")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On  ) )
	PORT_DIPNAME( 0x08, 0x00, "SW204" ) PORT_DIPLOCATION("SW2:04")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On  ) )
	PORT_DIPNAME( 0x10, 0x00, "SW205" ) PORT_DIPLOCATION("SW2:05")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On  ) )
	PORT_DIPNAME( 0x20, 0x00, "SW206" ) PORT_DIPLOCATION("SW2:06")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On  ) )
	PORT_DIPNAME( 0x40, 0x00, "SW207" ) PORT_DIPLOCATION("SW2:07")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On  ) )
	PORT_DIPNAME( 0x80, 0x00, "SW208" ) PORT_DIPLOCATION("SW2:08")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On  ) )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("17")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("18")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("19")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("20")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("24")

	PORT_START("STROBE3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("25")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Hi")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Lo")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("28")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("29")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("30")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Rear Door") PORT_TOGGLE
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_INTERLOCK) PORT_NAME("Cashbox Door")  PORT_CODE(KEYCODE_Q) PORT_TOGGLE

	PORT_START("STROBE4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_SPECIAL)//50p Tube
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_SPECIAL)//100p Tube rear
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SPECIAL)//100p Tube front
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("STROBE5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("49")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("50")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Cancel")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Hold 1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Hold 2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_NAME("Hold 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("Hold 4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_START1)

	PORT_START("STROBE6")
	PORT_SERVICE_NO_TOGGLE(0x01,IP_ACTIVE_HIGH)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("58")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("59")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("60")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("61")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("62")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("63")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("64")

	PORT_START("STROBE7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("65")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("66")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("67")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("68")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("69")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("70")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("72")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("73")

INPUT_PORTS_END

void maygay1b_state::machine_start()
{
	int i;

// setup 8 mechanical meters ////////////////////////////////////////////
	MechMtr_config(machine(),8);

// setup 6 default 96 half step reels ///////////////////////////////////
	for ( i = 0; i < 6; i++ )
	{
		stepper_config(machine(), i, &starpoint_interface_48step);
	}

}
WRITE8_MEMBER(maygay1b_state::reel12_w)
{
	stepper_update(0, data & 0x0F );
	stepper_update(1, (data>>4) & 0x0F );

	if ( stepper_optic_state(0) ) m_optic_pattern |=  0x01;
	else                          m_optic_pattern &= ~0x01;
	if ( stepper_optic_state(1) ) m_optic_pattern |=  0x02;
	else                          m_optic_pattern &= ~0x02;

	awp_draw_reel(0);
	awp_draw_reel(1);
}

WRITE8_MEMBER(maygay1b_state::reel34_w)
{
	stepper_update(2, data & 0x0F );
	stepper_update(3, (data>>4) & 0x0F );

	if ( stepper_optic_state(2) ) m_optic_pattern |=  0x04;
	else                          m_optic_pattern &= ~0x04;
	if ( stepper_optic_state(3) ) m_optic_pattern |=  0x08;
	else                          m_optic_pattern &= ~0x08;

	awp_draw_reel(2);
	awp_draw_reel(3);
}

WRITE8_MEMBER(maygay1b_state::reel56_w)
{
	stepper_update(4, data & 0x0F );
	stepper_update(5, (data>>4) & 0x0F );

	if ( stepper_optic_state(4) ) m_optic_pattern |=  0x10;
	else                          m_optic_pattern &= ~0x10;
	if ( stepper_optic_state(5) ) m_optic_pattern |=  0x20;
	else                          m_optic_pattern &= ~0x20;

	awp_draw_reel(4);
	awp_draw_reel(5);
}

static UINT8 m1_duart_r (device_t *device)
{
	maygay1b_state *state = device->machine().driver_data<maygay1b_state>();
	return ~(state->m_optic_pattern);
}

WRITE8_MEMBER(maygay1b_state::m1_meter_w)
{
	int i;

	for (i=0; i<8; i++)
	if ( data & (1 << i) )  MechMtr_update(i, data & (1 << i) );
}

WRITE8_MEMBER(maygay1b_state::m1_latch_w)
{
	switch ( offset )
	{
		case 0: // m_RAMEN
		m_RAMEN = (data & 1);
		break;
		case 1: // AlarmEn
		m_ALARMEN = (data & 1);
		break;
		case 2: // Enable
		//printf("nmi enable %02x\n",data);
		m_NMIENABLE = (data & 1);
		break;
		case 3: // RTS
		{
		}
		break;
		case 4: // PSURelay
		m_PSUrelay = (data & 1);
		break;
		case 5: // WDog
		m_WDOG = (data & 1);
		break;
		case 6: // Srsel
		// this is the ROM banking?
		printf("rom bank %02x\n",data);
		m_SRSEL = (data & 1);
		break;
	}
}

WRITE8_MEMBER(maygay1b_state::latch_ch2_w)
{
	device_t *msm6376 = machine().device("msm6376");
	okim6376_w(msm6376, space, 0, data&0x7f);
	okim6376_ch2_w(msm6376,data&0x80);
}

//A strange setup this, the address lines are used to move st to the right level
READ8_MEMBER(maygay1b_state::latch_st_hi)
{
	device_t *msm6376 = machine().device("msm6376");
	okim6376_st_w(msm6376,1);
	return 0;
}

READ8_MEMBER(maygay1b_state::latch_st_lo)
{
	device_t *msm6376 = machine().device("msm6376");
	okim6376_st_w(msm6376,0);
	return 0;
}

READ8_MEMBER(maygay1b_state::m1_meter_r)
{
	device_t *ay8910 = machine().device("aysnd");
	return ~ay8910_read_ym(ay8910);
}

static ADDRESS_MAP_START( m1_memmap, AS_PROGRAM, 8, maygay1b_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x2000, 0x2000) AM_WRITE(reel12_w)
	AM_RANGE(0x2010, 0x2010) AM_WRITE(reel34_w)
	AM_RANGE(0x2020, 0x2020) AM_WRITE(reel56_w)

	// there is actually an 8279 and an 8051..
	AM_RANGE(0x2030, 0x2031) AM_READWRITE(m1_8279_r,m1_8279_w)
	AM_RANGE(0x2040, 0x2041) AM_READWRITE(m1_8279_2_r,m1_8279_2_w)
//  AM_RANGE(0x2050, 0x2050)// SCAN on M1B

	AM_RANGE(0x2070, 0x207f) AM_DEVREADWRITE_LEGACY("duart68681", duart68681_r, duart68681_w )

	AM_RANGE(0x2090, 0x2091) AM_DEVWRITE_LEGACY("aysnd", ay8910_address_data_w)
	AM_RANGE(0x20B0, 0x20B0) AM_READ(m1_meter_r)

	AM_RANGE(0x20A0, 0x20A3) AM_DEVWRITE("pia", pia6821_device, write)
	AM_RANGE(0x20A0, 0x20A3) AM_DEVREAD("pia", pia6821_device, read)

	AM_RANGE(0x20C0, 0x20C7) AM_WRITE(m1_latch_w)

	AM_RANGE(0x2400, 0x2401) AM_DEVWRITE_LEGACY("ymsnd", ym2413_w ) // 2149F??
	AM_RANGE(0x2404, 0x2405) AM_READ(latch_st_lo)
	AM_RANGE(0x2406, 0x2407) AM_READ(latch_st_hi)

	AM_RANGE(0x2412, 0x2412) AM_READ(m1_firq_trg_r) // firq, sample playback?



	AM_RANGE(0x2420, 0x2421) AM_WRITE(latch_ch2_w ) // oki

	AM_RANGE(0x2800, 0xffff) AM_ROM
ADDRESS_MAP_END

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(maygay1b_state,m1_meter_w),
	DEVCB_NULL,
};

static const duart68681_config maygaym1_duart68681_config =
{
	duart_irq_handler,
	NULL,
	m1_duart_r,
	NULL
};

// machine driver for maygay m1 board /////////////////////////////////



MACHINE_CONFIG_START( maygay_m1, maygay1b_state )

	MCFG_CPU_ADD("maincpu", M6809, M1_MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(m1_memmap)

	MCFG_DUART68681_ADD("duart68681", M1_DUART_CLOCK, maygaym1_duart68681_config)
	MCFG_PIA6821_ADD("pia", m1_pia_intf)
	MCFG_MSC1937_ADD("vfd",0,RIGHT_TO_LEFT)
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd",AY8913, M1_MASTER_CLOCK)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ymsnd", YM2413, M1_MASTER_CLOCK/4) // should be a 2149F?

	MCFG_SOUND_ADD("msm6376", OKIM6376, M1_MASTER_CLOCK/4) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("nmitimer", maygay1b_state, maygay1b_nmitimer_callback, attotime::from_hz(75)) // freq?

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEFAULT_LAYOUT(layout_maygay1b)
MACHINE_CONFIG_END








WRITE8_MEMBER(maygay1b_state::m1ab_no_oki_w)
{
	popmessage("write to OKI, but no OKI rom");
}

DRIVER_INIT_MEMBER(maygay1b_state,m1)
{
	//AM_RANGE(0x2420, 0x2421) AM_WRITE(latch_ch2_w ) // oki
	// if there is no OKI region disable writes here, the rom might be missing, so alert user

	UINT8 *okirom = machine().root_device().memregion( "msm6376" )->base();

	if (!okirom) {
		machine().device("maincpu")->memory().space(AS_PROGRAM).install_write_handler(0x2420, 0x2421, write8_delegate(FUNC(maygay1b_state::m1ab_no_oki_w), this));
	}
	// print out the rom id / header info to give us some hints
	// note this isn't always correct, alley cat has 'Calpsyo' still in the ident string?
	{
		UINT8 *cpu = machine().root_device().memregion( "maincpu" )->base();
		int base = 0xff20;
		for (int i=0;i<14;i++)
		{
			for (int j=0;j<16;j++)
			{
				UINT8 rom = cpu[base];

				if ((rom>=0x20) && (rom<0x7f))
				{
					printf("%c", rom);
				}
				else
				{
					printf("*");
				}

				base++;
			}
			printf("\n");
		}
	}
}
