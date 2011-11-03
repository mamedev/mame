/*****************************************************************************************

    Maygay M1 A/B driver, (under heavy construction !!!)

    A.G.E Code Copyright J. Wallace and the AGEMAME Development Team.
    Visit http://agemame.mameworld.info for more information.

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
#include "cpu/m6809/m6809.h"
#include "video/awpvid.h"		//Fruit Machines Only
#include "machine/6821pia.h"
#include "machine/68681.h"
#include "machine/meters.h"
#include "machine/roc10937.h"	// vfd
#include "machine/steppers.h"	// stepper motor
#include "sound/ay8910.h"
#include "sound/2413intf.h"
#include "sound/okim6376.h"
#include "machine/nvram.h"

#define VERBOSE 1
#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define M1_MASTER_CLOCK (XTAL_8MHz)
#define M1_DUART_CLOCK  (XTAL_3_6864MHz)

struct i8279_state
{
	UINT8		command;
	UINT8		mode;
	UINT8		prescale;
	UINT8		inhibit;
	UINT8		clear;
	UINT8		ram[16];
	UINT8		read_sensor;
	UINT8		write_display;
	UINT8		sense_address;
	UINT8		sense_auto_inc;
	UINT8		disp_address;
	UINT8		disp_auto_inc;
};


class maygay1b_state : public driver_device
{
public:
	maygay1b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_lamppos;
	int m_alpha_clock;
	int m_RAMEN;
	int m_ALARMEN;
	int m_PSUrelay;
	int m_WDOG;
	int m_SRSEL;
	UINT8 m_Lamps[256];
	int m_optic_pattern;
	device_t *m_duart68681;
	i8279_state m_i8279[2];
};


static void m1_draw_lamps(maygay1b_state *state, int data,int strobe, int col)
{
	int i;
	int scramble[8] = { 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08 };

	state->m_lamppos = strobe + col * 8;

	for ( i = 0; i < 8; i++ )
	{
		state->m_Lamps[state->m_lamppos] = ( data & scramble[i] );
		output_set_lamp_value(state->m_lamppos, state->m_Lamps[state->m_lamppos]);
	}
	state->m_lamppos++;
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

static READ8_HANDLER( m1_8279_r )
{
	maygay1b_state *state = space->machine().driver_data<maygay1b_state>();
	i8279_state *chip = state->m_i8279 + 0;
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
				result = input_port_read(space->machine(),"SW1");
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
			result = input_port_read(space->machine(),portnames[chip->sense_address]);
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

static WRITE8_HANDLER( m1_8279_w )
{
	maygay1b_state *state = space->machine().driver_data<maygay1b_state>();
	i8279_state *chip = state->m_i8279 + 0;
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
			m1_draw_lamps(state,chip->ram[chip->disp_address],chip->disp_address, 0);
		}
		chip->ram[chip->disp_address] = data;
		if ( chip->disp_auto_inc )
		chip->disp_address ++;
	}
}

static READ8_HANDLER( m1_8279_2_r )
{
	maygay1b_state *state = space->machine().driver_data<maygay1b_state>();
	i8279_state *chip = state->m_i8279 + 1;
	UINT8 result = 0xff;
	UINT8 addr;

	/* read data */
	if ((offset & 1) == 0)
	{
		switch (chip->command & 0xe0)
		{
			/* read sensor RAM */
			case 0x40:
				//result = ~input_port_read(machine,"DSW1");  /* DSW 1 - inverted! */
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


static WRITE8_HANDLER( m1_8279_2_w )
{
	maygay1b_state *state = space->machine().driver_data<maygay1b_state>();
	i8279_state *chip = state->m_i8279 + 1;
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
			m1_draw_lamps(state,chip->ram[chip->disp_address],chip->disp_address, 128);
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

static MACHINE_RESET( m1 )
{
	maygay1b_state *state = machine.driver_data<maygay1b_state>();
	ROC10937_reset(0);	// reset display1
	state->m_duart68681 = machine.device( "duart68681" );
	m1_stepper_reset(machine);
}

///////////////////////////////////////////////////////////////////////////

static void duart_irq_handler(device_t *device, UINT8 state)
{
	cputag_set_input_line(device->machine(), "maincpu", M6809_IRQ_LINE, state?ASSERT_LINE:CLEAR_LINE);
	LOG(("6809 irq%d \n",state));
}

#if 0
static void cpu0_firq(int state)
{
	cpunum_set_input_line(Machine, 0, M6809_FIRQ_LINE, state?ASSERT_LINE:CLEAR_LINE);
	LOG(("6809 firq%d \n",state));
}
#endif

static void cpu0_nmi(running_machine &machine, int state)
{
	cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, state?ASSERT_LINE:CLEAR_LINE);
	LOG(("6809 nmi%d \n",state));
}

/***************************************************************************
    6821 PIA
***************************************************************************/

static WRITE8_DEVICE_HANDLER( m1_pia_porta_w )
{
	maygay1b_state *state = device->machine().driver_data<maygay1b_state>();
	if ( data & 0x40 ) ROC10937_reset(0);

	if ( !state->m_alpha_clock && (data & 0x20) )
	{
		ROC10937_shift_data(0, ( data & 0x10 )?0:1);
	}

	state->m_alpha_clock = data & 0x20;

	ROC10937_draw_16seg(0);
}

static WRITE8_DEVICE_HANDLER( m1_pia_portb_w )
{
	int i;
	for (i=0; i<8; i++)
		if ( data & (1 << i) )		output_set_indexed_value("triac", i, data & (1 << i));
}

static const pia6821_interface m1_pia_intf =
{
	DEVCB_NULL,		/* port A in */
	DEVCB_NULL,		/* port B in */
	DEVCB_NULL,		/* line CA1 in */
	DEVCB_NULL,		/* line CB1 in */
	DEVCB_NULL,		/* line CA2 in */
	DEVCB_NULL,		/* line CB2 in */
	DEVCB_HANDLER(m1_pia_porta_w),		/* port A out */
	DEVCB_HANDLER(m1_pia_portb_w),		/* port B out */
	DEVCB_NULL,		/* line CA2 out */
	DEVCB_NULL,		/* port CB2 out */
	DEVCB_NULL,		/* IRQA */
	DEVCB_NULL		/* IRQB */
};

// input ports for M1 board ////////////////////////////////////////

static INPUT_PORTS_START( m1 )
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

static MACHINE_START( m1 )
{
	int i;

// setup 8 mechanical meters ////////////////////////////////////////////
	MechMtr_config(machine,8);

// setup 6 default 96 half step reels ///////////////////////////////////
	for ( i = 0; i < 6; i++ )
	{
		stepper_config(machine, i, &starpoint_interface_48step);
	}

// setup the standard oki MSC1937 display ///////////////////////////////
	ROC10937_init(0, MSC1937,0);
}
static WRITE8_HANDLER( reel12_w )
{
	maygay1b_state *state = space->machine().driver_data<maygay1b_state>();
	stepper_update(0, data & 0x0F );
	stepper_update(1, (data>>4) & 0x0F );

	if ( stepper_optic_state(0) ) state->m_optic_pattern |=  0x01;
	else                          state->m_optic_pattern &= ~0x01;
	if ( stepper_optic_state(1) ) state->m_optic_pattern |=  0x02;
	else                          state->m_optic_pattern &= ~0x02;

	awp_draw_reel(0);
	awp_draw_reel(1);
}

static WRITE8_HANDLER( reel34_w )
{
	maygay1b_state *state = space->machine().driver_data<maygay1b_state>();
	stepper_update(2, data & 0x0F );
	stepper_update(3, (data>>4) & 0x0F );

	if ( stepper_optic_state(2) ) state->m_optic_pattern |=  0x04;
	else                          state->m_optic_pattern &= ~0x04;
	if ( stepper_optic_state(3) ) state->m_optic_pattern |=  0x08;
	else                          state->m_optic_pattern &= ~0x08;

	awp_draw_reel(2);
	awp_draw_reel(3);
}

static WRITE8_HANDLER( reel56_w )
{
	maygay1b_state *state = space->machine().driver_data<maygay1b_state>();
	stepper_update(4, data & 0x0F );
	stepper_update(5, (data>>4) & 0x0F );

	if ( stepper_optic_state(4) ) state->m_optic_pattern |=  0x10;
	else                          state->m_optic_pattern &= ~0x10;
	if ( stepper_optic_state(5) ) state->m_optic_pattern |=  0x20;
	else                          state->m_optic_pattern &= ~0x20;

	awp_draw_reel(4);
	awp_draw_reel(5);
}

static UINT8 m1_duart_r (device_t *device)
{
	maygay1b_state *state = device->machine().driver_data<maygay1b_state>();
	return (state->m_optic_pattern);
}

static WRITE8_DEVICE_HANDLER( m1_meter_w )
{
	int i;

	for (i=0; i<8; i++)
	if ( data & (1 << i) )	MechMtr_update(i, data & (1 << i) );
}

static WRITE8_HANDLER( m1_latch_w )
{
	maygay1b_state *state = space->machine().driver_data<maygay1b_state>();
	switch ( offset )
	{
		case 0: // state->m_RAMEN
		state->m_RAMEN = (data & 1);
		break;
		case 1: // AlarmEn
		state->m_ALARMEN = (data & 1);
		break;
		case 2: // Enable
	      cpu0_nmi(space->machine(),1);
	      cpu0_nmi(space->machine(),0);

		  //        if ( m1_enable == 0 && ( data & 1 ) && Vmm )
//      {
	//      cpu0_nmi(1)
		//  m1_enable = (data & 1);
//      }
		break;
		case 3: // RTS
		{
		}
		break;
		case 4: // PSURelay
		state->m_PSUrelay = (data & 1);
		break;
		case 5: // WDog
		state->m_WDOG = (data & 1);
		break;
		case 6: // Srsel
		state->m_SRSEL = (data & 1);
		break;
	}
}

static WRITE8_HANDLER( latch_ch2_w )
{
	device_t *msm6376 = space->machine().device("msm6376");
	okim6376_w(msm6376, 0, data&0x7f);
	okim6376_ch2_w(msm6376,data&0x80);
}

//A strange setup this, the address lines are used to move st to the right level
static READ8_HANDLER( latch_st_hi )
{
	device_t *msm6376 = space->machine().device("msm6376");
	okim6376_st_w(msm6376,1);
	return 0;
}

static READ8_HANDLER( latch_st_lo )
{
	device_t *msm6376 = space->machine().device("msm6376");
	okim6376_st_w(msm6376,0);
	return 0;
}

static ADDRESS_MAP_START( m1_memmap, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("nvram")

	AM_RANGE(0x2000, 0x2000) AM_WRITE(reel12_w)
	AM_RANGE(0x2010, 0x2010) AM_WRITE(reel34_w)
	AM_RANGE(0x2020, 0x2020) AM_WRITE(reel56_w)

	AM_RANGE(0x2030, 0x2031) AM_READWRITE(m1_8279_r,m1_8279_w)
	AM_RANGE(0x2040, 0x2041) AM_READWRITE(m1_8279_2_r,m1_8279_2_w)
	AM_RANGE(0x2050, 0x2050)// SCAN on M1B

	AM_RANGE(0x2070, 0x207f) AM_DEVREADWRITE( "duart68681", duart68681_r, duart68681_w )

	AM_RANGE(0x2090, 0x2091) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x20B0, 0x20B0) AM_DEVREAD("aysnd", ay8910_r)

	AM_RANGE(0x20A0, 0x20A3) AM_DEVWRITE_MODERN("pia", pia6821_device, write)
	AM_RANGE(0x20A0, 0x20A3) AM_DEVREAD_MODERN("pia", pia6821_device, read)

	AM_RANGE(0x20C0, 0x20C7) AM_WRITE(m1_latch_w)

	AM_RANGE(0x2400, 0x2401) AM_DEVWRITE( "ymsnd", ym2413_w )
	AM_RANGE(0x2404, 0x2405) AM_READ(latch_st_lo)
	AM_RANGE(0x2406, 0x2407) AM_READ(latch_st_hi)

	AM_RANGE(0x2420, 0x2421) AM_WRITE( latch_ch2_w ) // oki

	AM_RANGE(0x2800, 0xffff) AM_ROM
ADDRESS_MAP_END

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(m1_meter_w),
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

static MACHINE_CONFIG_START( m1, maygay1b_state )

	MCFG_MACHINE_START(m1)
	MCFG_MACHINE_RESET(m1)
	MCFG_CPU_ADD("maincpu", M6809, M1_MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(m1_memmap)

	MCFG_DUART68681_ADD("duart68681", M1_DUART_CLOCK, maygaym1_duart68681_config)
	MCFG_PIA6821_ADD("pia", m1_pia_intf)

	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("aysnd",AY8913, M1_MASTER_CLOCK)
	MCFG_SOUND_CONFIG(ay8910_config)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SOUND_ADD("ymsnd", YM2413, M1_MASTER_CLOCK/4)

	MCFG_SOUND_ADD("msm6376", OKIM6376, M1_MASTER_CLOCK/4) //?
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEFAULT_LAYOUT(layout_awpvid16)
MACHINE_CONFIG_END








// I assume all sets have this, or is M1B different? We don't use it right now anyway.
#define ROM_END_M1A_MCU \
	ROM_REGION( 0x1000, "mcu", ROMREGION_ERASE00  ) \
	ROM_LOAD( "m1a-mcu.bin", 0x0000, 0x1000, CRC(ae957b00) SHA1(83267b64335b4ab33cc033d5003c4c93c813fc37) ) \
	ROM_END \


#define m1_apollo_sound \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  ) \
	ROM_LOAD( "apl9snd1.bin", 0x000000, 0x080000, CRC(92ebbfb6) SHA1(ef15183416a208d5f51d9121af823ccbab53fc9f) )\
	ROM_LOAD( "apl9snd2.bin", 0x080000, 0x080000, CRC(83c1aba4) SHA1(fc1e2bd46be5de4edd4b66d06616e1ad805e35d8) )\

// I don't think this is correct, or at least it's not OKI, no header.
#define m1_bargn_sound \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  ) \
	ROM_LOAD( "bgsnd", 0x0000, 0x020000, CRC(abe7c01d) SHA1(21caadcd149772dfd79a9d30ebc1d8da91ff36f4) )\

#define m1_cik_sound \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  ) \
	ROM_LOAD( "digi153_1.bin", 0x000000, 0x080000, CRC(107d92c8) SHA1(7ef1f1bf2c91216e0060350f06a89ca9ea948a9a) )\
	ROM_LOAD( "digi153_2.bin", 0x080000, 0x080000, CRC(59ab59d4) SHA1(0b55151f356c6866c0b37442a098c211e05715af) )\

#define m1_clbfvr_sound \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )\
	ROM_LOAD( "cl_fever.sn1", 0x000000, 0x080000, CRC(0a5df5d2) SHA1(d73778a415656cd3880d5a011e015a760386a676) )\
	ROM_LOAD( "cl_fever.sn2", 0x080000, 0x080000, CRC(1d0b5e44) SHA1(fd21e4658dcc6a919b326eed81ccec942c6f1989) )\

#define m1_cluedo_sound \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )\
	ROM_LOAD( "cluedosnd1.hex", 0x000000, 0x080000, CRC(5a18a395) SHA1(d309acb315a2f62306e850308424c98744dfc6eb) )\
	ROM_LOAD( "cluedosnd2.hex", 0x080000, 0x080000, CRC(0aa15ee0) SHA1(eb156743a44e66b86c0c0443db0356e2f25d1cd2) )\

#define m1_cluecb_sound \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )\
	ROM_LOAD( "cluclub1", 0x000000, 0x080000, CRC(05e928ed) SHA1(41ae1f5342dc7afbdbdf3871e29d2a85c65a5965) )\
	ROM_LOAD( "cluclub2", 0x080000, 0x080000, CRC(91811c0e) SHA1(88b3259b241136cd549ed9b4930d165896eebcc4) )\

#define m1_cluess_sound \
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )\
	ROM_LOAD( "supersleuth.p1", 0x000000, 0x080000, CRC(2417208f) SHA1(5c51bdcfa566aa8b2379d529441d37b2145864bb) )\
	ROM_LOAD( "supersleuth.p2", 0x080000, 0x080000, CRC(892d3a4d) SHA1(bb585a9fda56f2f0859707973f771d60c5dfa080) )\

ROM_START( m1albsq )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-056", 0x0000, 0x010000, CRC(6f3bc318) SHA1(6dbe6e02ca762a8ffaed9c89a0da5f6a10d829cc) )//4.1 Normal Token
ROM_END_M1A_MCU

ROM_START( m1albsqp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-057", 0x0000, 0x010000, CRC(87d35efe) SHA1(6c40fa3d27e66d91a61f23eabcdbf273e7023a92) )//4.1 Prot Token
ROM_END_M1A_MCU

ROM_START( m1albsq3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-188.bin", 0x0000, 0x010000, CRC(e1d0822d) SHA1(47471baed1c98b597785d16784971b89210ff3f1) )//3.0 LNA
ROM_END_M1A_MCU

ROM_START( m1albsq2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-443", 0x0000, 0x010000, CRC(a8c4e6df) SHA1(3e390ce7ed708d481fc0087caf4fc6642fde5a74) )//2.2 LNA Token
ROM_END_M1A_MCU

ROM_START( m1albsq1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-055.bin", 0x0000, 0x010000, CRC(922a0396) SHA1(0e9949aef572a45c1f2c245c815659c332dfd8f5) )//1.1 LNA
ROM_END_M1A_MCU

ROM_START( m1albsq1p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-056.bin", 0x0000, 0x010000, CRC(2692dd45) SHA1(7d31d672d0a1ade2d9ed5542b1a83c090a59f4ec) )//1.1 LPA Token
ROM_END_M1A_MCU

ROM_START( m1alley )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa001029", 0x0000, 0x020000, CRC(d4c80f2c) SHA1(b7f3dcf025e18dc1ba7117f5129a64e2e01975a7) )
ROM_END_M1A_MCU

ROM_START( m1apollo )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk991006", 0x0000, 0x020000, CRC(4107d605) SHA1(0141cfc5d4265cea654c01790054401525ebb3c1) )//A1 Newest
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollop )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk991007", 0x0000, 0x020000, CRC(47130a78) SHA1(1d3f99c54a20851880d91af4b8ebb6f8a14a7ec1) )//A1 Newest P
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apolloh )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "a95p8c.bin", 0x0000, 0x020000, CRC(838a553c) SHA1(b2a4be0ce7cb81cac108c5e17d7d827e11a616d1) ) //5p GBP 8 All Cash Version A1, possible hack, copyright string NOPd
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apolloa )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-272", 0x0000, 0x020000, CRC(07f333ce) SHA1(2e6e18aba4af0a16caf0f67382dab5e76a2a8978) )//A.1
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apolloap )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-273", 0x0000, 0x020000, CRC(57b2cd72) SHA1(53f423efeefbca01b6eb43b1feecd8b052a99d1c) )//A.1 protocol
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apolloao )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-200", 0x0000, 0x020000, CRC(c0c33754) SHA1(d820ac1902beeb869cd814287c1795af6b81e231) ) //Version A1 15GBP?
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo9 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-196", 0x0000, 0x020000, CRC(54e17213) SHA1(05664df0eaaf8fc6f63ee5a7cf60384adf0f66aa) ) //9.1 6GBP?
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo9p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-197", 0x0000, 0x020000, CRC(04a08caf) SHA1(23b95350f1d33141e0b0121ac95632410786d918) )//9.1 P
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo4 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-072", 0x0000, 0x020000, CRC(89b3cd45) SHA1(2deed5d9d1cee6912585c6bc17e39803947647ab) )//4.1
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo4p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-073", 0x0000, 0x020000, CRC(d9f233f9) SHA1(ce185d4df53f092a9d2d4094f050c3893dc458aa) )//4.1 Protocol
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo11 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-735", 0x0000, 0x020000, CRC(d0d34746) SHA1(f8090490f38d1471da12ad9ebb95994c4af82441) )//1.1 (probably 11!) 10GBP
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo11b )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-735b", 0x0000, 0x020000, CRC(67552412) SHA1(dfb9dcb4a316646cee9f5c3ef278baad3065ef82) )//Has BwB string at the end, overdump?
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo11p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-736", 0x0000, 0x020000, CRC(c3147c10) SHA1(8135f17f4bad31cc1be4033f52fccac09354a547) )//1.1 (11!) Protocol
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo8 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-633", 0x0000, 0x020000, CRC(c5a0ca1b) SHA1(82084653b5fd6f7b407b010c31fcc9a32cafa20e) )//8.1
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo8p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-634", 0x0000, 0x020000, CRC(516a7a99) SHA1(b31c23fe9628d1251c755c70efa14ff50309837f) )//8.1 Protocol
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo7 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-631", 0x0000, 0x020000, CRC(6bc030cb) SHA1(aa0ec327a935f1b85753babc4c422df792baab7c) )//7.1
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo7p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-632", 0x0000, 0x020000, CRC(78070b9d) SHA1(84742572c86339f1f08ade232204086d2707ae26) )//7.1 Protocol
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo5 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-516", 0x0000, 0x020000, CRC(a3e3cbda) SHA1(5f918f4218e409752ea65083afbb7c901fc21839) )//5.1
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo5p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-517", 0x0000, 0x020000, CRC(f3a23566) SHA1(9f62f75f46c45e41e1066ff8db10e428dcd813d8) )//5.1 Protocol
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo4o )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-482", 0x0000, 0x020000, CRC(b669f8da) SHA1(0f0fb630a28b55f3cb8085633cba07e0568c96a6) ) //4.1 token
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo3 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-480", 0x0000, 0x020000, CRC(ea260f7a) SHA1(e63a8497336245bb80e9c079c6f426211c3bb371) )//3.1
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo3p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-481", 0x0000, 0x020000, CRC(ba67f1c6) SHA1(a2663261302d14c564ff60de4e44a9961ebbcee7) )//3.1 Protocol
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo2 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-478", 0x0000, 0x020000, CRC(b03371a5) SHA1(3c46e170e362ce23033232393fc0763e98e60a1c) )//2.1
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1apollo2p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-479", 0x0000, 0x020000, CRC(e0728f19) SHA1(d6c229fa2aed7af46865244d2abd2b2eb8dfc6a5) )//2.1 Protocol
	m1_apollo_sound
ROM_END_M1A_MCU

ROM_START( m1bankbs )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )//2.9
	ROM_LOAD( "sc010017", 0x0000, 0x020000, CRC(009a5a76) SHA1(296ef801e9b5dcca3deb3bbaa8f48e66147f5d9a) )
ROM_END_M1A_MCU

ROM_START( m1bankbsp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )//2.9 P
	ROM_LOAD( "sc010018", 0x0000, 0x020000, CRC(e6da0dd0) SHA1(fbcbe10ec10dee18127c9851994870f6b1073849) )
ROM_END_M1A_MCU

ROM_START( m1bankbso )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )//2.8
	ROM_LOAD( "sc001011", 0x0000, 0x020000, CRC(621404a6) SHA1(f43a1f4719bbe636d41d37e190cbc2634f6e9229) )
ROM_END_M1A_MCU

ROM_START( m1bankrl )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )//1.1
	ROM_LOAD( "sa5-226", 0x0000, 0x010000, CRC(7eba1e80) SHA1(a8224425fdf05ca53b0bbf51d088d51b88ac7345) )
ROM_END_M1A_MCU

ROM_START( m1bankrlp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )//1.1P
	ROM_LOAD( "sa5-227", 0x0000, 0x010000, CRC(96528366) SHA1(5927c519d9a610b1026d685647cdac285566b1fc) )
ROM_END_M1A_MCU

ROM_START( m1bankrl2p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )//2.1p
	ROM_LOAD( "sa5-225", 0x0000, 0x010000, CRC(14d2a45e) SHA1(8f5875dfd517826a89b3b13063474eda2725be68) )
ROM_END_M1A_MCU

ROM_START( m1bargn )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-077.bin", 0x0000, 0x010000, CRC(7ae8ea12) SHA1(537f828bdaba3c63abb83b5417a4ec115834a48a) )
	m1_bargn_sound
ROM_END_M1A_MCU

ROM_START( m1bargnp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-078.bin", 0x0000, 0x010000, CRC(dcc0b83e) SHA1(a85e3f60decb7dbc2de77b93dd8a79ff137d85b7) )
	m1_bargn_sound
ROM_END_M1A_MCU

ROM_START( m1bargnc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa2-141", 0x0000, 0x010000, CRC(9dcaaaa9) SHA1(6cd015990036c2e20e4f4a2e19a363e6c565b473) )
	m1_bargn_sound
ROM_END_M1A_MCU

ROM_START( m1bargncp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa2-142", 0x0000, 0x010000, CRC(2972747a) SHA1(6854a3cd9c6a834a24a0d8c763fa2a18c1d26a10) )
	m1_bargn_sound
ROM_END_M1A_MCU

ROM_START( m1bigdel )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "bdeal.p1", 0x0000, 0x020000, CRC(3cdebbb8) SHA1(8578441ef269c41fbe3f253055f687e1ccbf4770) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "bdeal.u2", 0x000000, 0x080000, CRC(7ef1d4dd) SHA1(97a99dd9325634ee28dda943d92257010c60306c) )
	ROM_LOAD( "bdeal.u3", 0x080000, 0x080000, CRC(28f48d4b) SHA1(642c2a2fc4f2faff510cbca2488ab6904bb489f6) )
ROM_END_M1A_MCU

ROM_START( m1blkhol )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "blackhole.bin", 0x0000, 0x010000, CRC(b0d92e24) SHA1(161a39efda1f7f1964d52f12c27bf7b8bc824e9e) )
ROM_END_M1A_MCU

ROM_START( m1bluemx )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-515", 0x0000, 0x010000, CRC(62451006) SHA1(0a1dd40097b378d8dc561894dbf587de7d47846b) )
ROM_END_M1A_MCU

ROM_START( m1bluemxp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-516", 0x0000, 0x010000, CRC(d6fdced5) SHA1(ea1afc8982683c799195116d7160c7d230e1db52) )
ROM_END_M1A_MCU

ROM_START( m1bondi )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-525", 0x0000, 0x020000, CRC(29f8f15c) SHA1(d4d023160c47322ce029409ddcf707556f571f27) )//1.1 newer
ROM_END_M1A_MCU

ROM_START( m1bondip )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-526", 0x0000, 0x020000, CRC(3a3fca0a) SHA1(1f00a87bfa1679c17a4eab79f0b30c0bb22ebe28) )//1.1p newer
ROM_END_M1A_MCU

ROM_START( m1bondi4 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-257", 0x0000, 0x020000, CRC(17c6dfac) SHA1(1d86653f68016c84df97240f12b841d1f8be5d10) )//4.1
ROM_END_M1A_MCU

ROM_START( m1bondi4p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-258", 0x0000, 0x020000, CRC(56667ec7) SHA1(e58a68e5ce5f77dbd295d346d4d797fafdef832b) )//4.1p
ROM_END_M1A_MCU

ROM_START( m1bondi3 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-259", 0x0000, 0x020000, CRC(0dee146c) SHA1(4f686847d94aec352c27a106ae57f2ea651ce75a) )//3.1 alt
ROM_END_M1A_MCU

ROM_START( m1bondi2 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-255", 0x0000, 0x020000, CRC(448f8f75) SHA1(abbbbf96284b41e2e4de5d22d24acaa19a019d51) )//2.1 in ROM
ROM_END_M1A_MCU

ROM_START( m1bondi2p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-256", 0x0000, 0x020000, CRC(5748b423) SHA1(687e2aafc2ea8a1e0a4097eed9d06a3b65e1d1ee) )//2.1p in ROM
ROM_END_M1A_MCU

ROM_START( m1bondi2po )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-227", 0x0000, 0x020000, CRC(f90f23d4) SHA1(45660c5a6b80f0f5eab924d3415c7b4687cd332d) )//2.1p
ROM_END_M1A_MCU

ROM_START( m1bondi1 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-208", 0x0000, 0x020000, CRC(cfc28e07) SHA1(06b214882252c9436afb441773de84e4be5cf17d) )//1.1
ROM_END_M1A_MCU

ROM_START( m1bondi1p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-209", 0x0000, 0x020000, CRC(9f8370bb) SHA1(205e03689926311755c6cc2700bb63eb88202046) )//1.1p
ROM_END_M1A_MCU

ROM_START( m1bounty )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "bhun004", 0x0000, 0x020000, CRC(c3ee9bb1) SHA1(f0a15411486a5dd9a906c2200813c38545f76b3f) )
ROM_END_M1A_MCU

ROM_START( m1bountc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc4-107.bin", 0x0000, 0x010000, CRC(0bdf41b3) SHA1(ce3564433a708ba50ca4099a26b1f75cf3cec947) )//1.3
ROM_END_M1A_MCU

ROM_START( m1bountcp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc4-108.bin", 0x0000, 0x010000, CRC(adf7139f) SHA1(5b2bd367df31e3c76d9fac2a71a90800d95c4719) )//1.3P
ROM_END_M1A_MCU

ROM_START( m1cik )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk991024", 0x0000, 0x020000, CRC(ac9d4f5c) SHA1(d75ef9a64357a9ac549e33b0671ed6704c283ab8) )//11
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cikp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk991025", 0x0000, 0x020000, CRC(aa899321) SHA1(6d02b0514fef131071aade01954810a735673655) )//11P
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cikh )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk991024h", 0x0000, 0x020000, CRC(dd1af636) SHA1(78ab959d14e7b2fd303aec50b12d5984e6fafcd0) )//cik5p8ct.bin - NOPd copyright string
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik51 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa991161", 0x0000, 0x020000, CRC(f8b4eba0) SHA1(fed049ff1a6418a28a3311bed181aa7719c68ca9) )//5.1
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik51p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa991162", 0x0000, 0x020000, CRC(118cb7b5) SHA1(8fd99b4b57c113a46cf644a3b9c7fe089606f54c) )//5.1P
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik51o )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-230", 0x0000, 0x020000, CRC(102216a2) SHA1(1d1f0800e3cb06396c2e9d4b018ec69aa19494c5) )//5.1 15v?
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik41 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa991159", 0x0000, 0x020000, CRC(e1b41832) SHA1(05199d157561647632915d612b4f2753511d7274) )//4.1
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik41p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa991160", 0x0000, 0x020000, CRC(8fb56e72) SHA1(f15c05151136e2f1182c32d700fa9dc369c666e1) )//4.1p
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik31 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-722", 0x0000, 0x020000, CRC(bdc48d9a) SHA1(818264c41e27bfd133de16c431db2cb5185a8418) )//3.1
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik31p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-723", 0x0000, 0x020000, CRC(ed857326) SHA1(b643df77d969c97b3d80e28112874a6d92dfe9d1) )//3.1P
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik12 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-138", 0x0000, 0x020000, CRC(18983237) SHA1(f12f2288383749ba12382f093937e5d6cafe9dc0) )//cik10fo 1.2
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik11 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-689", 0x0000, 0x020000, CRC(75b0075f) SHA1(34130ab64f3933395714181abdf9199313250827) )//1.1
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik11p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-690", 0x0000, 0x020000, CRC(0c351ea4) SHA1(bc3399086c0cda96127c9c316517784c251010f5) )//1.1P
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik11n )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-725", 0x0000, 0x020000, CRC(cd306165) SHA1(fc2342c91f84144dcc79cf3db6446260c2acb3a7) )//1.1
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik11np )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-726", 0x0000, 0x020000, CRC(def75a33) SHA1(f1d3db411ec3bcff2860a9db542b7f9ef1d057b1) )//1.1P
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik21 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-188", 0x0000, 0x020000, CRC(9d7eab15) SHA1(101d3f54bf09680c258bdf497e8132f9da7da7d9) )//2.1
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1cik21p )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-189", 0x0000, 0x020000, CRC(cd3f55a9) SHA1(f17595323a2b559d9d2711d23670d5facb449a33) )//2.1p
	m1_cik_sound
ROM_END_M1A_MCU

ROM_START( m1casclb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc1-170.bin", 0x0000, 0x010000, CRC(bf7094ce) SHA1(876a251b42efe8273ce7f941bd34f2349269f501) )//1.2
ROM_END_M1A_MCU

ROM_START( m1casclbp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc1-171.bin", 0x0000, 0x010000, CRC(1d20f2cf) SHA1(f2c9e1aa0c2c8903e293ef5ab3cc4b3d14349e64) )//1.2P
ROM_END_M1A_MCU

ROM_START( m1casclb1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sco-234.bin", 0x0000, 0x010000, CRC(27d11ba5) SHA1(f640a4902213997df0b612e8d1be48ac6d1e0569) )//1.1N
ROM_END_M1A_MCU

ROM_START( m1casgcl )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc3-524.bin", 0x0000, 0x010000, CRC(efde86a3) SHA1(aae7ecedb2ffdcae5fcb422574f9376b6a333497) )//1.2
ROM_END_M1A_MCU

ROM_START( m1casgclp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc3-525.bin", 0x0000, 0x010000, CRC(07361b45) SHA1(3caf71da665b6327e8337c5e8c39c110dbd4783e) )//1.2p
ROM_END_M1A_MCU

ROM_START( m1casroy )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc3-069.bin", 0x0000, 0x010000, CRC(0cc2707a) SHA1(b6db403cf0e7024a991e569be9f783325e09c76a) )//1.2
ROM_END_M1A_MCU

ROM_START( m1casroyp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc3-070.bin", 0x0000, 0x010000, CRC(8c548d12) SHA1(e86a0fc40f02c85a6139e11cc7824a3d05ba7dca) )//1.2p
ROM_END_M1A_MCU

ROM_START( m1casroy1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc1-197.bin", 0x0000, 0x010000, CRC(6e227a4a) SHA1(5070e3fa0e77f3e6ffa0915949e6c10ff3287fbf) )//1.1
ROM_END_M1A_MCU

ROM_START( m1chain )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "chainreaction.bin", 0x0000, 0x020000, CRC(0dfa71c7) SHA1(0979dd48000c9c9a03448a0ffdc9395bb131a5dd) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "chainreactionsnd.p1", 0x000000, 0x080000, CRC(25d6cc90) SHA1(e801219edff7745ec71cc146e7bf85e4ad8eb363) )
	ROM_LOAD( "chainreactionsnd.p2", 0x080000, 0x080000, CRC(77690cea) SHA1(10c655ab5ec922c31ab895b91096c89ef8220f99) )
ROM_END_M1A_MCU

ROM_START( m1clbfvr )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc001000", 0x0000, 0x020000, CRC(65ecc208) SHA1(2bf54a1e78d2816ec74137c63a3b44710c373baf) )//1.1n
	m1_clbfvr_sound
ROM_END_M1A_MCU

ROM_START( m1clbfvrp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc001001", 0x0000, 0x020000, CRC(63f81e75) SHA1(db9220265fbe53cc1bec0c448046a031e14ebe13) )//1.1p
	m1_clbfvr_sound
ROM_END_M1A_MCU


ROM_START( m1cluedo )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-110", 0x0000, 0x010000, CRC(d80c8f47) SHA1(73be41e2ba96d4f6759d375d61b9208b8516f59e) )//nhf 6.1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedop )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-111", 0x0000, 0x010000, CRC(30e412a1) SHA1(0a8755158d905bbb582092d4525dc866f05e77ca) )//phf 6.1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo5 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-521.bin", 0x0000, 0x010000, CRC(2fffafe0) SHA1(c15ebc0f8cc574a70c8f94a90c2f5381cc647f75) )//nhf 5.2 1995
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo5p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-522.bin", 0x0000, 0x010000, CRC(9b477133) SHA1(6dc0db8a50c166c1f8ae7aa06caa0869aefcaf09) )//phf 5.2 1995
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo4 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-301.bin", 0x0000, 0x010000, CRC(be3c3b72) SHA1(9bdf85c2c795e1920a9321511f4cd76e8c320ecc) )//nhf 4.1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo4p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-302.bin", 0x0000, 0x010000, CRC(0a84e5a1) SHA1(07f75bcebc3d27605e925800878206a006bd81a7) )//phf 4.1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo3 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-244.bin", 0x0000, 0x010000, CRC(cae6838d) SHA1(bfb53c21d9a23862d9d3e36f604dbfd9b8cb5651) )//nhf 3.1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo3p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-245.bin", 0x0000, 0x010000, CRC(220e1e6b) SHA1(46f5b3052e0dfeeb3b7d3c0819efc821d466d3b0) )//phf 3.1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo3h )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "clu20p10", 0x0000, 0x010000, CRC(7627bba4) SHA1(9d7d8be375324c492373e859f22f10d23249aba2) )//nhf 3.0, no string
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-235.bin", 0x0000, 0x010000, CRC(31f3d17f) SHA1(89c9715877d855885bb81383e3e2cb47a7945a78) )//nhf 1.1 1993
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo1p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-236.bin", 0x0000, 0x010000, CRC(854b0fac) SHA1(16ef9499a5223804042404698b7b66948bb35255) )//phf 1.1 1993
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedo1h )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cluedo.bin", 0x0000, 0x010000, CRC(d9cd3491) SHA1(c0f753a291ac30cd5af99b611b3cea16f1f8c599) )//nhf 1.0
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedoi )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-552.bin", 0x0000, 0x010000, CRC(70588685) SHA1(fa37feb43838a52087d6584004a43f006d0129cb) )//nhf 7.2 IoM 1995
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedoip )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-553.bin", 0x0000, 0x010000, CRC(98b01b63) SHA1(aa56d3a7c34a60f93da0dafd416a7550416076ee) )//phf 7.2 IoM 1995
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedon )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-519", 0x0000, 0x010000, CRC(6bc4eb57) SHA1(1581e14a25786dda8a98ee2643c63bcb4caaea89) )//nhf 1.2 1995
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedonp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-520", 0x0000, 0x010000, CRC(b4bfc448) SHA1(493b020a4d0a18ec2fd8f45d2eea1590b948ea51) )//phf 1.2 1995
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedob2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-302", 0x0000, 0x010000, CRC(08031924) SHA1(9aa997328285502e56af0d1552220ca27bcc053a) )//nbu 2.1 BwB 1995
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedob2p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-303", 0x0000, 0x010000, CRC(e0eb84c2) SHA1(3c2db9d41b9c561a483293f2258b654547d937d4) )//pbu 2.1 BwB 1995
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedob1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-326", 0x0000, 0x010000, CRC(0ace83db) SHA1(3387ac8583bc4ba7933abe001bc64a2b06a5451f) )//nbu 1.1 BwB 1995 (newer) cl_x6___.2o1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedob1p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-327", 0x0000, 0x010000, CRC(e2261e3d) SHA1(50be56d1c28a31fcdb12a83c230a983bd6c60f62) )//pbu 1.1 BwB 1995 (newer)cl_x6_d_.2o1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedobi2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-328", 0x0000, 0x010000, CRC(7aa2e6bd) SHA1(82752cd0e89d487016ccb50e55d2d7e11e5bb0ad) )//nbu 2.1 BwB 1995 (IoM) clix6___.2o1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedobi2p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-329", 0x0000, 0x010000, CRC(924a7b5b) SHA1(102fba24040c9f968f80747b830fda05444c69aa) )//pbu 2.1 BwB 1995 (IoM) clix6_d_.2o1
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedob2h )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "clu5p8", 0x0000, 0x010000, CRC(fb6c6527) SHA1(e42683512c537d653593c67a8d238069ac2f2d0e) )//nbu 2.0 - nop'd copyright string
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluedob1h )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "clu55", 0x0000, 0x010000, CRC(c852a989) SHA1(ff79e65f80d4230a0e0f9e87a9dcc544a5197a1a) )//nbu 1.0 - nop'd copyright string
	m1_cluedo_sound
ROM_END_M1A_MCU

ROM_START( m1cluecb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc5-437", 0x0000, 0x010000, CRC(0282878c) SHA1(90624916699e5866678b02260e0b0502041f32bf) )//nhq 3.1
	m1_cluecb_sound
	ROM_END_M1A_MCU

ROM_START( m1cluecbp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc5-438", 0x0000, 0x010000, CRC(26f6d094) SHA1(31fa78db1a581c00b39d4f6f64d8f08786dec97a) )//phq 3.1 (typo?)
	m1_cluecb_sound
	ROM_END_M1A_MCU

ROM_START( m1cluecb2 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc5-435", 0x0000, 0x010000, CRC(84095d36) SHA1(f86f7f25fa25eb2731d81a23aafaf5e4c8aa976b) )//nhq 2.1
	m1_cluecb_sound
	ROM_END_M1A_MCU

ROM_START( m1cluecb2p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc5-436", 0x0000, 0x010000, CRC(3b277a98) SHA1(cab3b277bec84056f6a97a87d97bbd86f80a1c8c) )//phq 2.1
	m1_cluecb_sound
ROM_END_M1A_MCU

ROM_START( m1cluecb1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc5-433", 0x0000, 0x010000, CRC(27254937) SHA1(b75f4a7e66f625c0db7d658f0427c8c1893a3d10) )//nhq 1.1
	m1_cluecb_sound
ROM_END_M1A_MCU

ROM_START( m1cluecb1p )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc5-434", 0x0000, 0x010000, CRC(6185ea69) SHA1(a20bccb86cfcd929908974500186e9ecf2cdc55b) )//phq 1.1
	m1_cluecb_sound
ROM_END_M1A_MCU

//These have no sound ROMs, could they use the standard cluedo ones?
ROM_START( m1cluesh )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-098", 0x0000, 0x010000, CRC(36c70b9d) SHA1(24659224f26a3bc0efea3b71666fc5c52479cb06) )//nhj 1.2
ROM_END_M1A_MCU

ROM_START( m1clueshp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-099", 0x0000, 0x010000, CRC(de2f967b) SHA1(e30c1e93cf47200d683cd00de232d017c00b9976) )//phj 1.2
ROM_END_M1A_MCU

ROM_START( m1cluesho )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-492", 0x0000, 0x010000, CRC(824eafd8) SHA1(19beeb7238eddfed4917dc809a620b695d2d8098) )//nhj 1.2
ROM_END_M1A_MCU

ROM_START( m1clueshop )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-493", 0x0000, 0x010000, CRC(6aa6323e) SHA1(fb45b027259cb703ac31230465a65f39e834c0f2) )//phj 1.2
ROM_END_M1A_MCU

ROM_START( m1cluess )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk001027", 0x0000, 0x020000, CRC(4b4c9c92) SHA1(5c981b19175491c275668a5686a15b77571cc8e7) )//ncf 2.3
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessh )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sleu5p8c.bin", 0x0000, 0x020000, CRC(e4fc65d7) SHA1(ba573a33247682a1a1a213381e49fe390c661b8c) )//hack ncf 2.3
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk001028", 0x0000, 0x020000, CRC(ad0ccb34) SHA1(1cfe0cc945ba3fe91645301abca40285984084e3) )//pcf 2.3
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessa )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk001025", 0x0000, 0x020000, CRC(9b30d25a) SHA1(185c2f56d69dc6635c75c18bc1c4f342d94a3c96) )//ncf 1.2
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessap )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sk001026", 0x0000, 0x020000, CRC(72088e4f) SHA1(1e41b656c0a7b45cc4f7c12a5c14b899b8ce24da) )//pcf 1.2
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessb )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-260", 0x0000, 0x020000, CRC(4302c3de) SHA1(12ba5359ecfa502cd2f548f83fb4cea1d84cdec6) )//ncl71
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessbp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-261", 0x0000, 0x020000, CRC(13433d62) SHA1(24bbb6425cd8156996fb17bd23672c59b8aa10d6) )//pcl71
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessc )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-258", 0x0000, 0x020000, CRC(88d5d9d0) SHA1(f500925d916ad20673d02f4de9eadacb08f1d8e1) )//ncl61
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluesscp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-259", 0x0000, 0x020000, CRC(d894276c) SHA1(bf9e19b1d1606d4d75916d5d405bbf2ad89d9211) )//pcl61
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessd )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-252", 0x0000, 0x020000, CRC(19610dd3) SHA1(7c4130e25285ce4804838dc0b776eff97a073d3e) )//ncl51
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessdp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-253", 0x0000, 0x020000, CRC(4920f36f) SHA1(bdece8e68faa5b5dc07d2d5ad87b8f41b4899831) )//pcl51
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluesse )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-250", 0x0000, 0x020000, CRC(f99f319b) SHA1(550ee8235c784de9d10c5e6810780e9bc135a788) )//ncl21
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessep )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-251", 0x0000, 0x020000, CRC(a9decf27) SHA1(507bbcfaf68a1f7e5e79e0ff5b43686974cf1d6c) )//pcl21
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessf )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-248", 0x0000, 0x020000, CRC(d620e3c7) SHA1(24a185906762acef44ee3662b4645d4ab21ae32e) )//ncl11
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessfp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-249", 0x0000, 0x020000, CRC(86611d7b) SHA1(d8fd94c69911da21b9db9e805b7b61f9b507e032) )//pcl11
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessg )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-246", 0x0000, 0x020000, CRC(9eabb468) SHA1(890e309a628150b629b7fdc89b868c56e9649ffe) )//ncl71 15
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessi )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-118", 0x0000, 0x020000, CRC(5f6ccfea) SHA1(18f4fcaee6a0e2eb5f4ccfde983a4e7b4e5cec6b) )//ncl21 ss10fo 10gbp
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessj )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-090", 0x0000, 0x020000, CRC(67f299a5) SHA1(fd4b15cfbf84a0966b6317c260e3c099ebaa1f2d) )//ncf 2.3 5gbp
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessk )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-082", 0x0000, 0x020000, CRC(ce9540dc) SHA1(e951afd45f95e9ea92bbb4f4f5c1854bde5edd8d) )//ncf 1.2
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessl )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-009", 0x0000, 0x020000, CRC(fa1ea087) SHA1(6dfa634daf78f51aa5c99080eb646ac029d73468) )//ncl41
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluesslp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-010", 0x0000, 0x020000, CRC(839bb97c) SHA1(4682e3e344a742bf72de828b1c9e1ce7e60de1e7) )//pcl41
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessm )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-007", 0x0000, 0x020000, CRC(758ef0be) SHA1(1ae1502d7d1d2fb532149d19d55023e4b7d64d3d) )//ncl31
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessmp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-008", 0x0000, 0x020000, CRC(342e51d5) SHA1(e9c4fcfc9a1c391b2c27b0fb049d424b86444060) )//pcl31
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessn )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-003", 0x0000, 0x020000, CRC(7b4b409b) SHA1(de8e05c6728b1af9cf080e43a4b22a22a5b06b62) )//ncl11 sslth10v
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluesso )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-745", 0x0000, 0x020000, CRC(d8e1a02b) SHA1(8e1244e60924d739ad67ebd04ab4de8ad5fb5929) )//ncl21
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessop )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-746", 0x0000, 0x020000, CRC(cb269b7d) SHA1(e8352e19215131b445a60721eb67c7245eff6ca7) )//pcl21
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessq )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-358", 0x0000, 0x020000, CRC(8550c577) SHA1(20d3a2fd0b4c172f7b521f795ec6e6644e16919e) )//ncl51
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessqp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-359", 0x0000, 0x020000, CRC(d5113bcb) SHA1(703459ab22f480637f20ce0837a2cfbd6481dc68) )//pcl51
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessr )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-356", 0x0000, 0x020000, CRC(079c1bb0) SHA1(1d0a5518ea9b480745e09e370244411ac932d499) )//ncl31
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluessrp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-357", 0x0000, 0x020000, CRC(57dde50c) SHA1(be31f42219213fbe27563f43c30571fb5887a92b) )//pcl31
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluesss )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-444", 0x0000, 0x020000, CRC(bb257c07) SHA1(eb402538fc11e759e54df814e59d2dd79ac895bc) )//ncl41
	m1_cluess_sound
ROM_END_M1A_MCU

ROM_START( m1cluesssp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-445", 0x0000, 0x020000, CRC(eb6482bb) SHA1(f6012913d79a22a69ed41beaf7bc506bae59fbcf) )//pcl41
	m1_cluess_sound
ROM_END_M1A_MCU













ROM_START( m1coderd )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc991130", 0x0000, 0x020000, CRC(d3add67f) SHA1(f7387978f18680921a2aff0296de2b9609f3215d) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc991131", 0x0000, 0x020000, CRC(d5b90a02) SHA1(18a5642175ee330832ce95ac2fcb3a662cfe4273) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "code_red.s1", 0x000000, 0x080000, CRC(616b939d) SHA1(1a94747efe430e508086fdb66da3f3e7daf7c4f3) )
	ROM_LOAD( "code_red.s2", 0x080000, 0x080000, CRC(bee44524) SHA1(dd3837559b375e1055e6aa6c984bfd88102a5825) )
ROM_END_M1A_MCU

ROM_START( m1coro )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cns58c", 0x0000, 0x010000, CRC(e254a369) SHA1(10c03b108f9fb9ffb16c4cf47fac74625d2e7877) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "cns58t", 0x0000, 0x010000, CRC(1b2c1306) SHA1(7fa7c0238685dc2a91354ae47674247e95707613) )
	ROM_LOAD( "coro2010", 0x0000, 0x010000, CRC(a7cfd3e9) SHA1(1b8fee6397d137cfae8bcd93b6a3b8e36b2716b8) )
	ROM_LOAD( "cst5p5bin", 0x0000, 0x010000, CRC(0ab4535b) SHA1(fc334b9da2736f8d57adb76095df8e957fb7667d) )
	ROM_LOAD( "sa4-450", 0x0000, 0x010000, CRC(355e10f4) SHA1(5de193ea05988e6eb1190cff357581671a87c82f) )
	ROM_LOAD( "sa4-451", 0x0000, 0x010000, CRC(ddb68d12) SHA1(b399504a8a17129b88e9f95ab1942c7448e5fbb3) )
	ROM_LOAD( "sa4-529", 0x0000, 0x010000, CRC(ca486f81) SHA1(001669f92d9d548854b3c2c9e9c5b7141c9d2b32) )
	ROM_LOAD( "sa4-530", 0x0000, 0x010000, CRC(4ade92e9) SHA1(2d5d18cfab8ff5a5f5790168375548a56b6903b0) )
	ROM_LOAD( "sa5-048", 0x0000, 0x010000, CRC(fbd5c2ae) SHA1(c05959664fcee7f3f05c6f81c1a98d2fc6b59141) )
	ROM_LOAD( "sa5-049", 0x0000, 0x010000, CRC(133d5f48) SHA1(b4f89e5cf1d4ef60f73be18d372c38b22126e651) )
	ROM_LOAD( "sa5-208.bin", 0x0000, 0x010000, CRC(66f06d18) SHA1(fd14b09280815c03126d113e6be791a20483aae9) )
	ROM_LOAD( "sa5-209", 0x0000, 0x010000, CRC(8e18f0fe) SHA1(8d6c0fbd05484dc42b976228b7575e0ca0eea239) )
	ROM_LOAD( "sa5-393", 0x0000, 0x010000, CRC(abbdfe46) SHA1(a6e005849b6da9801331155eb73fa169f6f42265) )
	ROM_LOAD( "sa5-394", 0x0000, 0x010000, CRC(a7a5a6ff) SHA1(0805d3f1148e0aa83f8c0c804c08df3624a3af72) )
	ROM_LOAD( "sa5-395", 0x0000, 0x010000, CRC(980a56ba) SHA1(372744f165e0105346c567013b0c8ddec2ec0f7d) )
	ROM_LOAD( "sa5-396", 0x0000, 0x010000, CRC(2cb28869) SHA1(0dc78252b8e56a4b18342e5d1583ed4a0ea1db38) )
	ROM_LOAD( "sa5-397", 0x0000, 0x010000, CRC(6b91fff5) SHA1(e29ae21bfd7d7e3139bfb65b60bd4bafcece5b24) )
	ROM_LOAD( "sa5-399", 0x0000, 0x010000, CRC(0e290f63) SHA1(a5c8f2b125836ca76a7707d09d8e4a2f3058a9df) )
	ROM_LOAD( "sa6-192", 0x0000, 0x010000, CRC(5a4b0f17) SHA1(04ae2db3a29485672faaedd22c5780dd71176c96) )
	ROM_LOAD( "sa6-193", 0x0000, 0x010000, CRC(b2a392f1) SHA1(d7b908373eb3e225e399c36847ef2481ea3ad65a) )

	ROM_LOAD( "cns5.10", 0x0000, 0x010000, CRC(91dc8625) SHA1(525c4311355bb3aa3052160bc439c49f5cecbcc6) )
	ROM_LOAD( "cnst5.5v2", 0x0000, 0x010000, CRC(a5b2589e) SHA1(cabd5abf996c1bb9ca7a0ffcc5d666aa632f6789) )
	ROM_LOAD( "sa5-306", 0x0000, 0x010000, CRC(a94a8f48) SHA1(56d5723d8087f523061280d9afe15a1bf8b269f5) )
	ROM_LOAD( "sa5-307", 0x0000, 0x010000, CRC(41a212ae) SHA1(463a7d93f3b8b073cb83bda2a957dab37070ba8b) )
	ROM_LOAD( "sa5-447", 0x0000, 0x010000, CRC(5b292c6e) SHA1(584a5d64ee47054870785222f85a58a721165530) )
	ROM_LOAD( "sa5-448", 0x0000, 0x010000, CRC(e0e481c3) SHA1(062b8f58a7d9fa9163fd98892a6cc8099d4d234c) )
	ROM_LOAD( "sa6-049", 0x0000, 0x010000, CRC(dfa086ae) SHA1(c5fa5d435a603851ab6bbe5860ee38e41aea814e) )

	// BWB Set
	ROM_LOAD( "cs______.1_1", 0x0000, 0x010000, CRC(5b292c6e) SHA1(584a5d64ee47054870785222f85a58a721165530) )


	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "digi0421.bin", 0x000000, 0x080000, CRC(9489c9cd) SHA1(1a858b2a6f41898bbf95611e9f13d668c8a05c9c) )
	ROM_LOAD( "digi0422.bin", 0x080000, 0x080000, CRC(cf17088e) SHA1(54c9a52ccdd1ca622367367e1304fe4e4037b0b9) )
ROM_END_M1A_MCU

ROM_START( m1cororr )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "rov5.10", 0x0000, 0x010000, CRC(04b421ca) SHA1(c0992edf4ecdfcf7231ae560f38954ce3a4db735) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "rov5.8", 0x0000, 0x010000, CRC(79be1896) SHA1(78a3f62fcbb2073257eeebc03524852d0c9d4648) )
	ROM_LOAD( "rover8ac", 0x0000, 0x010000, CRC(1f6bff96) SHA1(862e32dc9ea3fad5ef27b9146a8a62138d3b6406) )
	ROM_LOAD( "roversprog", 0x0000, 0x010000, CRC(35aded03) SHA1(978c49dad02cd1bb290028aa52d3048c5f2b9bdd) )
	ROM_LOAD( "roversreturn20p8ptokenv2.1", 0x0000, 0x010000, CRC(ee5020fa) SHA1(926c217d345a62b2b5073f35463ba67cf03b068b) )
	ROM_LOAD( "rovr208a", 0x0000, 0x010000, CRC(ee5020fa) SHA1(926c217d345a62b2b5073f35463ba67cf03b068b) )
	ROM_LOAD( "rovr5.10", 0x0000, 0x010000, CRC(2de4c3a0) SHA1(3342d3a6ca7a6f20aa0e094f64f757f1dcc43fa9) )
	ROM_LOAD( "rovr5.8", 0x0000, 0x010000, CRC(ff6bd9fe) SHA1(39e6ba2ee37ea029d307456b3e254e26d34697f3) )
	ROM_LOAD( "rret5p5c", 0x0000, 0x010000, CRC(ce590a5a) SHA1(f9b93fa830c7d49b52c327a7cf7fa98b357ea695) )
	ROM_LOAD( "rret5p8c", 0x0000, 0x010000, CRC(7183b915) SHA1(dd0bc6668013774e429d6fcaa82d518232c08ada) )
	ROM_LOAD( "rret5p8cbin", 0x0000, 0x010000, CRC(3fd51c76) SHA1(dd466b34277611dc8e61d182dbf35be2f4771ce3) )
	ROM_LOAD( "rret665p", 0x0000, 0x010000, CRC(0656ad1b) SHA1(d1efb0cde9354087815ea260ccc81152c1ccf354) )
	ROM_LOAD( "sa5-342", 0x0000, 0x010000, CRC(735e8151) SHA1(25b9f183e03c74fd918c9c540e5ebd0dc0d38fcd) )
	ROM_LOAD( "sa5-343", 0x0000, 0x010000, CRC(9bb61cb7) SHA1(0c42d7db308dac80a4910b2d2327833562e9c887) )
	ROM_LOAD( "sa5-344", 0x0000, 0x010000, CRC(ee5020fa) SHA1(926c217d345a62b2b5073f35463ba67cf03b068b) )
	ROM_LOAD( "sa5-345", 0x0000, 0x010000, CRC(06b8bd1c) SHA1(52c43b7f2774accdde4c153c84b5051df84cbe29) )
	ROM_LOAD( "sa5-354", 0x0000, 0x010000, CRC(132d0aec) SHA1(fd7febd1b7098a6a3b00fa5ed5f0323821fea9da) )
	ROM_LOAD( "sa5-355", 0x0000, 0x010000, CRC(fbc5970a) SHA1(41cf94e2ab0dc3d020cf30cf63c8939958e7805a) )
	ROM_LOAD( "sa5-539", 0x0000, 0x010000, CRC(8ba27a8e) SHA1(27ec7503d84585bbb791f6b4ee1ef538dcd5f619) )
	ROM_LOAD( "sa6-203", 0x0000, 0x010000, CRC(dd4570e5) SHA1(16f1530d68dcd043f67084c339b02f093d45c6cb) )
	ROM_LOAD( "sa6-252", 0x0000, 0x010000, CRC(31d81c51) SHA1(65d5578c7837499d0bfdacbe95400adff00cd24c) )
	ROM_LOAD( "sa6-253", 0x0000, 0x010000, CRC(d93081b7) SHA1(634fe0a75be7d4a175f11da6a6c045e215a8c139) )
	ROM_LOAD( "sa7-176", 0x0000, 0x010000, CRC(fd89552f) SHA1(cdb38e6388ada9a893dfc4971d2c2c2898b755a7) )
	ROM_LOAD( "sa7-177", 0x0000, 0x010000, CRC(1561c8c9) SHA1(8eea26e72f96413a6bff6b14e6acc4d311c1ea72) )
	ROM_LOAD( "sa8-062", 0x0000, 0x010000, CRC(5eebd57e) SHA1(e4b4de1388f28e0819baca4ba9c96573c367a4a1) )
	ROM_LOAD( "sa8-063", 0x0000, 0x010000, CRC(b6034898) SHA1(167f37456b9f3ffcc10adc910a5003b044473634) )
	ROM_LOAD( "sa8-066", 0x0000, 0x010000, CRC(0656ad1b) SHA1(d1efb0cde9354087815ea260ccc81152c1ccf354) )
	ROM_LOAD( "sa8-067", 0x0000, 0x010000, CRC(eebe30fd) SHA1(adc278973a08a81c4a62176e6ec33af570d719ac) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "roversreturn.p1", 0x000000, 0x080000, CRC(b21d4cca) SHA1(ad54c4f44de2c596fd5e8330666d0f4f859bfcb2) )
	ROM_LOAD( "roversreturn.p2", 0x080000, 0x080000, CRC(354a91e0) SHA1(13f3e1eacba3c80c83f12491d2668cc54536245a) )
ROM_END_M1A_MCU

ROM_START( m1corocb )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ccoro250v1-1.rom", 0x0000, 0x020000, CRC(e2188b21) SHA1(86238d31595814d9d1f82544c9766d068b6df132) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc7-197", 0x0000, 0x020000, CRC(b259759d) SHA1(cd84d959a4bcfcd942322af9f33893e626fe8759) )
	ROM_LOAD( "sc7-198", 0x0000, 0x020000, CRC(905b916d) SHA1(4c244ee49b4528e05cb074f0df0b3bbfd6b28fd2) )
	ROM_LOAD( "sc7-199", 0x0000, 0x020000, CRC(c01a6fd1) SHA1(2bb62f190843acb8850241ccd45fb17167c18376) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "clubcorriesnd.p1snd", 0x000000, 0x080000, CRC(e4cf4412) SHA1(6849fb9a71a6f0bbf40368238ed9104026013d36) )
	ROM_LOAD( "clubcorriesnd.p2snd", 0x080000, 0x080000, CRC(e33d2c08) SHA1(fd30b9c2936659a793d83e283d920c46990633c4) )
ROM_END_M1A_MCU

ROM_START( m1corosh )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-342", 0x0000, 0x010000, CRC(47ac83cf) SHA1(d23e14a714121bb67c130aae4b85bdcf62a949b6) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-343", 0x0000, 0x010000, CRC(af441e29) SHA1(6631d5282f896c9a7fe1b2e41c19d58dfef4e644) )
ROM_END_M1A_MCU

ROM_START( m1criss )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "crisscross.bin", 0x0000, 0x010000, CRC(422c5c6d) SHA1(b3a86f7482f0376b93899d28d4e6c610200fcd3a) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "ccsound.bin", 0x0000, 0x040000, CRC(8742981e) SHA1(1ba33c59ec5f878ebab111a77551213aad4b0993) )
ROM_END_M1A_MCU

ROM_START( m1dxmono )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "delmonopolygame.bin", 0x0000, 0x010000, CRC(b15da041) SHA1(caf85c80fc6128c8c28bdb9ea3e37308a15279de) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "deluxe_monopoly_v1_1", 0x0000, 0x010000, CRC(303bea68) SHA1(987f015818a1d97299af824c0ab58c9c6f8a2acb) )
	ROM_LOAD( "mondx5p5bin", 0x0000, 0x010000, CRC(53d44098) SHA1(45ae90464e2f9d4a2c7dc846acbc8f48449b3dab) )
	ROM_LOAD( "monodx8", 0x0000, 0x010000, CRC(4d6101a3) SHA1(86f9bde8aec67566b7b58fec8f01f09878773dc1) )
	ROM_LOAD( "sa3-032", 0x0000, 0x010000, CRC(8fe139f6) SHA1(0a7f78d284706199993a5f41adcf70bc50faa433) )
	ROM_LOAD( "sa4-098.bin", 0x0000, 0x010000, CRC(1775f26d) SHA1(89a96955ca4f345b0451c652939e93c93629c0d4) )
	ROM_LOAD( "sa4-261.bin", 0x0000, 0x010000, CRC(10854aab) SHA1(1b61d435814aa2f0eba565a5ac095e47f61bd0b2) )
	ROM_LOAD( "sa4-376.bin", 0x0000, 0x010000, CRC(44a94d58) SHA1(194e004dd68125f0c5bd83ea467cc1ebb5e616d6) )
	ROM_LOAD( "sa4-377.bin", 0x0000, 0x010000, CRC(ac41d0be) SHA1(bf049f247c273f9c09b02157a2e1d2af39ba612b) )
	ROM_LOAD( "sa5-078", 0x0000, 0x010000, CRC(f8eeb449) SHA1(820b075c931918a86c06946a7a6ce0b6db2c44b2) )
	ROM_LOAD( "sa6-015", 0x0000, 0x010000, CRC(fc7a30aa) SHA1(9133894464ba6bfb64996edb5bd99d88e34340c4) )
	ROM_LOAD( "sa6-018", 0x0000, 0x010000, CRC(134e772a) SHA1(e85a90ed475cd3b38e9174146b15c66c958116e5) )
	ROM_LOAD( "sa6-019", 0x0000, 0x010000, CRC(fba6eacc) SHA1(80576722f5862c1f27eb6e6d43d9a0a665e611c6) )

	// BWB Sets
	ROM_LOAD( "dm_x6_b_.1o1", 0x0000, 0x010000, CRC(77824728) SHA1(b836a6a2f42d2d8e46cc532db5b4052e16dafa87) )
	ROM_LOAD( "dm_x6_d_.1o1", 0x0000, 0x010000, CRC(05d4c471) SHA1(b5a26ff4824dd68de629911a60705375748cba1c) )
	ROM_LOAD( "dm_x6_k_.1o1", 0x0000, 0x010000, CRC(9f6adace) SHA1(958c710736d661756a1f25ba3d51554c2f05e4fc) )
	ROM_LOAD( "dmix6___.1o1", 0x0000, 0x010000, CRC(82f472a4) SHA1(c5cda8f7ae6fe69e1cd4044b3816da665b6ba9bc) )
	ROM_LOAD( "dmix6_d_.1o1", 0x0000, 0x010000, CRC(6a1cef42) SHA1(e9178fbcf29ff565d0672b550a92516c92ebfaa4) )
	ROM_LOAD( "mdlx10", 0x0000, 0x010000, CRC(e87ff39a) SHA1(4a5bc1d094b18cec55b7ed8291db68b73ee860ae) )
	ROM_LOAD( "monodeluxesa2-356 nd.bin", 0x0000, 0x010000, CRC(3d4394e9) SHA1(43da2007f1408d45764f0eba8594c800ad7ffc60) )

	ROM_LOAD( "sa4-196", 0x0000, 0x010000, CRC(9f6adace) SHA1(958c710736d661756a1f25ba3d51554c2f05e4fc) )
	ROM_LOAD( "sa4-197", 0x0000, 0x010000, CRC(77824728) SHA1(b836a6a2f42d2d8e46cc532db5b4052e16dafa87) )
	ROM_LOAD( "sa4-199", 0x0000, 0x010000, CRC(05d4c471) SHA1(b5a26ff4824dd68de629911a60705375748cba1c) )
	ROM_LOAD( "sa5-080", 0x0000, 0x010000, CRC(82f472a4) SHA1(c5cda8f7ae6fe69e1cd4044b3816da665b6ba9bc) )


	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "delmonopolysound.bin", 0x0000, 0x040000, CRC(8742981e) SHA1(1ba33c59ec5f878ebab111a77551213aad4b0993) )
ROM_END_M1A_MCU

ROM_START( m1dmnhrt )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "diamond.bin", 0x0000, 0x010000, CRC(d63a92c2) SHA1(66fe356662c353b2cca3831f7b55d0aea740aace) )
ROM_END_M1A_MCU

ROM_START( m1dm4ev )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "dafor200", 0x0000, 0x010000, CRC(0a1a3906) SHA1(bb16251bdf4726799218bf252b47184d999f97dc) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc4-085.bin", 0x0000, 0x010000, CRC(80cc889e) SHA1(464f08523754454e97e00108edc28a4accef204a) )
	ROM_LOAD( "sc4-086.bin", 0x0000, 0x010000, CRC(7eccadaa) SHA1(9d3d3f990960cf57eac033786826b046e15d594e) )
ROM_END_M1A_MCU

ROM_START( m1dkong )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "dko6615r", 0x0000, 0x020000, CRC(63768b11) SHA1(b7b0be4efcf61fb4c3c20c98844cadf879eba871) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "dkon510", 0x0000, 0x020000, CRC(56338a54) SHA1(b066bdf7f18793936790211c6d1eecb23391e63f) )
	ROM_LOAD( "dkon55", 0x0000, 0x020000, CRC(37305db5) SHA1(977d960931151b3c11a191c4661ee374e4f2dc45) )
	ROM_LOAD( "dkong.bin", 0x0000, 0x020000, CRC(c6a63fe4) SHA1(090fa72ce5329b0bf1aec3bdf6cf1abc8298b3dd) )
	ROM_LOAD( "dkong10", 0x0000, 0x020000, CRC(d4793c7f) SHA1(677b1f1065be15ec4b431a67138358830c687549) )
	ROM_LOAD( "dkong15.bin", 0x0000, 0x020000, CRC(c6a63fe4) SHA1(090fa72ce5329b0bf1aec3bdf6cf1abc8298b3dd) )
	ROM_LOAD( "dkong5.bin", 0x0000, 0x020000, CRC(cc3a66e6) SHA1(77e5cd98cb060e7730a66e35023d051ed606ed03) )
	ROM_LOAD( "dkong8.bin", 0x0000, 0x020000, CRC(023a660a) SHA1(a64c6d415ef1990d55abdc3cad8af81d3dac8369) )
	ROM_LOAD( "donk10fo", 0x0000, 0x020000, CRC(39fa98ea) SHA1(8405fa612c1e2bd1c8df260737072f46931cc303) )
	ROM_LOAD( "kong15f", 0x0000, 0x020000, CRC(367ab43f) SHA1(6940e6922d32126d67c2b0c47282e4bf42ebaf04) )
	ROM_LOAD( "kong5p8.bin", 0x0000, 0x020000, CRC(14380d4c) SHA1(3e66809af45f216489a5c65930726be0f5a6c555) )
	ROM_LOAD( "sa001014", 0x0000, 0x020000, CRC(1c6db3b3) SHA1(2ae8797d1794358bde6dca296a921d0a96277531) )
	ROM_LOAD( "sa001015", 0x0000, 0x020000, CRC(1a796fce) SHA1(163d58a3258309f5183b575b9e651f1c2f53ce36) )
	ROM_LOAD( "sa7-109", 0x0000, 0x020000, CRC(06aec0d7) SHA1(aa1dd411aa43ecf0908cb9db64636de319041159) )
	ROM_LOAD( "sa7-110", 0x0000, 0x020000, CRC(7f2bd92c) SHA1(d0f41d63db4e71ee4a7cc2ea878add0c72b1c7bb) )
	ROM_LOAD( "sa7-115", 0x0000, 0x020000, CRC(09efe6ed) SHA1(120615bb9b69386b5fffdf8756de16415b8ce778) )
	ROM_LOAD( "sa7-116", 0x0000, 0x020000, CRC(1a28ddbb) SHA1(e28297885c1321cff5bf87cbf9d98f3b6ae005d6) )
	ROM_LOAD( "sa7-284", 0x0000, 0x020000, CRC(d4793c7f) SHA1(677b1f1065be15ec4b431a67138358830c687549) )
	ROM_LOAD( "sa7-285", 0x0000, 0x020000, CRC(8438c2c3) SHA1(4fce8a96def574b7a44f45163b2141eb5629a5f1) )
	ROM_LOAD( "sa7-288", 0x0000, 0x020000, CRC(adc1603a) SHA1(6f34234dbfb8e042819c5ea4a11be7029949fa96) )
	ROM_LOAD( "sa7-289", 0x0000, 0x020000, CRC(fd809e86) SHA1(e41cef2cc6b3eee39ac3344b1573822c6f681c8f) )
	ROM_LOAD( "sa7-293", 0x0000, 0x020000, CRC(527b98b6) SHA1(4d3c317a1719f7efa2825da60b75e1beed698ecf) )
	ROM_LOAD( "sa7-731", 0x0000, 0x020000, CRC(39fa98ea) SHA1(8405fa612c1e2bd1c8df260737072f46931cc303) )
	ROM_LOAD( "sa7-732", 0x0000, 0x020000, CRC(2a3da3bc) SHA1(7a56e4e3bd12e900ed0f36241891b73f1981586a) )
	ROM_LOAD( "sa8-052", 0x0000, 0x020000, CRC(00671257) SHA1(e977b764dbaeb519f6d3174f786ba75628733bf8) )
	ROM_LOAD( "sa8-053", 0x0000, 0x020000, CRC(5026eceb) SHA1(7c444c380ee888e43e0f95577f09a1d949f1e010) )
	ROM_LOAD( "sa8-068", 0x0000, 0x020000, CRC(a5bfa528) SHA1(3b7619af8b4908986c15b777c953f34792126c31) )
	ROM_LOAD( "sa8-069", 0x0000, 0x020000, CRC(f5fe5b94) SHA1(49d3ae41c013140dbae32b9a4ab62202a39fd761) )
	ROM_LOAD( "sa8-112", 0x0000, 0x020000, CRC(658ba678) SHA1(2b95d2fcb14f6d10adf2db075b2598c262994fe1) )
	ROM_LOAD( "sa991072", 0x0000, 0x020000, CRC(2f9e63f1) SHA1(e912bee785f4b3d5b1894a42dbceadded34b0776) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "donkeykong.p1", 0x0000, 0x080000, CRC(11019875) SHA1(b171b46a7a98967668793a7ea7b5931c7a76dd82) )
	ROM_LOAD( "donkeykong.p2", 0x0000, 0x080000, CRC(e28f406f) SHA1(42a58c0f5c4f25dec4c0c49eb8415971a515c5a6) )
ROM_END_M1A_MCU

ROM_START( m1ee )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ea_x6__d.2o1", 0x0000, 0x010000, CRC(1e0a490c) SHA1(f0fa5887431915dc8395b5b353a1b04f6a8abe77) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "ea_x6__s.2o1", 0x0000, 0x010000, CRC(ff01a97d) SHA1(1a1f2626d1098380a5635c79d2bdb896430e016d) )
	ROM_LOAD( "ea_x6_dd.2o1", 0x0000, 0x010000, CRC(aab297df) SHA1(57bbf04c09146183b9f3d7bd5a9126e549a7e877) )
	ROM_LOAD( "ea_x6_ds.2o1", 0x0000, 0x010000, CRC(5929fb51) SHA1(ce1d6ee01d4647487e30d7ed49f3f2a14705cee3) )
ROM_END_M1A_MCU

ROM_START( m1eea )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-635", 0x0000, 0x010000, CRC(326a06cd) SHA1(4d85af4ac25660d36c661f578525063891908e8e) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-637", 0x0000, 0x010000, CRC(0ea202ce) SHA1(66357d73f1105178e072aea2e41ebaacf20a4de5) )
	ROM_LOAD( "sa4-638", 0x0000, 0x010000, CRC(0ea202ce) SHA1(66357d73f1105178e072aea2e41ebaacf20a4de5) )
	ROM_LOAD( "sa5-092", 0x0000, 0x010000, CRC(a435c1a0) SHA1(50d48d95532b5e907dbce9ad9341988bc8ef7989) )
	ROM_LOAD( "sa5-179", 0x0000, 0x010000, CRC(367cb048) SHA1(7bfe313e43615fdd6ea0539191a735b3b9e51e76) )
	ROM_LOAD( "sa5-180", 0x0000, 0x010000, CRC(f21a7424) SHA1(a4c294f1a1616ef89e9e65b0c477aaa9ddeabe92) )
	ROM_LOAD( "sa5-181", 0x0000, 0x010000, CRC(564e75c8) SHA1(60193cf125d241e858036d6305bedf5a1721ce90) )
	ROM_LOAD( "sa5-182", 0x0000, 0x010000, CRC(e2f6ab1b) SHA1(ac8171429934bc7b9c8652ac58a1c77fcfecafbb) )
	ROM_LOAD( "sa5-515", 0x0000, 0x010000, CRC(1e0a490c) SHA1(f0fa5887431915dc8395b5b353a1b04f6a8abe77) )
	ROM_LOAD( "sa5-517", 0x0000, 0x010000, CRC(ff01a97d) SHA1(1a1f2626d1098380a5635c79d2bdb896430e016d) )
	ROM_LOAD( "sa5-555", 0x0000, 0x010000, CRC(1e0a490c) SHA1(f0fa5887431915dc8395b5b353a1b04f6a8abe77) )
ROM_END_M1A_MCU

ROM_START( m1eeb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "easte-8a.p1", 0x0000, 0x010000, CRC(3d099816) SHA1(594b705a81e3897ca8aa340351475489a012c2ae) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "eastend.bin", 0x0000, 0x010000, CRC(a17ded63) SHA1(2b0d8deb30c96eba2cec6bc910a5175606c96593) )
	ROM_LOAD( "eastenders special 4_8 - 2 bars pays 4.bin", 0x0000, 0x010000, CRC(ffab9325) SHA1(be04106cd049ede8af28f77bd2d88e0545d98e48) )
	ROM_LOAD( "eastenders-sa5-515.bin", 0x0000, 0x010000, CRC(ffab9325) SHA1(be04106cd049ede8af28f77bd2d88e0545d98e48) )
	ROM_LOAD( "eastenders8tok4cash.bin", 0x0000, 0x010000, CRC(3d099816) SHA1(594b705a81e3897ca8aa340351475489a012c2ae) )
	ROM_LOAD( "ee58c", 0x0000, 0x010000, CRC(b8ac7013) SHA1(8a79c8594116eb89b668fa0359e1bffb1246b111) )
	ROM_LOAD( "eend2010", 0x0000, 0x010000, CRC(7d74a544) SHA1(ee2e92c19601cb77b11fa793b76b1b937e50a717) )
	ROM_LOAD( "eend208c", 0x0000, 0x004000, CRC(f6c59222) SHA1(46438d9369c9fe870176cc88b63e34fddca47b8d) ) // bad dump, or rom overlay?
	ROM_LOAD( "eend20a", 0x0000, 0x010000, CRC(8d30542e) SHA1(fd95f78efac76a496a10129b704176c3c30d28bb) )
	ROM_LOAD( "eend58c", 0x0000, 0x010000, CRC(fcd56799) SHA1(8346c5688b91b81d62ff06ee1a4fd8bc59783578) )
	ROM_LOAD( "eend58t", 0x0000, 0x010000, CRC(f69622a5) SHA1(217e64f0929541eb2b841d77e9cadc2df8786c03) )
	ROM_LOAD( "eend5p", 0x0000, 0x010000, CRC(9f5db9d7) SHA1(fc064e8a69585ac8e8a94e5c4570098365107ec6) )
	ROM_LOAD( "eend8", 0x0000, 0x010000, CRC(2ff2aade) SHA1(1a4a92664ee0ea972e99d5824fd516598740989e) )
	ROM_LOAD( "end510p", 0x0000, 0x010000, CRC(7fa9ad28) SHA1(219b3ab89d2708ecd75ef78ea3b9af6e6cadf81a) )
	ROM_LOAD( "end5p35bin", 0x0000, 0x010000, CRC(b8c4ae48) SHA1(f3cb6b132ba77f9bf9ebd02081aca69bca15b7e1) )
	ROM_LOAD( "end5p45bin", 0x0000, 0x010000, CRC(ce27e982) SHA1(a7b40d603c56927a3fec304e4010d42d731d1d70) )
	ROM_LOAD( "sa4-039.bin", 0x0000, 0x010000, CRC(b9eb7d8d) SHA1(1bb89319585bc3dfc2ed43eb68c9490c407ebb0b) )
	ROM_LOAD( "sa4-040.bin", 0x0000, 0x010000, CRC(d94bf67c) SHA1(d4aff083455608afe7458213723de600bbf698d3) )
	ROM_LOAD( "sa4-154.bin", 0x0000, 0x010000, CRC(bb489619) SHA1(5d7a86ffdab60b0541722af7ecc0f17cc1964dcc) )
	ROM_LOAD( "sa4-155.bin", 0x0000, 0x010000, CRC(53a00bff) SHA1(d1f06495594000da50057d68d63b39d47b24a4db) )
	ROM_LOAD( "sa4-188.bin", 0x0000, 0x010000, CRC(aaa7623d) SHA1(302a7e0dff8473b7d8b12a6cbf702d585ba7465c) )
	ROM_LOAD( "sa4-189.bin", 0x0000, 0x010000, CRC(424fffdb) SHA1(a0320d4c4bbda2534b3f7cb1a461d5567b565b6c) )
	ROM_LOAD( "sa4-388", 0x0000, 0x010000, CRC(3a094e03) SHA1(0e436abe3c533aebbc9bcf7469bb03db8ab8fc3d) )
	ROM_LOAD( "sa4-389", 0x0000, 0x010000, CRC(d2e1d3e5) SHA1(3038145c919c3f3f93b8095d583443d2a13ed0dd) )
	ROM_LOAD( "sa5-054", 0x0000, 0x010000, CRC(bb2a4008) SHA1(85c8e666b0726c6fb9a7531fbe1f5eeb0e00fc61) )
	ROM_LOAD( "sa5-055", 0x0000, 0x010000, CRC(53c2ddee) SHA1(13937da8766d394158d8ed559d1b17a412a60985) )
	ROM_LOAD( "sa5-483", 0x0000, 0x010000, CRC(aac97b39) SHA1(f8b2898dab5b07013b2b971ab9200d5c2fdf68be) )
	ROM_LOAD( "sa5-484", 0x0000, 0x010000, CRC(a6d12380) SHA1(bb89d2d12680a65cb11b162a3b5b1102d5b5a875) )
	ROM_LOAD( "sa5-485", 0x0000, 0x010000, CRC(8e102800) SHA1(c8e7d3aecdef9eabcd9ba0d63dae379415dbfd92) )
	ROM_LOAD( "sa5-486", 0x0000, 0x010000, CRC(3aa8f6d3) SHA1(82a492377bffdefb6952f65716773867363991b3) )
	ROM_LOAD( "sa5-487", 0x0000, 0x010000, CRC(27cc4abf) SHA1(cea422fd93c550552e471fb3ed7b97cdba83db02) )
	ROM_LOAD( "sa6-274", 0x0000, 0x010000, CRC(ad65b5da) SHA1(53a6ae9c0d70ee7f40f5c51cfc6d8a78ff47fe94) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "ee-snd1.bin", 0x000000, 0x080000, CRC(3eaa138c) SHA1(ad7d6e3ffc8fe19ea8cb9188998c75c90a77e09e) )
	ROM_LOAD( "ee-snd2.bin", 0x080000, 0x080000, CRC(89fde428) SHA1(f0942a2f1d3890ad18b01e8433333e5412c57644) )
ROM_END_M1A_MCU

ROM_START( m1eeqv )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "whitbread", 0x0000, 0x010000, CRC(6cfa52cc) SHA1(8a75cb58cd808a5f7556fa00c4eefa9181a91df5) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "queenvic.p1", 0x000000, 0x080000, CRC(af665967) SHA1(b87b13e759765eeb701ff8ead41eb3c09bec1e92) )
	ROM_LOAD( "queenvic.p2", 0x080000, 0x080000, CRC(92ce90ce) SHA1(9fca67429b9f1496dc745818b3ed7747b8eedf5d) )
ROM_END_M1A_MCU

ROM_START( m1eeqva )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "qv10a", 0x0000, 0x020000, CRC(efe47e9b) SHA1(83ce75026a1194e645a03016263f091be1ced437) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "qvgame", 0x0000, 0x010000, CRC(ab32aae7) SHA1(7167b90a2581b66ee8c50142d0d9d779c3f487a3) )
	ROM_LOAD( "qvic10", 0x0000, 0x010000, CRC(fcde6317) SHA1(51b18bd49cec1a91bb724bdd902466103630f3fe) )
	ROM_LOAD( "qvic5.8", 0x0000, 0x010000, CRC(14705d7d) SHA1(37d2d2f0c99ed89d8f8c1d57d2d1bcef9972d74c) )
	ROM_LOAD( "qvic510", 0x0000, 0x010000, CRC(57da1e08) SHA1(7c32960488409fcbbc92c0f1229027baf0fe68d8) )
	ROM_LOAD( "qvic58", 0x0000, 0x010000, CRC(95f9ad68) SHA1(c706462dc16b7e0d6a4128d94b80f6e5a1fb0923) )
	ROM_LOAD( "qvic5p8c.bin", 0x0000, 0x010000, CRC(84cd625b) SHA1(f47794b399247adcc80a515445dacf04c2e95018) )
	ROM_LOAD( "qvic8ac", 0x0000, 0x010000, CRC(a539e85e) SHA1(1dec987925450348321c70d1d6928b3882859ddf) )
	ROM_LOAD( "qvicv3", 0x0000, 0x010000, CRC(54a483be) SHA1(1956ee40dc0bade4f052b2043b1d1708d1c2c0a5) )
	ROM_LOAD( "sa6-173", 0x0000, 0x010000, CRC(cd7b3f3e) SHA1(4c81b167a7ce2d91d5d97725e51347bdf3c9581d) )
	ROM_LOAD( "sa6-174", 0x0000, 0x010000, CRC(c1636787) SHA1(4d1695d42541241de85d8a60b98150d60ced7a2b) )
	ROM_LOAD( "sa6-221", 0x0000, 0x010000, CRC(fcde6317) SHA1(51b18bd49cec1a91bb724bdd902466103630f3fe) )
	ROM_LOAD( "sa6-222", 0x0000, 0x010000, CRC(4866bdc4) SHA1(dc5548a1f9738e8ad06cf8911b727849fc877760) )
	ROM_LOAD( "sa6-235", 0x0000, 0x010000, CRC(81c7f044) SHA1(84501b900cbe6491101e9ae4ac176333deebcf41) )
	ROM_LOAD( "sa6-236", 0x0000, 0x010000, CRC(357f2e97) SHA1(e1eadc18717b072c2e0fdd169df1531396248d05) )
	ROM_LOAD( "sa6-311", 0x0000, 0x020000, CRC(225461cb) SHA1(3d6398f3226e54dcfed1b670aeece82eec4f424d) )
	ROM_LOAD( "sa6-312", 0x0000, 0x020000, CRC(31935a9d) SHA1(74423332183927ebec40073bfb65e162da2b6f6d) )
	ROM_LOAD( "sa6-385", 0x0000, 0x010000, CRC(a539e85e) SHA1(1dec987925450348321c70d1d6928b3882859ddf) )
	ROM_LOAD( "sa6-562", 0x0000, 0x020000, CRC(c6bf45de) SHA1(2ff6c5f98b3a0eca51ff218b95bda6e8ecda7ef8) )
	ROM_LOAD( "sa6-563", 0x0000, 0x020000, CRC(96febb62) SHA1(40fef32664cd1d531294cf801063b54f32514832) )
	ROM_LOAD( "sa7-496", 0x0000, 0x020000, CRC(efe47e9b) SHA1(83ce75026a1194e645a03016263f091be1ced437) )
	ROM_LOAD( "sa7-497", 0x0000, 0x020000, CRC(bfa58027) SHA1(12f8defa0cfa6e04e2a96c1669e6a0bd8874819f) )
	ROM_LOAD( "sa7-558", 0x0000, 0x010000, CRC(cad200e0) SHA1(5225181c6f42a3663266575fdf3fd1ca225988de) )
	ROM_LOAD( "sa7-605", 0x0000, 0x020000, CRC(79736157) SHA1(dc58fbf566b4d383b4e92d3d2680c11f99094b92) )
	ROM_LOAD( "sa7-606", 0x0000, 0x020000, CRC(6ab45a01) SHA1(16d1c4da1aae50ef89e6e92f3dc242c1de293e7f) )
	ROM_LOAD( "sa7-635", 0x0000, 0x010000, CRC(fff80770) SHA1(b9c935f540278c0945cbac6a7a2fe16e897898b3) )
	ROM_LOAD( "sa7-647", 0x0000, 0x010000, CRC(80b39331) SHA1(53dab262d9a605eff917fe8c3d4e4889a86171ee) )
	ROM_LOAD( "sa7-716", 0x0000, 0x010000, CRC(29e2064a) SHA1(a5a1b319e273c1245a8a5c9a01ed2a7f63b5b958) )
	ROM_LOAD( "sa7-717", 0x0000, 0x010000, CRC(c10a9bac) SHA1(8594ee99a51f7b7c4be1c716f0976ab8840c4ad0) )
	ROM_LOAD( "sa8-070", 0x0000, 0x010000, CRC(1c4961d7) SHA1(88e0d7a1fe1ff1f9ff24da3c13153143d67cca97) )
	ROM_LOAD( "sa8-071", 0x0000, 0x010000, CRC(f4a1fc31) SHA1(d3a636db2eb7669d457b02cc32910540a329f553) )
	ROM_LOAD( "sa8-316", 0x0000, 0x020000, CRC(551190ee) SHA1(89d21cb5f73ceb5531432430b61b3b229021d316) )
	ROM_LOAD( "sa8-317", 0x0000, 0x020000, CRC(05506e52) SHA1(a50abbfe7c3422f0b75e117fe08ac9b235a15128) )
	ROM_LOAD( "sa8-319", 0x0000, 0x010000, CRC(43da3701) SHA1(f1227fa360a4055dd3786d30b4e10d683f46729d) )
	ROM_LOAD( "vic15f", 0x0000, 0x020000, CRC(01fb6767) SHA1(83885fed9f7272c49b581fdce1720a6464dfc9c7) )
	ROM_LOAD( "vic15r", 0x0000, 0x010000, CRC(2659aad7) SHA1(456cfbc6ac7924871756138d1fd3353c7a236191) )
	ROM_LOAD( "15qvro", 0x0000, 0x020000, CRC(fb5da3dd) SHA1(9043276c782e4e2eb51366cbb6fb98ead5c02c67) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "queenvic.p1", 0x000000, 0x080000, CRC(af665967) SHA1(b87b13e759765eeb701ff8ead41eb3c09bec1e92) )
	ROM_LOAD( "queenvic.p2", 0x080000, 0x080000, CRC(92ce90ce) SHA1(9fca67429b9f1496dc745818b3ed7747b8eedf5d) )
ROM_END_M1A_MCU

ROM_START( m1expclb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cexplsc0", 0x0000, 0x010000, CRC(27d11ba5) SHA1(f640a4902213997df0b612e8d1be48ac6d1e0569) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc3-175", 0x0000, 0x010000, CRC(1b3bb880) SHA1(cce01c7fcc55eb012f5cffd43f8e8aa78e6379ea) )
	ROM_LOAD( "sc3-176", 0x0000, 0x010000, CRC(af836653) SHA1(4c97402e43f80040edaa93cf0630b36a9b9ca12e) )
ROM_END_M1A_MCU

ROM_START( m1fantfb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ffoot10", 0x0000, 0x010000, CRC(05641cb9) SHA1(52ca66c9bf325b080477b6a09b6ee964f2aca6e3) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "ffoot201", 0x0000, 0x010000, CRC(348c9517) SHA1(cc4d0d8964977a51bcb56b7ed1cb299581f6e0e7) )
	ROM_LOAD( "ffoot20a", 0x0000, 0x010000, CRC(6766a83f) SHA1(156e4f8212d8f3c4f1579d7d12da721096bf2fda) )
	ROM_LOAD( "sa6-043", 0x0000, 0x010000, CRC(9afdd391) SHA1(9caa30143edc338238fec9eb38b26ea91caddb01) )
	ROM_LOAD( "sa6-044", 0x0000, 0x010000, CRC(96e58b28) SHA1(0ac0b3c6ede1d2b5a6613e7af85e83736f2a7740) )
	ROM_LOAD( "sa6-045", 0x0000, 0x010000, CRC(6766a83f) SHA1(156e4f8212d8f3c4f1579d7d12da721096bf2fda) )
	ROM_LOAD( "sa6-046", 0x0000, 0x010000, CRC(d3de76ec) SHA1(18b912343a3ab02f1a543e9a264c3d24a06d39b6) )
	ROM_LOAD( "sa6-081", 0x0000, 0x010000, CRC(54ad781f) SHA1(e1f8589a99334f612a8d6ad44e243dd16325b5b5) )
	ROM_LOAD( "sa6-082", 0x0000, 0x010000, CRC(e015a6cc) SHA1(66970cba63a8f1f5ea5ad2f5a0a3ffb086ab28d9) )
	ROM_LOAD( "sa6-327", 0x0000, 0x010000, CRC(05641cb9) SHA1(52ca66c9bf325b080477b6a09b6ee964f2aca6e3) )
	ROM_LOAD( "sa6-328", 0x0000, 0x010000, CRC(a34c4e95) SHA1(a5e3d93d8f58fedcd4b56bcbc2e5d622f704260e) )
	ROM_LOAD( "sa6-329", 0x0000, 0x010000, CRC(eb3316ab) SHA1(79b64e400ec3e104fcfadcfe8f56b47771cf26ad) )
	ROM_LOAD( "sa6-330", 0x0000, 0x010000, CRC(6ba5ebc3) SHA1(99425719445abc974cba88c1c84b87cba4797ccc) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "fantasyfootball.p1", 0x000000, 0x080000, CRC(620a5736) SHA1(a202685b7b4882d262a111cfb1f35f91b00cbe5f) )
	ROM_LOAD( "fantasyfootball.p2", 0x080000, 0x080000, CRC(e25d95b0) SHA1(c6238d9d6c1ab3b478e40fa9145b6842bcc8fae5) )
ROM_END_M1A_MCU

ROM_START( m1fight )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "fight15f", 0x0000, 0x020000, CRC(ceb7ca5d) SHA1(911164a69541de05fd6dc64f54aad13a0d57ebd7) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "fight15r", 0x0000, 0x020000, CRC(46c7d3ca) SHA1(24bc785d888cb051ded94b04a47d286f26cfe4e4) )
	ROM_LOAD( "fightnt10.bin", 0x0000, 0x020000, CRC(95e891b5) SHA1(3412ce4db34f89a4a18209440770c3b8b3b37024) )
	ROM_LOAD( "finight55", 0x0000, 0x020000, CRC(50918abf) SHA1(bc9bd47637304d92028f2369d17e9f95362de301) )
	ROM_LOAD( "fn5p8ct.bin", 0x0000, 0x020000, CRC(603d3b0e) SHA1(4bd43497f40971a3e6e13f3cfbe7366747484fd7) )
	ROM_LOAD( "fnightapollo-10.bin", 0x0000, 0x020000, CRC(bfa1b1b1) SHA1(ad6af237c31b39d2483223a7376125b7d287d28e) )
	ROM_LOAD( "fnite10a", 0x0000, 0x020000, CRC(4615089a) SHA1(5d314b7dbeedf77591c4e9576e76631b309ddd93) )
	ROM_LOAD( "fn5p_pound15", 0x0000, 0x020000, CRC(cb5775d9) SHA1(48a01efa2b987befcf48b0b4dce98d3feb08273b) )
	ROM_LOAD( "sa7-401", 0x0000, 0x020000, CRC(c5a96f09) SHA1(e341e73d811f89db59976118d28dceef5cc1c819) )
	ROM_LOAD( "sa7-402", 0x0000, 0x020000, CRC(dd3635a8) SHA1(e518823e9f44c5591c1478a13b1d034f356dd9d1) )
	ROM_LOAD( "sa7-403", 0x0000, 0x020000, CRC(8d77cb14) SHA1(aa22eb13413347bfd93174a3dfea821ce796c4e2) )
	ROM_LOAD( "sa7-419", 0x0000, 0x020000, CRC(eb09da42) SHA1(32d5d16df6ef22a7a80c6d68736d55c53c10beb8) )
	ROM_LOAD( "sa7-420", 0x0000, 0x020000, CRC(b0a028a4) SHA1(2f607a076652281298b406965e2e42eae98762ce) )
	ROM_LOAD( "sa7-641", 0x0000, 0x020000, CRC(81a86bd4) SHA1(6a153c155dd1e8063b61d069da8a5127aa72e494) )
	ROM_LOAD( "sa7-642", 0x0000, 0x020000, CRC(926f5082) SHA1(60ea911d3c38a638c5daca72b7f15c5a773d64a2) )
	ROM_LOAD( "sa7-661", 0x0000, 0x020000, CRC(1f822a8c) SHA1(90989be5fe1ca6da87443f1988072fed4ec7a25e) )
	ROM_LOAD( "sa7-662", 0x0000, 0x020000, CRC(0c4511da) SHA1(5d0453d2f13b304cdd1412953c510888d601b19e) )
	ROM_LOAD( "sa7-741", 0x0000, 0x020000, CRC(4615089a) SHA1(5d314b7dbeedf77591c4e9576e76631b309ddd93) )
	ROM_LOAD( "sa7-742", 0x0000, 0x020000, CRC(55d233cc) SHA1(73e0f68f860f6aedd4dc5bc4c0f040799333bde8) )
	ROM_LOAD( "sa8-262", 0x0000, 0x020000, CRC(bdc36f37) SHA1(91cf9b321921511c7fbb90cd079ce470759836a6) )
	ROM_LOAD( "sa8-263", 0x0000, 0x020000, CRC(ed82918b) SHA1(0ec5b9f1b111013d26bd6ba0320e279a6f96395e) )
	ROM_LOAD( "sa8-264", 0x0000, 0x020000, CRC(2f8fa25e) SHA1(d20dcf6072fd4a2247e5870b76e9714dede52d66) )
	ROM_LOAD( "sa8-265", 0x0000, 0x020000, CRC(7fce5ce2) SHA1(8cc4bd8039622e15841477ae19cb0d8897a02296) )
	ROM_LOAD( "sk991157", 0x0000, 0x020000, CRC(a238eb60) SHA1(610a08854e05e5b56a3fd300b6e6cfa23b4eec6b) )
	ROM_LOAD( "sk991158", 0x0000, 0x020000, CRC(4478bcc6) SHA1(7cd2c42f271ae341430a70408512ef609d27a8d9) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "fnsnd.p1", 0x000000, 0x080000, CRC(f6f1334f) SHA1(607b136a3d8cf4ae2e306a4332b69f564936d383) )
	ROM_LOAD( "fnsnd.p2", 0x080000, 0x080000, CRC(13419292) SHA1(40f314b4f42384334e6929b0de6b6a899fa2c09f) )
ROM_END_M1A_MCU

ROM_START( m1fivest )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "fivestar.bin", 0x0000, 0x010000, CRC(c4228e1e) SHA1(a472c3b562a6d2585cb771c84587bf555ab82be5) )
ROM_END_M1A_MCU

ROM_START( m1frexpl )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "fex312v.bin", 0x0000, 0x020000, CRC(1a64fcce) SHA1(e38bcfeebf055a0a70d5ba228bbb15c53c90011e) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "fex5p8ct.bin", 0x0000, 0x020000, CRC(3335e861) SHA1(3dcd835217d977991ed0e3a08ebb18d934dd9caa) )
	ROM_LOAD( "fexp10.bin", 0x0000, 0x020000, CRC(1a64fcce) SHA1(e38bcfeebf055a0a70d5ba228bbb15c53c90011e) )
	ROM_LOAD( "fexp15f.bin", 0x0000, 0x020000, CRC(be6d81dc) SHA1(e6a3c38e1b231a3c31a5ff77c8c09f0ccde62138) )
	ROM_LOAD( "fexp15r.bin", 0x0000, 0x020000, CRC(a32b1fc1) SHA1(ace907e8eb75f8c311942df303e524f3df819c54) )
	ROM_LOAD( "frexp55", 0x0000, 0x020000, CRC(c91d2cca) SHA1(0b4ab76e66ce2ca3b7c14049e744966e8263f5d8) )
	ROM_LOAD( "fruitexp.bin", 0x0000, 0x020000, CRC(be6d81dc) SHA1(e6a3c38e1b231a3c31a5ff77c8c09f0ccde62138) )
	ROM_LOAD( "fruitexpapollo-10p8.bin", 0x0000, 0x020000, CRC(fb52cf03) SHA1(32739e60c8a4b7641e959250b1d259ce653c86f5) )
	ROM_LOAD( "sa7-188", 0x0000, 0x020000, CRC(c202e087) SHA1(49e98cb1f764c2efe7312ee0f2c239e735559bd7) )
	ROM_LOAD( "sa7-189", 0x0000, 0x020000, CRC(92431e3b) SHA1(cf35f864b73326b764c0111cab8164dee1d35f6c) )
	ROM_LOAD( "sa7-313", 0x0000, 0x020000, CRC(4a250272) SHA1(41f5a57fea8942562fa6a3d76d815d58ea09b657) )
	ROM_LOAD( "sa7-316", 0x0000, 0x020000, CRC(751e52ea) SHA1(4d248502c438bd0ba2d18073ba5c670a52699971) )
	ROM_LOAD( "sa7-317", 0x0000, 0x020000, CRC(255fac56) SHA1(8b4960f760ff612ff334376851ddd5305ea24819) )
	ROM_LOAD( "sa7-318", 0x0000, 0x020000, CRC(faacab9f) SHA1(64e185ab4a58e60928287f4095c3654e39b21770) )
	ROM_LOAD( "sa7-319", 0x0000, 0x020000, CRC(aaed5523) SHA1(4fed28bc17d3f59def1be220d6d4bc7464a6ea01) )
	ROM_LOAD( "sa7-729", 0x0000, 0x020000, CRC(8c1caaf6) SHA1(e8d457e3d3d0c6b098e8e2967e2a04c1a7df508d) )
	ROM_LOAD( "sa7-730", 0x0000, 0x020000, CRC(f599b30d) SHA1(74af8da6858c0483513860723c04b7da1ca68588) )
	ROM_LOAD( "sa8-116", 0x0000, 0x020000, CRC(41bc3827) SHA1(ae6571619e3aee0bdabb3d975300f92bf379fbfa) )
	ROM_LOAD( "sa8-117", 0x0000, 0x020000, CRC(11fdc69b) SHA1(373ec69d3a5502b9ff8daec0500ddaf263afea5e) )
	ROM_LOAD( "sa8-162", 0x0000, 0x020000, CRC(2623492b) SHA1(e6c701317100028eb7e542bfbbd4f03ff59d9df9) )
	ROM_LOAD( "sa8-163", 0x0000, 0x020000, CRC(7662b797) SHA1(137e3f812072e756d8ed3f5d3d6e3660cdcf8a34) )
	ROM_LOAD( "sa8-255", 0x0000, 0x020000, CRC(f36ae17d) SHA1(e7f13281ebe2eb295a0d54a09d01945dfb02d025) )
	ROM_LOAD( "sa8-257", 0x0000, 0x020000, CRC(ee2c7f60) SHA1(a66dc8e02d759868d0d723a1ed0082d0f26caa22) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "fexpsnd1.bin", 0x000000, 0x080000, CRC(68bb3788) SHA1(2e906d5ff597960ac0deeab0211a8eb0f0eba348) )
	ROM_LOAD( "fexpsnd2.bin", 0x080000, 0x080000, CRC(db357b2f) SHA1(10dc9c2c616582e12c423e52caa3686f334dd9ad) )
ROM_END_M1A_MCU

ROM_START( m1glad )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "game.gladiatorsstd(fb8b)1.u6", 0x0000, 0x010000, CRC(eae9f323) SHA1(1a345480b37ff88f263beb0ba3715954e0c6ecb0) ) // aka sa6-295

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "glad20p", 0x0000, 0x010000, CRC(b8803541) SHA1(a7c96501c031a84638bacf34a3e2c76dcd26bfe2) )
	ROM_LOAD( "glad5.10", 0x0000, 0x010000, CRC(1562bfcb) SHA1(294a770e42143b7a009a9f071b00a1ef0da20ae6) )
	ROM_LOAD( "glad5.8c", 0x0000, 0x010000, CRC(7bea2d24) SHA1(afdecaa90a6b86ce297fcbe4abd929669272ca21) )
	ROM_LOAD( "glad5.8t", 0x0000, 0x010000, CRC(094393b3) SHA1(8c0890a08c9b225c2382fea3dcaf45693158a4a4) )
	ROM_LOAD( "glad55", 0x0000, 0x010000, CRC(4c1cc2b1) SHA1(19c8decd5bc8a06898bae1132f3467ebd37477a0) )
	ROM_LOAD( "glad58t", 0x0000, 0x010000, CRC(aca9872b) SHA1(ff86b9138a87ee89778e0a611a51b9caf71bcf22) )
	ROM_LOAD( "gladiator.bin", 0x0000, 0x010000, CRC(3abab0d2) SHA1(749f6cb8ef7c2ef9c9634ce59719406eb0c89744) )
	ROM_LOAD( "sa5-013", 0x0000, 0x010000, CRC(82ec8a3e) SHA1(345acb122433332ad2ebb549fe506315fbb5f7ad) )
	ROM_LOAD( "sa5-014", 0x0000, 0x010000, CRC(82ec8a3e) SHA1(345acb122433332ad2ebb549fe506315fbb5f7ad) )
	ROM_LOAD( "sa5-028", 0x0000, 0x010000, CRC(023013f4) SHA1(f3a9304a82fbe38f28e22053e0c9bea54300f3fe) )
	ROM_LOAD( "sa5-029", 0x0000, 0x010000, CRC(ead88e12) SHA1(d09aa8c28cb43487c88f162562c2c5a3fbe39368) )
	ROM_LOAD( "sa5-205", 0x0000, 0x010000, CRC(d58e81ab) SHA1(2142b3cd6c3af1600ee6b26e77a7468e2074da3a) )
	ROM_LOAD( "sa5-206", 0x0000, 0x010000, CRC(61365f78) SHA1(ed0f5d1b26446b8d146b9c1dd0cbc5828a2347e8) )
	ROM_LOAD( "sa5-413", 0x0000, 0x010000, CRC(aa536516) SHA1(b5da5006e40635dda7b3dd317a31b4a7547b8cce) )
	ROM_LOAD( "sa5-414", 0x0000, 0x010000, CRC(a64b3daf) SHA1(bb88cd1966fe74e063947c804f09b4909db2a50a) )
	ROM_LOAD( "sa5-415", 0x0000, 0x010000, CRC(02607a5e) SHA1(d1ca5868738f76c0dbe2c5e3ce2cb28ee0356f20) )
	ROM_LOAD( "sa5-416", 0x0000, 0x010000, CRC(b6d8a48d) SHA1(7b1d83f813224b591485a52d0407a449d37ad17b) )
	ROM_LOAD( "sa5-417", 0x0000, 0x010000, CRC(843a5b96) SHA1(747b4a798fa9baf127dd1025b56a38e5a0368513) )
	ROM_LOAD( "sa6-296", 0x0000, 0x010000, CRC(5e512df0) SHA1(5df8a4dea14892319d9af58644c8ce8c4456db9a) )
	ROM_LOAD( "sa7-172", 0x0000, 0x010000, CRC(a9df5626) SHA1(bf872ea0ff26037bbf07408e66b57a74068fec90) )
	ROM_LOAD( "sa7-173", 0x0000, 0x010000, CRC(4137cbc0) SHA1(921c202d15578ae2deffba5a0b900a384389ed9e) )
	ROM_LOAD( "sa7-456", 0x0000, 0x010000, CRC(a069d090) SHA1(a617ae1405dc538edda5a6cca4ffcb02e55fb915) )
	ROM_LOAD( "sa7-457", 0x0000, 0x010000, CRC(48814d76) SHA1(266b5ff6267c7a4d3868577dcbe846b71767ef45) )
	ROM_LOAD( "sa7-657", 0x0000, 0x010000, CRC(232adb67) SHA1(81181dc48532e134857ebe63cc770c205755c088) )
	ROM_LOAD( "sa7-657.bin", 0x0000, 0x010000, CRC(232adb67) SHA1(81181dc48532e134857ebe63cc770c205755c088) )
	ROM_LOAD( "sa7-658", 0x0000, 0x010000, CRC(8502894b) SHA1(714b9c8085f19a518fd6a73daba0fdc17447dae5) )

	ROM_REGION( 0x100000, "gals", ROMREGION_ERASE00  ) // these might be protected?
	ROM_LOAD( "gal16v8.esp.b.u9", 0x0000, 0x000117, CRC(d0ea9b54) SHA1(b7611fb4004431a21f81be10934392bea8dc00a0) )
	ROM_LOAD( "gal16v8a.m1a.a.u32", 0x0000, 0x000117, CRC(5da2b5ab) SHA1(7af5ee675e280905fa41aee23b06394a59c8758d) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "sound1.dig1-049.u2", 0x0000, 0x080000, CRC(ab0ef8aa) SHA1(e9cd8c7c0fd0bec44d0531eff6272aa10b88b08c) )
	ROM_LOAD( "sound2.dig1-049.u3", 0x0000, 0x080000, CRC(44c05fb6) SHA1(8d40d62d7c55224ddca8ff2f90779d5fad2af3ba) )
//  ROM_LOAD( "dig1-049.p1", 0x0000, 0x080000, CRC(ab0ef8aa) SHA1(e9cd8c7c0fd0bec44d0531eff6272aa10b88b08c) )
//  ROM_LOAD( "dig1-049.p2", 0x0000, 0x080000, CRC(44c05fb6) SHA1(8d40d62d7c55224ddca8ff2f90779d5fad2af3ba) )
//  ROM_LOAD( "dig1-049_u2.bin", 0x0000, 0x080000, CRC(ab0ef8aa) SHA1(e9cd8c7c0fd0bec44d0531eff6272aa10b88b08c) )
//  ROM_LOAD( "dig1-049_u3.bin", 0x0000, 0x080000, CRC(44c05fb6) SHA1(8d40d62d7c55224ddca8ff2f90779d5fad2af3ba) )
ROM_END_M1A_MCU

ROM_START( m1glada )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-640", 0x0000, 0x010000, CRC(5a1a3b23) SHA1(1ca9bc9b718df8d7bdf33044c05d90e7c60d0c73) )
ROM_END_M1A_MCU

ROM_START( m1gladsh )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-009", 0x0000, 0x010000, CRC(90d4abe9) SHA1(db5992ed2314e36fce2624c414415fdda945b99b) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-010", 0x0000, 0x010000, CRC(10425681) SHA1(bfb044a2815d50f10b8fa9eaf371db2a2954e9e2) )
ROM_END_M1A_MCU

ROM_START( m1goldsv )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-161.bin", 0x0000, 0x010000, CRC(e5aaf4c3) SHA1(d08bbfc2df17e722c4a9e0688eefe6ad133c3cd2) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-162.bin", 0x0000, 0x010000, CRC(0a0aacca) SHA1(d481e5fbbb89c8f46f2a13324d3fcccced0e1f62) )
ROM_END_M1A_MCU

ROM_START( m1gold10 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "golden10.bin", 0x0000, 0x010000, CRC(2838c1d4) SHA1(7d6fbdae68bf44de264e5edcdf9aba439d8e23a8) )
ROM_END_M1A_MCU

ROM_START( m1goldng )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc3-511.bin", 0x0000, 0x010000, CRC(ccb200ad) SHA1(98da9a936f89a78eb86d89a2335df6d86340bbcb) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc3-512.bin", 0x0000, 0x010000, CRC(780ade7e) SHA1(ffe2411b6c108b8506994240f625873052724fc6) )
	ROM_LOAD( "sc3-551.bin", 0x0000, 0x010000, CRC(e916abf6) SHA1(e62809a041f6df9bf034f8e741e023dd5459a4ce) )
	ROM_LOAD( "sc3-552.bin", 0x0000, 0x010000, CRC(5dae7525) SHA1(077513c0031ee09b4c32d0e6cb0ba385f73cfbf7) )
	ROM_LOAD( "sc4-393.bin", 0x0000, 0x010000, CRC(d7e74cdf) SHA1(42cee5d40c4c34c77e8062dd8cbdb9ace1258031) )
	ROM_LOAD( "sc4-394.bin", 0x0000, 0x010000, CRC(9147ef81) SHA1(c8c65dd40ff96c02c15715a9248fe1ac5b9e00c5) )
ROM_END_M1A_MCU

ROM_START( m1gresc )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ge5p810.bin", 0x0000, 0x020000, CRC(67cb37b9) SHA1(48ef081a52400afd533909eacf8872f094b98049) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "geprog", 0x0000, 0x020000, CRC(f3c2bdd4) SHA1(9c71b6ef179f7c059715d9a7005cc36e07d8854b) )
	ROM_LOAD( "gtesc55", 0x0000, 0x020000, CRC(96042c55) SHA1(e54c8d323d3bc845ed270f4325741fb7bf31f55e) )
	ROM_LOAD( "sa7-157", 0x0000, 0x020000, CRC(6753dc1a) SHA1(34f7dc84482a020fb028b036ca941422454afcf6) )
	ROM_LOAD( "sa7-158", 0x0000, 0x020000, CRC(26f37d71) SHA1(9ae6e4f816feb71c8938f435ff2085e41ae7a44e) )
	ROM_LOAD( "sa7-244", 0x0000, 0x020000, CRC(b6828a2d) SHA1(36335507acbed03275b02762f0dce0fdedd96374) )
	ROM_LOAD( "sa7-245", 0x0000, 0x020000, CRC(e6c37491) SHA1(2915815c5885974010bb337f9b4adf3dfecaf1ce) )
	ROM_LOAD( "sa7-249", 0x0000, 0x020000, CRC(c7b7e6be) SHA1(ba523bc79f3bc1def9974b821b010df835832cb0) )
	ROM_LOAD( "sa7-250", 0x0000, 0x020000, CRC(be32ff45) SHA1(14f7ebe10482a192de6527afacfe7fa6ad18822c) )
	ROM_LOAD( "sa7-740", 0x0000, 0x020000, CRC(53e170b7) SHA1(964bc3a5598ee9303cad7fd90a65322a2a1d20dd) )
	ROM_LOAD( "sa8-102", 0x0000, 0x020000, CRC(1fd002c2) SHA1(cca5bcbc845d5aaf8b7416670a35b0557d5d7dd6) )
	ROM_LOAD( "sa8-176", 0x0000, 0x020000, CRC(f21f7497) SHA1(aaaa049b3006297d830441a6ecb2db700dab15ae) )
	ROM_LOAD( "sa8-177", 0x0000, 0x020000, CRC(a25e8a2b) SHA1(ba8e0b2ea13c5ab3f7f6f83ef1b19c2041a5de88) )
	ROM_LOAD( "sa8-178", 0x0000, 0x020000, CRC(494b19ec) SHA1(1a7414a7c0c5c50155c07e6e409f119e4fc93add) )
	ROM_LOAD( "sa8-179", 0x0000, 0x020000, CRC(190ae750) SHA1(d9138ed325b8450c431c7685a3395e43b31c73c2) )
	ROM_LOAD( "sa8-274", 0x0000, 0x020000, CRC(cb6da2c4) SHA1(fb7eff962d3c0508a893d56851d1d46598a082c5) )
	ROM_LOAD( "sa8-275", 0x0000, 0x020000, CRC(9b2c5c78) SHA1(ab16de1da60eb0fd95e18ab0849bcc7ab2413804) )
	ROM_LOAD( "tge.bin", 0x0000, 0x020000, CRC(4c11546b) SHA1(453f9980f5aa02b44a1341ac37f4f101feab05bd) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "greatescape.p1", 0x000000, 0x080000, CRC(a7e0a7a8) SHA1(f65171d72a6add5ebc903aac18b426d91134a492) )
	ROM_LOAD( "greatescape.p2", 0x080000, 0x080000, CRC(61de5e0f) SHA1(26ca6786de4da1e9c48de47887ed6e3e7e3a108b) )
ROM_END_M1A_MCU

ROM_START( m1guvnor )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "guvnor-8.p1", 0x0000, 0x010000, CRC(b328a620) SHA1(47da578423c95c4bfe30c96b9cce357435f20376) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "guvnor 20p 10.bin", 0x0000, 0x010000, CRC(63405fd0) SHA1(0e4fddb86bd0978e4d0facc954f1e67d7b4c0690) )
	ROM_LOAD( "sa4-386", 0x0000, 0x010000, CRC(912512fd) SHA1(4fc1591cfffa5de8501766092f425f8c39b353a5) )
	ROM_LOAD( "sa4-548", 0x0000, 0x010000, CRC(b0a9bea6) SHA1(3ed5911e7d57deab662ec0c75c0b1f9b88970064) )
	ROM_LOAD( "sa4-549", 0x0000, 0x010000, CRC(1b6a1ab4) SHA1(b940b2bda7cf83b61b18a7826b90ea58d8accecb) )
	ROM_LOAD( "sa5-046", 0x0000, 0x010000, CRC(f3d0ca0c) SHA1(1e8af05e089782165b2da39ca3f2bb3c72a6813e) )
	ROM_LOAD( "sa5-047", 0x0000, 0x010000, CRC(1b3857ea) SHA1(0116b5d29c703cad0c6ad53d7b56d2403e0b9c1a) )
	ROM_LOAD( "sa5-350", 0x0000, 0x010000, CRC(b8fef43c) SHA1(a612d30c0c0bd1bb02abe01d1a599a37c16db3e0) )
	ROM_LOAD( "sa5-351", 0x0000, 0x010000, CRC(1c41c115) SHA1(70874566b21a8845e3b870a96baf3bd8c252795e) )
	ROM_LOAD( "sa5-352", 0x0000, 0x010000, CRC(42aebd6f) SHA1(86b7549e2d5f9faa6c83b8966e2360a5fe0fbda9) )
	ROM_LOAD( "sa5-425", 0x0000, 0x010000, CRC(d0145d33) SHA1(c4fff32cd3ce4a6c88b081e12833d04ad0d647e9) )
	ROM_LOAD( "sa5-426", 0x0000, 0x010000, CRC(64ac83e0) SHA1(590d230a5da803e3e3c5805da414a5b2be2aae43) )
	ROM_LOAD( "sa5-457", 0x0000, 0x010000, CRC(e5291aa0) SHA1(c7e6ff169b6ecd0d8b4a60b4bf83caccfe5ce9bf) )
	ROM_LOAD( "sa5-458", 0x0000, 0x010000, CRC(4301488c) SHA1(6c14e306033e1a8cfd6ed5c2f6db9eef20b84ce4) )
	ROM_LOAD( "sa5-536", 0x0000, 0x010000, CRC(0ec98bad) SHA1(f1da45a9a2fa3f6bf505fd3bb9115176f0b38834) )
	ROM_LOAD( "sa6-026", 0x0000, 0x010000, CRC(765055b9) SHA1(52112f8b1876ee36bc01c39a6219b98d1e749617) )
	ROM_LOAD( "sa6-124", 0x0000, 0x010000, CRC(42ca37b1) SHA1(0b4fd418fdb468ca3618e1ad81f955dd33099e50) )
	ROM_LOAD( "sa6-125", 0x0000, 0x010000, CRC(aa22aa57) SHA1(fb5216756d2996b34cbc6568d19fa31b8498cd24) )
	ROM_LOAD( "sa6-212", 0x0000, 0x010000, CRC(63405fd0) SHA1(0e4fddb86bd0978e4d0facc954f1e67d7b4c0690) )
	ROM_LOAD( "sa6-213", 0x0000, 0x010000, CRC(8ba8c236) SHA1(4855c31ef42ca854278c9fb22aa1ea9aba5a1302) )
	ROM_LOAD( "sa6-214", 0x0000, 0x010000, CRC(d801f442) SHA1(923e86f2ab0c8d30ad831690d57f257f4a7a49f5) )
	ROM_LOAD( "sa6-215", 0x0000, 0x010000, CRC(bfb4cd16) SHA1(8129b75c7dd236a4053ed9af4af4410a7cd1bd0a) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "dig1-036 snd u2.bin", 0x000000, 0x080000, CRC(ba0cec08) SHA1(03c009f0157000785931139107745df7df005227) )
	ROM_LOAD( "dig1-036 snd u3.bin", 0x080000, 0x080000, CRC(2213e2e9) SHA1(b442f1af81326946df2bf4c7ea12c805d221f4f6) )
ROM_END_M1A_MCU

ROM_START( m1htclb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "hitensionclubv1-2.rom", 0x0000, 0x010000, CRC(b33fa7a9) SHA1(9aa2b61bac96441a3aa9da254c54636d29a895e2) )
	ROM_LOAD( "hitensio.bin", 0x0000, 0x010000, CRC(b8c8edc9) SHA1(1f989a3a72fb7c7747fc6deeb08e06da429620ee) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc3-057.bin", 0x0000, 0x010000, CRC(b33fa7a9) SHA1(9aa2b61bac96441a3aa9da254c54636d29a895e2) )
ROM_END_M1A_MCU

ROM_START( m1hotpot )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "hot5.8", 0x0000, 0x010000, CRC(bcba1c2b) SHA1(e8fa304b6775802d72ce21b7893659be34ca7921) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "hotp208t", 0x0000, 0x010000, CRC(d53b48a8) SHA1(b7282cbe647f74da53a15fa916016c56bdd075f2) )
	ROM_LOAD( "hotp490v", 0x0000, 0x010000, CRC(e9a99b05) SHA1(9706b004e9aade7850e8644cc471c78c46d8e116) )
	ROM_LOAD( "hotp5.8", 0x0000, 0x010000, CRC(bcba1c2b) SHA1(e8fa304b6775802d72ce21b7893659be34ca7921) )
	ROM_LOAD( "hotp510", 0x0000, 0x010000, CRC(e6e61c12) SHA1(67f364dcdf228b8a673ae5ff565ac595c9d2c10a) )
	ROM_LOAD( "hotp58", 0x0000, 0x010000, CRC(c97afa46) SHA1(163c02a03931f1378e6b4274974b6ae013689cd7) )
	ROM_LOAD( "hpot060v", 0x0000, 0x010000, CRC(11b77bfb) SHA1(ec99ccab4ffd53a0848110e66e2e039f5dc24684) )
	ROM_LOAD( "hpot385p", 0x0000, 0x010000, CRC(e8cb1cf8) SHA1(5b5bc24f6920472e1bcddb2b35b333266f7da61e) )
	ROM_LOAD( "hpot58c.bin", 0x0000, 0x010000, CRC(6fba62f0) SHA1(8f68a6e19de6454d3031a3d681e0f7b5fbc29a1a) )
	ROM_LOAD( "hpots5-8tok.bin", 0x0000, 0x010000, CRC(59b8de78) SHA1(f96c154bd14e80e83d100ad32fcedc6f20f48f6e) )
	ROM_LOAD( "sa6-060", 0x0000, 0x010000, CRC(11b77bfb) SHA1(ec99ccab4ffd53a0848110e66e2e039f5dc24684) )
	ROM_LOAD( "sa6-061", 0x0000, 0x010000, CRC(f95fe61d) SHA1(551109d0b0d5ce809a334667f2d97f560c15ba23) )
	ROM_LOAD( "sa6-139", 0x0000, 0x010000, CRC(bd5a297d) SHA1(288d9a5d1b5607c0b49709cb36abeb12fd7b9db7) )
	ROM_LOAD( "sa6-140", 0x0000, 0x010000, CRC(ddfaa28c) SHA1(042c94547550dda0153a785a777809d47f7218c7) )
	ROM_LOAD( "sa6-141", 0x0000, 0x010000, CRC(9d1178af) SHA1(01413ec9c33a7908c3c4be575e3dc13f0d2ddb4a) )
	ROM_LOAD( "sa6-142", 0x0000, 0x010000, CRC(29a9a67c) SHA1(cf8965c68e7c1a68a8cb6d01759e6a0b64684f3c) )
	ROM_LOAD( "sa6-207", 0x0000, 0x010000, CRC(93545e49) SHA1(da636e9c66ab37947b95f965caf487d8c674c4d5) )
	ROM_LOAD( "sa7-438", 0x0000, 0x010000, CRC(e8cb1cf8) SHA1(5b5bc24f6920472e1bcddb2b35b333266f7da61e) )
	ROM_LOAD( "sa7-439", 0x0000, 0x010000, CRC(0023811e) SHA1(95eae806990ff91525597aad11e1eed6e6454fbc) )
	ROM_LOAD( "sa7-490", 0x0000, 0x010000, CRC(e9a99b05) SHA1(9706b004e9aade7850e8644cc471c78c46d8e116) )
	ROM_LOAD( "sa7-491", 0x0000, 0x010000, CRC(014106e3) SHA1(1e3ba445ed2e2d62ba68595829984f3853ad909e) )
	ROM_LOAD( "sa7-492", 0x0000, 0x010000, CRC(0433f900) SHA1(5d41b19a750ff2d44cbe33bc8fb6e3b6d61e409b) )
	ROM_LOAD( "sa7-493", 0x0000, 0x010000, CRC(ecdb64e6) SHA1(a555e8a657fab82535fe60961573a0fa4c208f12) )
	ROM_LOAD( "sa7-494", 0x0000, 0x010000, CRC(d53b48a8) SHA1(b7282cbe647f74da53a15fa916016c56bdd075f2) )
	ROM_LOAD( "sa7-495", 0x0000, 0x010000, CRC(3dd3d54e) SHA1(bf1213a91b3911d3eb61d2100ce78e5ce68bf0a9) )
	ROM_LOAD( "sa7-564", 0x0000, 0x010000, CRC(066df1e2) SHA1(f3b6cd5bf23a96098ecda9981a61059916ec97b8) )
	ROM_LOAD( "sa9-027", 0x0000, 0x010000, CRC(808a6217) SHA1(452139e227131738928cc703176758c1ac8ec9f5) )
	ROM_LOAD( "sa9-028", 0x0000, 0x010000, CRC(26a2303b) SHA1(01bf60f7695972cbc416e65a7929532f122841fc) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "hotpots.p1", 0x000000, 0x080000, CRC(4988fd70) SHA1(7aaeca764b57787414dbec8d3519cb5681e011a5) )
	ROM_LOAD( "hotpots.p2", 0x080000, 0x080000, CRC(79f03328) SHA1(bcccfc62db91c79d6a4552ad4b15040137b254e6) )
ROM_END_M1A_MCU

ROM_START( m1infern )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "inferno.bin", 0x0000, 0x010000, CRC(02b623e8) SHA1(1dcefb2da67dd27fc1270cd59b5019c3017f6eb0) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "inferno_6p.bin", 0x0000, 0x010000, CRC(02b623e8) SHA1(1dcefb2da67dd27fc1270cd59b5019c3017f6eb0) )
	ROM_LOAD( "sa5-072", 0x0000, 0x010000, CRC(a158f803) SHA1(949676619349ecf8274bf0c64ea897dcba485745) )
	ROM_LOAD( "sa5-077", 0x0000, 0x010000, CRC(ea5ebe0e) SHA1(52b48c6e479191f31d6cbe0e6db7921d3d60b219) )
	ROM_LOAD( "sa5-094", 0x0000, 0x010000, CRC(8dcc3b83) SHA1(839e15a704a681dc1cfe8156d94f10106f0ceb06) )
	ROM_LOAD( "sa5-095", 0x0000, 0x010000, CRC(6524a665) SHA1(7d396a662d1d0645717f994590bf94f7fa77e6c2) )
	ROM_LOAD( "sa5-106", 0x0000, 0x010000, CRC(82b00205) SHA1(19bf200e2d51623d2ef082c6c4a9a4993ab4ff48) )
	ROM_LOAD( "sa5-264", 0x0000, 0x010000, CRC(762e765b) SHA1(4f90e49a6b189672a8d2a3bf19bc18c4c1734d46) )
	ROM_LOAD( "sa6-616", 0x0000, 0x010000, CRC(f214197d) SHA1(6420bb0d9dead1d5583162d9d32153e2c0ab30d6) )
	ROM_LOAD( "sa6-617", 0x0000, 0x010000, CRC(1afc849b) SHA1(119e3687ed2c8e04e818fde43a1233dc07c744a2) )
	ROM_LOAD( "sa6-654", 0x0000, 0x010000, CRC(9e7237b0) SHA1(6a37690a5ecc1b1dc606f25cd3eb3d5b44169288) )
	ROM_LOAD( "sa6-655", 0x0000, 0x010000, CRC(769aaa56) SHA1(1584e7cb4c6a6a8ffc0a2a9e0a41e976bc5b0127) )

	ROM_LOAD( "sa6-351", 0x0000, 0x010000, CRC(1e4125cb) SHA1(e24e7c264d26df71191461acc388b31e73deca21) )
	ROM_LOAD( "sa6-352", 0x0000, 0x010000, CRC(aaf9fb18) SHA1(0e9cde894f1eb601e397dd79c96b05973a805a9e) )


	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "inferno.p1", 0x000000, 0x080000, CRC(ff8749ff) SHA1(509b53f09cdfe5ee865e60ab42fd578586ac53ea) )
	ROM_LOAD( "inferno.p2", 0x080000, 0x080000, CRC(c8165b6c) SHA1(7c5059ee8630da31fc3ad50d84a4730297757d46) )
ROM_END_M1A_MCU



ROM_START( m1imclb )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc6-363", 0x0000, 0x010000, CRC(b7e15193) SHA1(a026243b2ba71da26b058d899e1f04c9e897fd15) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc6-364", 0x0000, 0x010000, CRC(bbf9092a) SHA1(459a9851c3e2316f57593bd4442a9633ef8d04de) )
	ROM_LOAD( "imilclub", 0x0000, 0x020000, CRC(fbdf8964) SHA1(f71b8d70b28c0a2d7d96dd05128b7bc13437c97b) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  ) // might be wrong, these weren't marked as club version
	ROM_LOAD( "instantmillionaresound.p1", 0x000000, 0x080000, CRC(c1e354c4) SHA1(420c6d862034fe4f6a30767f83671ac7fd86780a) )
	ROM_LOAD( "instantmillionaresound.p2", 0x080000, 0x080000, CRC(444b9b50) SHA1(cbb4f5ee3fb91ecb261bd79cb20823795d7c620a) )
ROM_END_M1A_MCU

ROM_START( m1inwin )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "inst5.10", 0x0000, 0x010000, CRC(d45ccc7d) SHA1(333a5842ea77687d8281c553d709b533fd390f4c) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "inst5.8", 0x0000, 0x010000, CRC(b780b52b) SHA1(feb0ed1dfabcf06f39f8dbf5d40c74db2e1695a3) )
	ROM_LOAD( "inst55", 0x0000, 0x010000, CRC(edac7794) SHA1(8045d36a17a8fe5fb4907598f6174f989ce30e5e) )
	ROM_LOAD( "instanwin.bin", 0x0000, 0x010000, CRC(c0773375) SHA1(2f0fd15d7fe997dc853aadf53192bdd9fb05e137) )
	ROM_LOAD( "instw10", 0x0000, 0x010000, CRC(953d570b) SHA1(a37dc929a310a2c30dccce2bf8e265c9557d2aa4) )
	ROM_LOAD( "its10", 0x0000, 0x010000, CRC(59690eff) SHA1(6d40a89f1cf1df46cfa832cfadd9883c0019ba9f) )
	ROM_LOAD( "iwin5p8c.bin", 0x0000, 0x010000, CRC(bff9750a) SHA1(121238a47fa6b70a5f1673dffd4ce300b4fa5b74) )
	ROM_LOAD( "iwin705p", 0x0000, 0x010000, CRC(734df32c) SHA1(873137f9c998885251e59d27e2b3f11a935f2db0) )
	ROM_LOAD( "sa5-316", 0x0000, 0x010000, CRC(bb7ff871) SHA1(f6205b40af696184c71c8ec4fea2295b51075288) )
	ROM_LOAD( "sa5-317", 0x0000, 0x010000, CRC(53976597) SHA1(3a45df120114c473da125eabbe2f6fbfaf3696bc) )
	ROM_LOAD( "sa5-318", 0x0000, 0x010000, CRC(511f804a) SHA1(e161735816131ca31c0386050c5394a124f53695) )
	ROM_LOAD( "sa5-319", 0x0000, 0x010000, CRC(b9f71dac) SHA1(d2f811b30e317dacd290ae8337349e5a92896078) )
	ROM_LOAD( "sa5-321", 0x0000, 0x010000, CRC(289fae93) SHA1(c4ab31995a895688e417b1ae93e190bf94aa2c7d) )
	ROM_LOAD( "sa6-089", 0x0000, 0x010000, CRC(5c516ecd) SHA1(185447031707afb0571b21e08119ae2c398c330a) )
	ROM_LOAD( "sa6-090", 0x0000, 0x010000, CRC(dcc793a5) SHA1(e0a908b212a194b751787ab13e47a232779406df) )
	ROM_LOAD( "sa6-190", 0x0000, 0x010000, CRC(953d570b) SHA1(a37dc929a310a2c30dccce2bf8e265c9557d2aa4) )
	ROM_LOAD( "sa6-191", 0x0000, 0x010000, CRC(7dd5caed) SHA1(db3d1e565bcb75a83558651e845a87342047c7cc) )
	ROM_LOAD( "sa6-256", 0x0000, 0x010000, CRC(cc4d8b81) SHA1(b8e522511c9371fd348b20001538c649bda77701) )
	ROM_LOAD( "sa6-257", 0x0000, 0x010000, CRC(24a51667) SHA1(edcc38c46f111ca906f220f67de800956206d054) )
	ROM_LOAD( "sa7-168", 0x0000, 0x010000, CRC(e2e751c9) SHA1(3c7702c62838d1a08754162e89660baf7d6fd8cd) )
	ROM_LOAD( "sa7-169", 0x0000, 0x010000, CRC(0a0fcc2f) SHA1(fd8595582ec279928ae297c7f61eaa1459cf355d) )
	ROM_LOAD( "sa7-170", 0x0000, 0x010000, CRC(734df32c) SHA1(873137f9c998885251e59d27e2b3f11a935f2db0) )
	ROM_LOAD( "sa7-171", 0x0000, 0x010000, CRC(9ba56eca) SHA1(15d43e9fbb9ecd3541196507964ad00550843153) )
	ROM_LOAD( "sa7-655", 0x0000, 0x010000, CRC(0244bdd7) SHA1(1e2a222645063cf9bf22f9b432757f994096a51c) )
	ROM_LOAD( "sa7-656", 0x0000, 0x010000, CRC(b6fc6304) SHA1(ca6ecfbd16385db0292997fdc07924e222206c0d) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "iwsnd1.bin", 0x000000, 0x080000, CRC(7658063f) SHA1(89cb329dc792b6086e50c01aed7b483f941b4b27) )
	ROM_LOAD( "iwsnd2.bin", 0x080000, 0x080000, CRC(c9708d5b) SHA1(fe01a351911e48a386c5ce9200a2ac28cf399e54) )
ROM_END_M1A_MCU

ROM_START( m1itsko )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "iak5p8cbin", 0x0000, 0x010000, CRC(15592078) SHA1(f8a55bb1623895eabe8b6cc8d2df14b70806a77c) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "iakn850v", 0x0000, 0x010000, CRC(272ef4a9) SHA1(3e013e867ddda41b09ac51f20a5b8d59b63c15f1) )
	ROM_LOAD( "iaknoc10", 0x0000, 0x010000, CRC(93ff1f4c) SHA1(7aed91b2abc2d8df6f85456320c94ce58746fe53) )
	ROM_LOAD( "iako.bin", 0x0000, 0x010000, CRC(03e7c80b) SHA1(c8ca6f65f0d91f23b2714f7bd54760b1e4b6c949) )
	ROM_LOAD( "iako58", 0x0000, 0x010000, CRC(6a35ff9e) SHA1(ff04ded050973bca1d84f1984bdb1f881e8373ac) )
	ROM_LOAD( "iko8", 0x0000, 0x010000, CRC(272ef4a9) SHA1(3e013e867ddda41b09ac51f20a5b8d59b63c15f1) )
	ROM_LOAD( "its10", 0x0000, 0x010000, CRC(59690eff) SHA1(6d40a89f1cf1df46cfa832cfadd9883c0019ba9f) )
	ROM_LOAD( "its5.58", 0x0000, 0x010000, CRC(bc8803f5) SHA1(39acb4a66a82a7237aff204f99f4f710eaec55b6) )
	ROM_LOAD( "its55", 0x0000, 0x010000, CRC(315397eb) SHA1(00bdc965dfbb42162a2a746a2124bb25b381379c) )
	ROM_LOAD( "its58t", 0x0000, 0x010000, CRC(50bea13b) SHA1(8d9620c9c9a640ca790a47dc0596a3daf20a8dbd) )
	ROM_LOAD( "its8", 0x0000, 0x010000, CRC(ec66b836) SHA1(745aebd9a967b6c00e40a3cd6e1c9706fe21bffa) )
	ROM_LOAD( "sa6-028", 0x0000, 0x010000, CRC(a5cf9a27) SHA1(a0ab2dbc4766b992bce5959f72c0790a9444f627) )
	ROM_LOAD( "sa6-029", 0x0000, 0x010000, CRC(35ff1f57) SHA1(5980ea2e53c71de2b96476497b55e1d0061a44c0) )
	ROM_LOAD( "sa6-030", 0x0000, 0x010000, CRC(b569e23f) SHA1(08b55a38677c3f6915d5d24273425afd3b25a978) )
	ROM_LOAD( "sa6-083", 0x0000, 0x010000, CRC(92fe2658) SHA1(3959c0c6917e19d89c25005de9fd3e008ac1e82d) )
	ROM_LOAD( "sa6-085", 0x0000, 0x010000, CRC(029d7568) SHA1(ce3e59f430f43f9472ac8cee8daef7485873c8a5) )
	ROM_LOAD( "sa6-107", 0x0000, 0x010000, CRC(0c713932) SHA1(6b534623bee56a7300d7100c963f4888431bf8b5) )
	ROM_LOAD( "sa6-108", 0x0000, 0x010000, CRC(aa596b1e) SHA1(5581d88e698f10c0194ba59a1f897a8785b4e1c4) )
	ROM_LOAD( "sa6-109", 0x0000, 0x010000, CRC(272ef4a9) SHA1(3e013e867ddda41b09ac51f20a5b8d59b63c15f1) )
	ROM_LOAD( "sa6-110", 0x0000, 0x010000, CRC(a7b809c1) SHA1(542ceaf6a86fff0954d489a55c0952b4b8cfb684) )
	ROM_LOAD( "sa6-178", 0x0000, 0x010000, CRC(3aef5e61) SHA1(d97ce79c1623ffce19ca7099ee13cc2533a4e5bb) )
	ROM_LOAD( "sa6-179", 0x0000, 0x010000, CRC(d207c387) SHA1(2cbbabdb33f493c6a9c8fc209145ef986a92ad7e) )
	ROM_LOAD( "sa6-303", 0x0000, 0x010000, CRC(763e3771) SHA1(a57d5f80ada66298c1924d909fd3b360c8f03609) )
	ROM_LOAD( "sa6-304", 0x0000, 0x010000, CRC(7a266fc8) SHA1(987a35c1fea41c444d3b4654b20d7ef9356c12cc) )
	ROM_LOAD( "sa6-401", 0x0000, 0x010000, CRC(cc318abe) SHA1(d937a41054834ce409bd6160adcff5dca3c4c4ba) )
	ROM_LOAD( "sa6-402", 0x0000, 0x010000, CRC(7889546d) SHA1(c67b309ccb0d89c1d0820b450dd790ebf70e7bbd) )
	ROM_LOAD( "sa7-498", 0x0000, 0x010000, CRC(2326a0b5) SHA1(aba3b246ab4a162709cdf233aed222814a857b19) )
	ROM_LOAD( "sa7-499", 0x0000, 0x010000, CRC(cbce3d53) SHA1(ae43386e85640cb4831fa0180e51953eb8a18e29) )
	ROM_LOAD( "sa7-500", 0x0000, 0x010000, CRC(c8871f4a) SHA1(f0b399686b5bec4a8236ec295d3fbf18afe74edc) )
	ROM_LOAD( "sa7-501", 0x0000, 0x010000, CRC(206f82ac) SHA1(cb9e824b81127f2316cb9265f1548b71a48e0171) )
	ROM_LOAD( "sa7-534", 0x0000, 0x010000, CRC(1d6cb8a4) SHA1(d4fdb0ae259a9979dfa81f6db6d83f2ac5787135) )
	ROM_LOAD( "sa7-535", 0x0000, 0x010000, CRC(f5842542) SHA1(8c9afd2d6ba7a440a5600dbf377d823fddb8fd46) )
	ROM_LOAD( "sa8-132", 0x0000, 0x010000, CRC(de5e6f10) SHA1(69909b551587aec878690bd509a410610da37e54) )
	ROM_LOAD( "sa8-133", 0x0000, 0x010000, CRC(36b6f2f6) SHA1(fd90c83e796d392a5406546c12a5013dc5e797c9) )
	ROM_LOAD( "sa8-134", 0x0000, 0x010000, CRC(1e1f3be8) SHA1(db59165237968144040632c548cf2d10fe07c134) )
	ROM_LOAD( "sa8-135", 0x0000, 0x010000, CRC(f6f7a60e) SHA1(8c4e8ad3d9cbd557d21d4c8f4444b11258e08be4) )
	ROM_LOAD( "sa8-136", 0x0000, 0x010000, CRC(207b26f5) SHA1(33e50b9675a92f9264fe2fed808a1b141e039d9a) )
	ROM_LOAD( "sa8-137", 0x0000, 0x010000, CRC(c893bb13) SHA1(d22171d75ec697a44e8ddb3544d610244f020300) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "itsaknockout.p1", 0x000000, 0x080000, CRC(b7c9bf1b) SHA1(1de3758e1deca35e54f22921594d96a10491e8c0) )
	ROM_LOAD( "itsaknockout.p2", 0x080000, 0x080000, CRC(f37b0a62) SHA1(18af0ef42268a965fd5dd3ae30c677a75bd12033) )
ROM_END_M1A_MCU

ROM_START( m1itjob )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ij5p810.bin", 0x0000, 0x020000, CRC(9596a49b) SHA1(67ab3a01f9e1522f8b8f53657b952e184ab07002) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "ijob10", 0x0000, 0x020000, CRC(e88fcea5) SHA1(ab453071a6c3ff619e7e692e0c2da3472c5e58d0) )
	ROM_LOAD( "ijob398v", 0x0000, 0x020000, CRC(cc5a7b14) SHA1(0c48dd5294622c7edd8a3ac19e4d183b6c2d6770) )
	ROM_LOAD( "it_job.bin", 0x0000, 0x020000, CRC(6fd00000) SHA1(bbdfb3af0d794207c27a6f9197706ae258806a75) )
	ROM_LOAD( "italianjob5.bin", 0x0000, 0x020000, CRC(aaf68bd6) SHA1(857eced307156032e7b35af25d58182d07ac4cd2) )
	ROM_LOAD( "sa7-398", 0x0000, 0x020000, CRC(cc5a7b14) SHA1(0c48dd5294622c7edd8a3ac19e4d183b6c2d6770) )
	ROM_LOAD( "sa7-399", 0x0000, 0x020000, CRC(9c1b85a8) SHA1(a3a9887773fd87f439e11379f419566f9ef81467) )
	ROM_LOAD( "sa7-411", 0x0000, 0x020000, CRC(8b1e836a) SHA1(e9e7a64d9cf22db65c871b51e3d9de076eb847c6) )
	ROM_LOAD( "sa7-412", 0x0000, 0x020000, CRC(98d9b83c) SHA1(faee5b6dff454b154e35a653b579f60058ef8908) )
	ROM_LOAD( "sa7-413", 0x0000, 0x020000, CRC(665120b7) SHA1(b586a58d0a1175799e32bcbfc841e92b252ab315) )
	ROM_LOAD( "sa7-414", 0x0000, 0x020000, CRC(f29b9035) SHA1(01e5cb3de4d446827b1e1a671cf21d71b34689e6) )
	ROM_LOAD( "sa8-001", 0x0000, 0x020000, CRC(e88fcea5) SHA1(ab453071a6c3ff619e7e692e0c2da3472c5e58d0) )
	ROM_LOAD( "sa8-002", 0x0000, 0x020000, CRC(fb48f5f3) SHA1(de319699e9ee60706315fa423112dcaeb1f6fbfe) )
	ROM_LOAD( "sa8-234", 0x0000, 0x020000, CRC(7425ab48) SHA1(a42e19402588dc27d0c2a113be48ab5b6f7ccfb8) )
	ROM_LOAD( "sa8-235", 0x0000, 0x020000, CRC(246455f4) SHA1(f065c4357521e71cfad42690142f030c91f3571b) )
	ROM_LOAD( "sa8-236", 0x0000, 0x020000, CRC(606ce1d2) SHA1(d694f640fe7c5028eff237f5bbb8b32364c558a8) )
	ROM_LOAD( "sa8-237", 0x0000, 0x020000, CRC(302d1f6e) SHA1(a458fecae1f09e4c56b4375fd77aa1c5ccb3dfb1) )
	ROM_LOAD( "sa8-266", 0x0000, 0x020000, CRC(f23250b4) SHA1(c464bbd3a396fa32bba7c3e16de4c9d12005a4dc) )
	ROM_LOAD( "sa8-267", 0x0000, 0x020000, CRC(a273ae08) SHA1(72876cb81e80e96e75758ce8317a1843947a431a) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "digi141_1.bin", 0x000000, 0x080000, CRC(f90c7236) SHA1(5c8ebdf4ce40af04d96b64a47d9a01ea7dd40fa3) )
	ROM_LOAD( "digi141_2.bin", 0x080000, 0x080000, CRC(dfe50779) SHA1(643d1f932a7f42ee26c9a26dde470c87a8e00fc3) )
	//ROM_LOAD( "digi141a.bin", 0x0000, 0x080000, CRC(f90c7236) SHA1(5c8ebdf4ce40af04d96b64a47d9a01ea7dd40fa3) )
	//ROM_LOAD( "digi141b.bin", 0x0000, 0x080000, CRC(dfe50779) SHA1(643d1f932a7f42ee26c9a26dde470c87a8e00fc3) )
	// which set of sound roms are correct?
	ROM_LOAD( "italianjob.p1", 0x000000, 0x080000, CRC(21d6e0bf) SHA1(431510606c1c7fb1f452fa44a974361437bfada1) )
	ROM_LOAD( "italianjob.p2", 0x080000, 0x080000, CRC(ef58a68b) SHA1(f05215733fc47d6f33ab3b839b0dcc2bf7b12e7d) )
ROM_END_M1A_MCU

ROM_START( m1jpmult )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa0-253.bin", 0x0000, 0x010000, CRC(6f2273cd) SHA1(08cb6414a02a385995e0c4c52a108f76144a12f5) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa0-254.bin", 0x0000, 0x010000, CRC(633a2b74) SHA1(5e9aaff1ac900ba6d63642df165e67305247e1fa) )
ROM_END_M1A_MCU

ROM_START( m1jbond )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "jbnd2010", 0x0000, 0x010000, CRC(8a41f5c0) SHA1(f52187b04a9b0103495e93eac16b75789012e072) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-605", 0x0000, 0x010000, CRC(a0882696) SHA1(ae2055b7dbf5644408c21e799a4a2d5355617e0a) )
	ROM_LOAD( "sa4-606", 0x0000, 0x010000, CRC(1430f845) SHA1(687d54958db78c0da499e58814d916500e6d426e) )
	ROM_LOAD( "sa4-616", 0x0000, 0x010000, CRC(f02e862a) SHA1(2ad751d627f612eeb55f571677f325aaeb778242) )
	ROM_LOAD( "sa4-617", 0x0000, 0x010000, CRC(18c61bcc) SHA1(28ad6724478f6f613b79be8328b7bbfb99349a48) )
	ROM_LOAD( "sa4-619", 0x0000, 0x010000, CRC(0f1294f8) SHA1(8938e1deb9aab3357ec3d32db7d1a673113c4227) )
	ROM_LOAD( "sa4-641", 0x0000, 0x010000, CRC(6a31fa0c) SHA1(65e1a14d0e196148329cc7b6d5cf349a47556c42) )
	ROM_LOAD( "sa4-642", 0x0000, 0x010000, CRC(de8924df) SHA1(80dcde1915d1e6ffc7a6eb057f8267af783f7d66) )
	ROM_LOAD( "sa5-096", 0x0000, 0x010000, CRC(ab3e4077) SHA1(17548f1187cb9cea723fb2273cea9600b10a937e) )
	ROM_LOAD( "sa5-449", 0x0000, 0x010000, CRC(e8f06923) SHA1(581f33248be2f3f9e3897a1e9d4c6eaf6deda177) )
	ROM_LOAD( "sa5-450", 0x0000, 0x010000, CRC(6866944b) SHA1(76ac204be81c8e5c97e5d119666e44b306cb121a) )
	ROM_LOAD( "sa5-451", 0x0000, 0x010000, CRC(28945984) SHA1(416209cd1e4a26ee027dd89f97e122576405bc1d) )
	ROM_LOAD( "sa5-452", 0x0000, 0x010000, CRC(9c2c8757) SHA1(26040a81a6ea28126b8095a77b384939fe3227af) )
	ROM_LOAD( "sa5-453", 0x0000, 0x010000, CRC(0504d0ec) SHA1(766825a3afa187204b5a3f26b5e95babd53617bf) )
	ROM_LOAD( "sa5-454", 0x0000, 0x010000, CRC(091c8855) SHA1(0fc76d25fa75a263c314b9a8c4bff88b4b4772f9) )
	ROM_LOAD( "sa6-299", 0x0000, 0x010000, CRC(aed2feb0) SHA1(007762818674379548643da01714251c7e442539) )
	ROM_LOAD( "sa6-300", 0x0000, 0x010000, CRC(e91c6be8) SHA1(7e53c55f60626cf574f5126d5c8b70b1c285f010) )
	ROM_LOAD( "james-bond-10pound-iom.bin", 0x0000, 0x010000, CRC(409e30fa) SHA1(028df19cbb7cc2fb48a6f935b6477f1a8871770f) )
ROM_END_M1A_MCU

ROM_START( m1jdwins )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "jdwinstreak10.bin", 0x0000, 0x010000, CRC(aa1e3350) SHA1(c3cc5ad07e2b20a8e40a8cb112073f577785eb40) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-187", 0x0000, 0x010000, CRC(f3afdfce) SHA1(caa55cd313420d86fed0a80d3029635dc95fb4df) )
	ROM_LOAD( "sa5-188", 0x0000, 0x010000, CRC(55878de2) SHA1(64544c19da5779761d583be31e9eca82b45cb4cb) )
	ROM_LOAD( "sa5-189", 0x0000, 0x010000, CRC(fc915a26) SHA1(66393e357da26f5e12007ec529b183cbb5b9736c) )
	ROM_LOAD( "sa5-190", 0x0000, 0x010000, CRC(9f6ee9d5) SHA1(b549021cf0319d844c36a0e30b527f6c00c6ab7d) )
	ROM_LOAD( "sa5-191", 0x0000, 0x010000, CRC(553e6085) SHA1(c16f9c9d8b619fae47c07abafce6a2ed747cd8d0) )
	ROM_LOAD( "sa5-192", 0x0000, 0x010000, CRC(e186be56) SHA1(94be1d1dcb2b6fb1d632c6cc721fc0046160e55f) )
	ROM_LOAD( "sa5-362", 0x0000, 0x010000, CRC(96790069) SHA1(a88dc84c79caa900482b2456a200e6ad105faa60) )
	ROM_LOAD( "sa5-404", 0x0000, 0x010000, CRC(3c3ed908) SHA1(d47359186a7b34afdbce3f94363d2b9fa469a78d) )
	ROM_LOAD( "sa5-405", 0x0000, 0x010000, CRC(d4d644ee) SHA1(e0a9b503e3ca03cf0b1f1036265398398889e6b9) )
	ROM_LOAD( "sa6-319", 0x0000, 0x010000, CRC(aa1e3350) SHA1(c3cc5ad07e2b20a8e40a8cb112073f577785eb40) )
	ROM_LOAD( "sa6-320", 0x0000, 0x010000, CRC(75651c4f) SHA1(c4f89359e54e046d3e9a7a879aa8b0beaf695ab4) )
	ROM_LOAD( "sa6-321", 0x0000, 0x010000, CRC(b11b3e4b) SHA1(f14726f7ee3c8f68bac395d5f41fea60b53f6a09) )
	ROM_LOAD( "sa6-322", 0x0000, 0x010000, CRC(538ac0ad) SHA1(f937853402de4ae6104198d4a2148972e0a9c778) )
	ROM_LOAD( "sa6-513", 0x0000, 0x010000, CRC(c423c225) SHA1(3f170c75e8084b062fa00494d422d2bdc4a440c9) )
	ROM_LOAD( "sa6-514", 0x0000, 0x010000, CRC(c83b9a9c) SHA1(675e2a1cc5d4fe918d77798d85b2fd9c57edaec0) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "jimwinstrku2snd.bin", 0x000000, 0x080000, CRC(ded378b7) SHA1(e974ace69cacbe27a98127e8623e79dec06fde08) )
	ROM_LOAD( "jimwinstrku3snd.bin", 0x080000, 0x080000, CRC(da05f628) SHA1(039276be47ce2d5be34a55cca91868ac93b0d9ae) )
ROM_END_M1A_MCU


ROM_START( m1liveam )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-473", 0x0000, 0x010000, CRC(e8688274) SHA1(685de7fa350d80946d3dfe9b2d25ed07c2a493c2) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-502.bin", 0x0000, 0x010000, CRC(a65f5f5f) SHA1(dd02db137ad195845630f47a3c42b38d7a2cb8f3) )
	ROM_LOAD( "sa4-446", 0x0000, 0x010000, CRC(bb3c979a) SHA1(ea5aff8a689a8cee30088e64f94d6a8787a85100) )
ROM_END_M1A_MCU


ROM_START( m1lotmil )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc6-031", 0x0000, 0x010000, CRC(a283c82f) SHA1(8c0c487c3f6a16f777a9ba3dff74d724d00fea41) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc6-032", 0x0000, 0x010000, CRC(5c83ed1b) SHA1(b4ba1955e44f86aec605958050d70101ee285ca1) )
	ROM_LOAD( "sc6-033", 0x0000, 0x010000, CRC(e0394256) SHA1(021b06f111bc5063c5ed6e5d470c7065f062701f) )
	ROM_LOAD( "sc6-034", 0x0000, 0x010000, CRC(6e7d1fdb) SHA1(da87d8beebeed8274428e8f8f2f42b36bf5bef69) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "lottteymillion.p1", 0x000000, 0x080000, CRC(c1e354c4) SHA1(420c6d862034fe4f6a30767f83671ac7fd86780a) )
	ROM_LOAD( "lottteymillion.p2", 0x080000, 0x080000, CRC(444b9b50) SHA1(cbb4f5ee3fb91ecb261bd79cb20823795d7c620a) )
ROM_END_M1A_MCU


ROM_START( m1luckno )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "luckynumbers10.bin", 0x0000, 0x010000, CRC(296ae5eb) SHA1(7e1f78038a997ccb228ed23b5dba0c28e7aa1a22) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-267", 0x0000, 0x010000, CRC(503167ab) SHA1(3922499857314dac7c6f067892c20fdb1036ffcb) )
	ROM_LOAD( "sa5-268", 0x0000, 0x010000, CRC(f6193587) SHA1(10eaae6e48862b97d868a275b7e996684d99836c) )
	ROM_LOAD( "sa5-269", 0x0000, 0x010000, CRC(46b09c33) SHA1(3499d8f72f7eaa0ea0885518a4285a06e04cb08f) )
	ROM_LOAD( "sa5-270", 0x0000, 0x010000, CRC(8e227559) SHA1(7bbab32442b9fd58135401716dbb58b80c945920) )
	ROM_LOAD( "sa5-336", 0x0000, 0x010000, CRC(93e70cdc) SHA1(3653aea48f21a49ef331f9ea8ffa8f800214acca) )
	ROM_LOAD( "sa5-337", 0x0000, 0x010000, CRC(7b0f913a) SHA1(c71d768e04062d56922c4bf4e54d231a99c4ff45) )
	ROM_LOAD( "sa5-340", 0x0000, 0x010000, CRC(9ced04eb) SHA1(76e42a3efa26271c4c9051e4db15ea4cc382b485) )
	ROM_LOAD( "sa5-341", 0x0000, 0x010000, CRC(7e6b3c8d) SHA1(b1f35d7ab19b1771172e628280121d933c7fd588) )
	ROM_LOAD( "sa5-475", 0x0000, 0x010000, CRC(d16e3301) SHA1(3cd326af86b890607cc8038dbe048366620dde2d) )
	ROM_LOAD( "sa5-476", 0x0000, 0x010000, CRC(65d6edd2) SHA1(f1f8f277621f39a78d216d048238c3b012e1dce4) )
	ROM_LOAD( "sa5-477", 0x0000, 0x010000, CRC(f799b59d) SHA1(9ce3c6293f1881a7a7258f37dc9ec7aa4af9d482) )
	ROM_LOAD( "sa5-478", 0x0000, 0x010000, CRC(330d86a4) SHA1(90d5378e1aadb4280861690e06466d04eb36c71a) )
	ROM_LOAD( "sa5-498", 0x0000, 0x010000, CRC(2ab7c940) SHA1(444c6367248f20a26c0a9c9163944c229e4d2be9) )
	ROM_LOAD( "sa6-242", 0x0000, 0x010000, CRC(d345ed17) SHA1(6199004af81d5507c7e0ea4e816b0d0609e7ad1b) )
	ROM_LOAD( "sa6-243", 0x0000, 0x010000, CRC(3bad70f1) SHA1(09d2c933ee0289f7c77853f5e2b9707dde660519) )
	ROM_LOAD( "sa6-260", 0x0000, 0x010000, CRC(c0274b19) SHA1(896a40db5e379c5cb4a971129ef994c74b30f25d) )
	ROM_LOAD( "sa6-261", 0x0000, 0x010000, CRC(f51baa62) SHA1(efaa1f3be41edc86ade9e02710592bc352162812) )
	ROM_LOAD( "sa6-515", 0x0000, 0x010000, CRC(444ef475) SHA1(86343d0a8e51f196f7ddb3652cbd4fda6c9fabc1) )
	ROM_LOAD( "sa6-516", 0x0000, 0x010000, CRC(f0f62aa6) SHA1(b78a3aa39c812fcbbdfddd14da37cfa59dac905f) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "luckynumbersu2snd.bin", 0x000000, 0x080000, CRC(2c9216b2) SHA1(e6ebcc6fa6c5a88db592f21f07f4911edf2e4abc) )
	ROM_LOAD( "luckynumbersu3snd.bin", 0x080000, 0x080000, CRC(958ac365) SHA1(c307d973601336e3eb8769b15920465de92547a3) )
ROM_END_M1A_MCU


ROM_START( m1magic )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-169", 0x0000, 0x010000, CRC(fda3c0f4) SHA1(399bb4e55130ee06ca429fd52876ddcde0f07482) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-170", 0x0000, 0x010000, CRC(7d353d9c) SHA1(baef8b3ff9fed2414672b710d8acae02c06cf1ba) )
	ROM_LOAD( "sa4-171", 0x0000, 0x010000, CRC(74957d2b) SHA1(16f08b13a7fff8275c1012a93990bb1ea249d33b) )
	ROM_LOAD( "sa4-172", 0x0000, 0x010000, CRC(c02da3f8) SHA1(e7d1a3129f398855e4c466b4a5aac26844978beb) )
ROM_END_M1A_MCU

ROM_START( m1manhat )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "skyline.bin", 0x0000, 0x010000, CRC(ea9f3b18) SHA1(14d5c8cff598b43100d1a7c8692528de40c9e58d) )
ROM_END_M1A_MCU

ROM_START( m1bignit )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sc4-336.bin", 0x0000, 0x010000, CRC(e5af59c0) SHA1(e4d17a65f24477e15494f97c1daea1d9fddfb8f6) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc4-337.bin", 0x0000, 0x010000, CRC(0d47c426) SHA1(2ccf905f93484d6a2aa4400a19edbf44323482d3) )
	ROM_LOAD( "sc4-338.bin", 0x0000, 0x010000, CRC(e642508e) SHA1(b2ae16092c98d825a9a6436ba82a0a225a6dcd07) )
	ROM_LOAD( "sc4-339.bin", 0x0000, 0x010000, CRC(0eaacd68) SHA1(d17e43d565e8f61d768019c23f1d11ef31526aa2) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "mikereidsbignightout-snd_p1.rom", 0x000000, 0x080000, CRC(846b6223) SHA1(cd337b3499bfa3fd88c44ede71e7777032ce3d1f) )
	ROM_LOAD( "mikereidsbignightout-snd_p2.rom", 0x080000, 0x080000, CRC(56630516) SHA1(5064afd678b92316ea7bceaf0097eae80b54ea31) )
ROM_END_M1A_MCU

ROM_START( m1mongam )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "mgame200", 0x0000, 0x010000, CRC(4cfe0ef2) SHA1(8dae7d1fdb6481902bcc38f3f993b55c7acc919b) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc5-114", 0x0000, 0x010000, CRC(63dbb45e) SHA1(3ccb45ae290b5e2d2249a36268a9e690846bf3d9) )
	ROM_LOAD( "sc5-115", 0x0000, 0x010000, CRC(8b3329b8) SHA1(2c89ac10ad82d8425104a74a7bc24aa9c28cbe01) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "mgamesnd", 0x0000, 0x040000, CRC(80ea7b3d) SHA1(a26dbc55ba205fc94c9b224c549516ba149627d7) )
ROM_END_M1A_MCU

ROM_START( m1monmon )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "mmm.bin", 0x0000, 0x020000, CRC(3b4eef7f) SHA1(4ad32a92eb2f31c1cefb1cf86f558dff3e8328bb) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "mmm10", 0x0000, 0x020000, CRC(a8526a02) SHA1(6f5e38c63d8c44df7e227e4014b89d05cfb8d3c0) )
	ROM_LOAD( "mmm10v", 0x0000, 0x020000, CRC(da39e2f8) SHA1(8e289d2fcfb5455e3ec14f98e9f918e074964a8c) )
	ROM_LOAD( "mmm15f", 0x0000, 0x020000, CRC(60f13207) SHA1(6cdd8a23e4b53f820a3b72dc728315b91193a786) )
	ROM_LOAD( "mmm15r", 0x0000, 0x020000, CRC(442c4386) SHA1(c610e1d8da1a55b6ccb618dce00db55ee8cfe039) )
	ROM_LOAD( "mmm221f", 0x0000, 0x020000, CRC(60f13207) SHA1(6cdd8a23e4b53f820a3b72dc728315b91193a786) )
	ROM_LOAD( "mmm510", 0x0000, 0x020000, CRC(ad61d45d) SHA1(4b66d5a2a33ffa62e6fb763ef4f2bfda2832a10e) )
	ROM_LOAD( "mmm55", 0x0000, 0x020000, CRC(f5506f09) SHA1(e4664a09f1a09f23363c07812cce93e9ac1acea4) )
	ROM_LOAD( "mmm5p815.bin", 0x0000, 0x020000, CRC(34d8eeb5) SHA1(d9202de6b2a291a055763e669c08e4b2b02c1e88) )
	ROM_LOAD( "mmm86f", 0x0000, 0x020000, CRC(a8526a02) SHA1(6f5e38c63d8c44df7e227e4014b89d05cfb8d3c0) )
	ROM_LOAD( "money510", 0x0000, 0x020000, CRC(ad61d45d) SHA1(4b66d5a2a33ffa62e6fb763ef4f2bfda2832a10e) )
	ROM_LOAD( "sa6-480", 0x0000, 0x020000, CRC(d10e676a) SHA1(a84252fdd78a982f16dc95caedf935ad6e6b109d) )
	ROM_LOAD( "sa6-481", 0x0000, 0x020000, CRC(814f99d6) SHA1(9d12439da761955c7e968e99bcb6ddf5ed770211) )
	ROM_LOAD( "sa6-590", 0x0000, 0x020000, CRC(8ff74ac1) SHA1(fec7b4d6b923ebcde406bb2b943cc6b162eda90c) )
	ROM_LOAD( "sa6-591", 0x0000, 0x020000, CRC(dfb6b47d) SHA1(ced1072695c873872cc663a1e567bfe9e01f7a9e) )
	ROM_LOAD( "sa7-300", 0x0000, 0x020000, CRC(acb52668) SHA1(5513860fcf8be181159d77678c1fc710357c91a2) )
	ROM_LOAD( "sa7-301", 0x0000, 0x020000, CRC(fcf4d8d4) SHA1(384ed15b73bdd019bef56301774e18a688632f46) )
	ROM_LOAD( "sa8-079", 0x0000, 0x020000, CRC(6b0f11c3) SHA1(290aa27939c204a79b3f708d2c13d98a3a099a85) )
	ROM_LOAD( "sa8-086", 0x0000, 0x020000, CRC(a8526a02) SHA1(6f5e38c63d8c44df7e227e4014b89d05cfb8d3c0) )
	ROM_LOAD( "sa8-087", 0x0000, 0x020000, CRC(f81394be) SHA1(d34d3a90f768cf3051e33939b8a5f9ff4a4029fc) )
	ROM_LOAD( "sa8-126", 0x0000, 0x020000, CRC(80e7bd56) SHA1(ee6fb3c9abf709e79d62ede086a1c7884a55faaa) )
	ROM_LOAD( "sa8-127", 0x0000, 0x020000, CRC(d0a643ea) SHA1(e0e96811dd4de29b2ecef09d95881fd967e74a89) )
	ROM_LOAD( "sa8-128", 0x0000, 0x020000, CRC(5109340d) SHA1(67e46edc3c98d514ef72ca5eb3e7cb5ec52c9703) )
	ROM_LOAD( "sa8-129", 0x0000, 0x020000, CRC(0148cab1) SHA1(b6d9161bb41a3e15023caee1ad7c2ee75f8759dc) )
	ROM_LOAD( "sa8-218", 0x0000, 0x020000, CRC(146dbd3a) SHA1(89c00f69877fad4bf943812d167f436f9a9d797a) )
	ROM_LOAD( "sa8-219", 0x0000, 0x020000, CRC(442c4386) SHA1(c610e1d8da1a55b6ccb618dce00db55ee8cfe039) )
	ROM_LOAD( "sa8-220", 0x0000, 0x020000, CRC(30b0ccbb) SHA1(c9ea32c6ff95e3d5f21650b824402457df7269a5) )
	ROM_LOAD( "sa8-221", 0x0000, 0x020000, CRC(60f13207) SHA1(6cdd8a23e4b53f820a3b72dc728315b91193a786) )
	ROM_LOAD( "sa9-015", 0x0000, 0x020000, CRC(a20db239) SHA1(437967107c18d647539eeabe17157dfcc9ab64a7) )
	ROM_LOAD( "sa9-016", 0x0000, 0x020000, CRC(b1ca896f) SHA1(782ca31e42e8b0c2f2b1b792ec21c859ed3cf7d2) )
//  ROM_LOAD( "mon55", 0x0000, 0x020000, CRC(f5506f09) SHA1(e4664a09f1a09f23363c07812cce93e9ac1acea4) ) // was in the monopoly set!

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "mmm.p1", 0x000000, 0x080000, CRC(687ccf90) SHA1(cd3fd5994b7809aa267eff419a54fddb38675947) )
	ROM_LOAD( "mmm.p2", 0x080000, 0x080000, CRC(da5a7d93) SHA1(107659124fabc2d8d7f91d8fe6d0e7d9d00bf2cb) )
ROM_END_M1A_MCU

ROM_START( m1monod )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "monopoly.bin", 0x0000, 0x010000, CRC(b7a2911b) SHA1(74507dd3a947d1b4d7bd0b58adb53d4f6e7ce200) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "monopolysnddutch.bin", 0x0000, 0x040000, CRC(8742981e) SHA1(1ba33c59ec5f878ebab111a77551213aad4b0993) )
ROM_END_M1A_MCU

ROM_START( m1mono )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "maygaymonopoly delx 5p-10p 6.bin", 0x0000, 0x010000, CRC(ed3c5997) SHA1(5a1e82894bd71073e08136a9071528833b529f5e) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "mon deluxe old 6 5_10.bin", 0x0000, 0x010000, CRC(f6ff2c55) SHA1(dec0be9c5584285b47943dd7d8751acd5e244daf) )
	ROM_LOAD( "mon20p10", 0x0000, 0x010000, CRC(25b617b9) SHA1(adffbae086c83c1d9342e0fdded0ec8651f4efdd) )
	ROM_LOAD( "mon5p5cbin", 0x0000, 0x010000, CRC(5589d97d) SHA1(d8776200d5c85fc1946ab4a4d0f7b7fb721a08f0) )
	ROM_LOAD( "mono10", 0x0000, 0x010000, CRC(25b617b9) SHA1(adffbae086c83c1d9342e0fdded0ec8651f4efdd) )
	ROM_LOAD( "mono10p", 0x0000, 0x010000, CRC(e7eead08) SHA1(94a512ff43487c4294afa3f280759ae86489ccbf) )
	ROM_LOAD( "mono8", 0x0000, 0x010000, CRC(e19cf85b) SHA1(24bfa0086bda37f9ca9b5cf4cdc7d7873c305e76) )
	ROM_LOAD( "monopoly 5p.bin", 0x0000, 0x010000, CRC(75125a1d) SHA1(20a16835b39d3eaa38c88ed885aef04bdb08bb65) )
	ROM_LOAD( "monopoly6jp.bin", 0x0000, 0x010000, CRC(4a5bc1a5) SHA1(09d576ef92ac03a1e5e5d852851414830ee7ebe9) )
	ROM_LOAD( "monopoly_5p10p_ndp_6pound-sa5-015.bin", 0x0000, 0x010000, CRC(03e359b7) SHA1(e469adaf5d0ed6e44c17a5881bbb42b391d680ee) )
	ROM_LOAD( "monopolygame.bin", 0x0000, 0x010000, CRC(07f82db9) SHA1(a46a4a62d7d4ca9fcf9674301f9960e04b879b20) )
	ROM_LOAD( "sa3-005", 0x0000, 0x010000, CRC(6aeecc7e) SHA1(2d2d077c26e01f35a7b76533a4d27a7266e1a1cb) )
	ROM_LOAD( "sa4-314", 0x0000, 0x010000, CRC(708a92ea) SHA1(0f5b9123e4356447215a6d76764c6a124a5206e2) )
	ROM_LOAD( "sa4-315", 0x0000, 0x010000, CRC(98620f0c) SHA1(a92f3ccf36dde20dc5ba8e655212f8a7d8888ce4) )
	ROM_LOAD( "sa4-316", 0x0000, 0x010000, CRC(b0c9b2d2) SHA1(4d6efe7453a398cd8f8a7290a4ea13ee1027425c) )
	ROM_LOAD( "sa4-317", 0x0000, 0x010000, CRC(58212f34) SHA1(04ee3037861ca97d874fa24954ce3c76fca7e815) )
	ROM_LOAD( "sa5-035", 0x0000, 0x010000, CRC(a4e07340) SHA1(c566e5ddb780755a8e16454eec8aecfa8d4130a6) )
	ROM_LOAD( "sa5-036", 0x0000, 0x010000, CRC(1058ad93) SHA1(2a53a5af56e148526ddc47deb49bf3d5aa3063b5) )
	ROM_LOAD( "sa6-005", 0x0000, 0x010000, CRC(6b3655bb) SHA1(cd600a6a1730765bf271cc195343712fb2ba3ad7) )
	ROM_LOAD( "sa6-007", 0x0000, 0x010000, CRC(ac7c3a2f) SHA1(50d734a2b58d78bfe3fce6455caca0524c464d7e) )
	ROM_LOAD( "sa6-008", 0x0000, 0x010000, CRC(0a546803) SHA1(6122bf392f37c420cf194ff3b21d5b1e615c437f) )
	ROM_LOAD( "sa6-009", 0x0000, 0x010000, CRC(07f82db9) SHA1(a46a4a62d7d4ca9fcf9674301f9960e04b879b20) )
	ROM_LOAD( "sa6-010", 0x0000, 0x010000, CRC(876ed0d1) SHA1(6af96a7e0ff2757836741054f19cfe3469bc0bb5) )

	// BWB sets?
	ROM_LOAD( "mono5p", 0x0000, 0x010000, CRC(805d33e8) SHA1(4d946ee37f8d25f0f7bb3497a538974e1bae14b1) )
	ROM_LOAD( "mox3&1", 0x0000, 0x010000, CRC(8a790928) SHA1(91594136eb133b6112beb036a973756d0b54b648) )
	ROM_LOAD( "sa4-205.bin", 0x0000, 0x010000, CRC(7461f169) SHA1(5d54d259d252fddd0c222790cdcd098267a8f9a7) )
	ROM_LOAD( "sa4-206.bin", 0x0000, 0x010000, CRC(c0d92fba) SHA1(c6ef33c26bbdd522107622235f4d429d4043fe9e) )
	ROM_LOAD( "sa4-207.bin", 0x0000, 0x010000, CRC(de05924e) SHA1(fa5ff4383161989481765c8cbf73322d180104ed) )
	ROM_LOAD( "sa4-208.bin", 0x0000, 0x010000, CRC(782dc062) SHA1(bd43e37b02db247f0c5c41c273dc67a8b80ca7d9) )
	ROM_LOAD( "sa4-211.bin", 0x0000, 0x010000, CRC(faac3556) SHA1(13b7dd369994669eb971807620c0434ce425f918) )
	ROM_LOAD( "sa4-212.bin", 0x0000, 0x010000, CRC(4e14eb85) SHA1(eb26b5593e250d0b508b346821ee7591d800dacf) )
	ROM_LOAD( "sa4-601", 0x0000, 0x010000, CRC(1b5ad34d) SHA1(4f32fa4c8a667b19b09e5b4e4a1dee2e2f8b4bf0) )
	ROM_LOAD( "sa4-602", 0x0000, 0x010000, CRC(afe20d9e) SHA1(5f1648443c46138d1256f8da92179b2c58ddfd4b) )
	ROM_LOAD( "sa4-603", 0x0000, 0x010000, CRC(98fe65f2) SHA1(4944291118d5ed7a642d15441bf7e67ab489ffda) )
	ROM_LOAD( "sa4-604", 0x0000, 0x010000, CRC(94e63d4b) SHA1(81c549ebc8b28c005fdfc430d49e548403d72ce0) )
	//ROM_LOAD( "sa5-015.asv", 0x0000, 0x0000db, CRC(76eb74b3) SHA1(d04ee03bbaa1e53ef4c07ea703ea0607bf2b9314) )
	ROM_LOAD( "sa5-016", 0x0000, 0x010000, CRC(b75b8764) SHA1(91513702719d6a3049f264c5e547b8a69058b82c) )
	ROM_LOAD( "sa5-184", 0x0000, 0x010000, CRC(51a8fde3) SHA1(c73441f79b877aaed57791629de9f9b02ff544da) )
	ROM_LOAD( "sa5-185", 0x0000, 0x010000, CRC(b9406005) SHA1(f1b025140ba7579b1eeb497a64c348df5a341d5d) )


	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  ) // the sound roms for the SWP video version were in here too...
	ROM_LOAD( "monopolysnd.bin", 0x0000, 0x020000, CRC(f93ef281) SHA1(b2c2bf361c44499a13731d494af66d2aa45ccebd) )
ROM_END_M1A_MCU


ROM_START( m1moncls )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "classicmonopoly(maygay)programromsa3-316.bin", 0x0000, 0x010000, CRC(98f9d6b0) SHA1(f30fa1fb88bfd4098b189cf03a7e0b9dcc5bfdef) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-318.bin", 0x0000, 0x010000, CRC(01e8224c) SHA1(dfad509c00d6311eb1e8bbbfe1ca5fd6aeb9da43) )
	ROM_LOAD( "sa3-319.bin", 0x0000, 0x010000, CRC(e900bfaa) SHA1(ce6f1021234979d0c27d5668b470fd31594ca222) )
	ROM_LOAD( "sa4-120", 0x0000, 0x010000, CRC(c594d56d) SHA1(4a48f3d80c575025de7624528647891c179c1b0d) )
	ROM_LOAD( "sa4-121", 0x0000, 0x010000, CRC(2d7c488b) SHA1(ee63973447cb21fa2872ed74612f431add2b7a46) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "classicmonopoly(maygay)soundromdig1-027.bin", 0x0000, 0x040000, CRC(d5243b51) SHA1(c7e3a61071c566e8ea9c8842839b70242ca67308) )
ROM_END_M1A_MCU


ROM_START( m1monclb )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "mon.bin", 0x0000, 0x020000, CRC(9941e3ee) SHA1(2abae2333e35e55510eb73da03c8b385960a5d26) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "mono250", 0x0000, 0x020000, CRC(c7b07773) SHA1(056c97adf93edaadd6174304136273122e76577c) )
	ROM_LOAD( "sc7-002", 0x0000, 0x020000, CRC(418de7f0) SHA1(a80c13a96d101cf8e94a342e1cece3cf4f0c72e4) )
	ROM_LOAD( "sc7-003", 0x0000, 0x020000, CRC(11cc194c) SHA1(36df2ac0b809bb6f8657c8d6edf9e8312dc9bb5e) )
	ROM_LOAD( "sc7-415", 0x0000, 0x020000, CRC(9b62636e) SHA1(5bd54b7c8709c4bac8b5afe8d40eeffa633cef71) )
	ROM_LOAD( "sc7-416", 0x0000, 0x020000, CRC(88a55838) SHA1(3cf3eae6aaab082114a18b2ec0f90eb26a9429f1) )
	ROM_LOAD( "sc7-521", 0x0000, 0x020000, CRC(c9001d52) SHA1(2b9181e66ebc25bc00abbff6369f0238590da0a7) )
	ROM_LOAD( "sc7-547", 0x0000, 0x020000, CRC(3b074e23) SHA1(aff0a2450aba1848f77a319dd655e452e5e44717) )
	ROM_LOAD( "sc7-548", 0x0000, 0x020000, CRC(7aa7ef48) SHA1(45660b8e82d499487db1d88b3d2031dfa1fd6fb2) )
	ROM_LOAD( "sc8-038", 0x0000, 0x020000, CRC(7f227fe0) SHA1(636ffcae599c9cc8e1f9894c67e4e2f846a7c980) )
	ROM_LOAD( "sc8-092", 0x0000, 0x020000, CRC(37edc96a) SHA1(3d6f8f5bc2340890798bb29f6fc98f6e284c7aad) )
	ROM_LOAD( "sc8-308", 0x0000, 0x020000, CRC(3f438e55) SHA1(ea07d462088cfb48e6c9a91a43313812d44b045c) )
	ROM_LOAD( "sc8-309", 0x0000, 0x020000, CRC(6f0270e9) SHA1(87c2f653a7297801410120de94618c71cb997efa) )
	ROM_LOAD( "monclub", 0x0000, 0x020000, CRC(0604d7a4) SHA1(fa475fcfc91f9962280329855730fb00c8f421c3) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
//  ROM_LOAD( "sound.u2", 0x0000, 0x080000, CRC(f7303657) SHA1(a66c1110b38d36ebbd0ec3a2d3a08d9164b9d4a0) )
//  ROM_LOAD( "sound.u3", 0x0000, 0x080000, CRC(3881a59a) SHA1(e7f13152ff4c8e502bd97ea8ab90416126de73f3) )
	ROM_LOAD( "monsnd1.bin", 0x0000, 0x080000, CRC(f7303657) SHA1(a66c1110b38d36ebbd0ec3a2d3a08d9164b9d4a0) )
	ROM_LOAD( "monsnd2.bin", 0x0000, 0x080000, CRC(3881a59a) SHA1(e7f13152ff4c8e502bd97ea8ab90416126de73f3) )
ROM_END_M1A_MCU


ROM_START( m1monstr )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-549", 0x0000, 0x020000, CRC(8eaab93c) SHA1(a14a4b68994594df1e20695b056102db52dd33d1) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-550", 0x0000, 0x020000, CRC(f72fa0c7) SHA1(8eecd458d58a4ccc58a42bc149737c9c335fbfec) )
	ROM_LOAD( "sa7-553", 0x0000, 0x020000, CRC(703a437a) SHA1(e2b5b6ba1c5b73d84f403b0f2c40e5e7bbcc435a) )
	ROM_LOAD( "sa7-554", 0x0000, 0x020000, CRC(e4f0f3f8) SHA1(152728dec092e20e102239d396cfa9525c7d92cf) )
ROM_END_M1A_MCU


ROM_START( m1nhp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "noel.bin", 0x0000, 0x010000, CRC(b747e835) SHA1(98faaff432182adaf8ae7249187ac0e81bfe10d8) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "noel10", 0x0000, 0x010000, CRC(84ad7b31) SHA1(e350a1806c05215e0a60f8dc724345b6b1a0be12) )
	ROM_LOAD( "noels-8s.p1", 0x0000, 0x010000, CRC(1e2d92d3) SHA1(467b49234f4498a5ab2fc01bcf9d4982234e36cb) )
	ROM_LOAD( "sa5-216", 0x0000, 0x010000, CRC(46e9e772) SHA1(098c23b8676b53dce6087c7812eb45a31e1cad0d) )
	ROM_LOAD( "sa5-217", 0x0000, 0x010000, CRC(ae017a94) SHA1(66358ddcc01a508c724e8323236fa6d4466776ba) )
	ROM_LOAD( "sa5-218", 0x0000, 0x010000, CRC(d8b49c71) SHA1(fa19b503f036490321786ee229502f2506aaaf3f) )
	ROM_LOAD( "sa5-219", 0x0000, 0x010000, CRC(305c0197) SHA1(db47abc5036df1ea85a9ef1516976762be82ca9e) )
	ROM_LOAD( "sa5-243", 0x0000, 0x010000, CRC(5faf75d3) SHA1(bc6f2bf5d5da15d330298d4094ec088f4ba8ae32) )
	ROM_LOAD( "sa5-366", 0x0000, 0x010000, CRC(1e2d92d3) SHA1(467b49234f4498a5ab2fc01bcf9d4982234e36cb) )
	ROM_LOAD( "sa5-367", 0x0000, 0x010000, CRC(f6c50f35) SHA1(4c7b6f85429212946d6ccab27e77214b52f458f5) )
	ROM_LOAD( "sa5-368", 0x0000, 0x010000, CRC(7b64a929) SHA1(4de9951a76a8fa7e858efe40fa202e5d6eebdce3) )
	ROM_LOAD( "sa5-369", 0x0000, 0x010000, CRC(938c34cf) SHA1(ecff6c8b20fba39301d1d137767ffcdcbbc7f2e1) )
	ROM_LOAD( "sa5-371", 0x0000, 0x010000, CRC(f7ce27b5) SHA1(859176f0e5d6b2c839293b9542df9abf5790e2d7) )
	ROM_LOAD( "sa6-628", 0x0000, 0x010000, CRC(afbc389c) SHA1(d19b7b82e54dd7e846a8f1255d6d8de298f71b31) )
	ROM_LOAD( "sa6-629", 0x0000, 0x010000, CRC(4754a57a) SHA1(759b9b527f5bade7abf8079fb61c77690af94e92) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "noelshouseparty.p1", 0x000000, 0x080000, CRC(aa8e62e4) SHA1(719cb6d5c2b3ffca3952c01e143f6096207b2520) )
	ROM_LOAD( "noelshouseparty.p2", 0x080000, 0x080000, CRC(e8a06839) SHA1(7452f48355c64e0424879d53d87fa523953eec0a) )
ROM_END_M1A_MCU


ROM_START( m1nudbnk )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "naban510", 0x0000, 0x020000, CRC(70c25167) SHA1(8aeeaecbc9fdf44bef6d4bf32ab3c36050c6d812) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "nban510", 0x0000, 0x020000, CRC(70c25167) SHA1(8aeeaecbc9fdf44bef6d4bf32ab3c36050c6d812) )
	ROM_LOAD( "nban55", 0x0000, 0x020000, CRC(a2d7b81c) SHA1(4ecd0f56ff6e043d1cff3c9a2baf0ebec2bf20d2) )
	ROM_LOAD( "sa6-436", 0x0000, 0x020000, CRC(1566d056) SHA1(4ea5927e1f5fb5d58072e1618d6988eab2fd6158) )
	ROM_LOAD( "sa6-437", 0x0000, 0x020000, CRC(45272eea) SHA1(903eaa4ee405b66a1b81b770935fcd24e25b8b85) )
	ROM_LOAD( "sa6-525", 0x0000, 0x010000, CRC(ccce8ada) SHA1(bed46cfd6f63685a5aaf013ce58b9c67b5e0bc26) )
	ROM_LOAD( "sa6-526", 0x0000, 0x010000, CRC(78765409) SHA1(d496406db10119963245bab06ef603231db151e8) )
	ROM_LOAD( "sa6-529", 0x0000, 0x010000, CRC(aba06cb3) SHA1(8944b48038fda54aecbab4e5420d425828d316d0) )
	ROM_LOAD( "sa6-530", 0x0000, 0x010000, CRC(2b3691db) SHA1(ecd730efc1c7308d4305dfebb446be37bbd1fd67) )
	ROM_LOAD( "sa6-531", 0x0000, 0x010000, CRC(e46334a0) SHA1(8bba3a9dc90465c8e5f4430c822457fb5aeba45e) )
	ROM_LOAD( "sa6-532", 0x0000, 0x010000, CRC(50dbea73) SHA1(ff93858f53b6025b4396f2d7aa4e14dea3445129) )
	ROM_LOAD( "sa6-533", 0x0000, 0x020000, CRC(db4115b0) SHA1(19e92c0e3b1e63e8eef2b92b8baba84c6531e99a) )
	ROM_LOAD( "sa6-534", 0x0000, 0x020000, CRC(4f8ba532) SHA1(6e24c304b2ef3c7a0207f4a471e540aff3595313) )
	ROM_LOAD( "sa7-470", 0x0000, 0x020000, CRC(bc0e16e5) SHA1(6223c4f5cd2c1dc8469dacc75874b31d178468fb) )
	ROM_LOAD( "sa7-471", 0x0000, 0x020000, CRC(ec4fe859) SHA1(425cccdf88ade50cab3f9ab8b463cb139de4cb5b) )
	ROM_LOAD( "sa8-172", 0x0000, 0x020000, CRC(306783a9) SHA1(ab9675ba48e4d72f313c115970c3fd6789415662) )
	ROM_LOAD( "sa8-173", 0x0000, 0x020000, CRC(60267d15) SHA1(312290db2c35c8cdc2ba3495d72614fb736834e2) )
	ROM_LOAD( "sa8-174", 0x0000, 0x010000, CRC(0aba9710) SHA1(53801823d65cb796112e24d1f2d98a85f4ea3adf) )
	ROM_LOAD( "sa8-175", 0x0000, 0x010000, CRC(e2520af6) SHA1(27a171b480586ac41262e16e365cadb29e8f9017) )
	ROM_LOAD( "sa9-011", 0x0000, 0x020000, CRC(e170f4ed) SHA1(525a12187918c9a8bc8040c8dfde9007264da708) )
	ROM_LOAD( "sa9-012", 0x0000, 0x020000, CRC(f2b7cfbb) SHA1(0d0be0438679f3955b6f1f96d536279028f71b60) )
	ROM_LOAD( "sa9-013", 0x0000, 0x020000, CRC(92de377e) SHA1(3ebf9a81fb709df84221d9a9f7dcc2eb9bfb188f) )
	ROM_LOAD( "sa9-014", 0x0000, 0x020000, CRC(061487fc) SHA1(db80449154d1add4af6e78416d25fc4dd60916c0) )
	ROM_LOAD( "15nbro", 0x0000, 0x020000, CRC(fff4b810) SHA1(0dc941d54853eb9c3e26210e1f4032d7a6de3dc6) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "nbanu2", 0x000000, 0x080000, CRC(4d5ef011) SHA1(f19ae0b958f08ccd9a40e91719e096bad1ae1d0e) )
	ROM_LOAD( "nbanu3", 0x080000, 0x080000, CRC(c2ea4a53) SHA1(74b6144f22903565ac3526c235a75bd85fe49256) )
ROM_END_M1A_MCU


ROM_START( m1nudunl )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-650", 0x0000, 0x020000, CRC(bfb700d8) SHA1(8d09812287ff207f87887215f265954debbe9f2b) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-651", 0x0000, 0x020000, CRC(eff6fe64) SHA1(448170ed3d36e5b9fbfc99c710f2b2d948291d44) )
	ROM_LOAD( "sa6-652", 0x0000, 0x020000, CRC(ce0d8e53) SHA1(31ff918d3baa34e318f4e3895e9ce2182d8841ce) )
	ROM_LOAD( "sa6-653", 0x0000, 0x020000, CRC(9e4c70ef) SHA1(90ba28a4de87d025ec869af4a44e4f638eaddd88) )
	ROM_LOAD( "sa7-007", 0x0000, 0x020000, CRC(f50d2357) SHA1(355223dae2180f79c41fbd756cf4f7fd7582cfc3) )
	ROM_LOAD( "sa7-008", 0x0000, 0x020000, CRC(b4ad823c) SHA1(0b890a808338843fabb5b449fd792c3eaa82e837) )
ROM_END_M1A_MCU


ROM_START( m1omega )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-089.bin", 0x0000, 0x010000, CRC(ada5f2ae) SHA1(190813e17460acd1f43606da08ac50e0e0fe2108) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-090.bin", 0x0000, 0x010000, CRC(2d330fc6) SHA1(a74a83a3824e6593fed8ebfd341151f56c1f47fa) )
ROM_END_M1A_MCU


ROM_START( m1onbus )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "onbuses10p10.bin", 0x0000, 0x020000, CRC(27765bd8) SHA1(890d4fc934b0e1d66a52896f176b086453909332) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "otbgame", 0x0000, 0x020000, CRC(48e1bdea) SHA1(bca684ae6def40ff5ba8cfda8ee90dcf6cda6e13) )
	ROM_LOAD( "sa7-579", 0x0000, 0x020000, CRC(ed23e05c) SHA1(b798b71319f05ebae977696e3b38a9bdff24fca8) )
	ROM_LOAD( "sa7-580", 0x0000, 0x020000, CRC(7a6068f4) SHA1(346f3568a3d7208f0413710d60d05c27575c1276) )
	ROM_LOAD( "sa7-674", 0x0000, 0x020000, CRC(ad6c9bc6) SHA1(ade618002c6dfa9de48331c394487964134fe810) )
	ROM_LOAD( "sa7-675", 0x0000, 0x020000, CRC(fd2d657a) SHA1(02e43821aca7682adab1121fb538435f834551d5) )
	ROM_LOAD( "sa7-728", 0x0000, 0x020000, CRC(09411c81) SHA1(8e6b0b093c251df3a2ccca2c0d376090c9e67a8a) )
	ROM_LOAD( "sa7_728", 0x0000, 0x010000, CRC(b942cf3a) SHA1(bb1ddcc3d03237fb34caa7891a556afd7cf1823d) )
	ROM_LOAD( "sa9-003", 0x0000, 0x020000, CRC(93874b8b) SHA1(2afc397ec907e212313e55d4e2a53e0e3039ce1d) )
	ROM_LOAD( "sa9-004", 0x0000, 0x020000, CRC(074dfb09) SHA1(d8cae78f3e74ceef49c11992bb7f62bcf6236898) )
	ROM_LOAD( "sa9-005", 0x0000, 0x020000, CRC(c132a9be) SHA1(fe5fd99e4e8a5d88b6d0ce1d9093e8143163d261) )
	ROM_LOAD( "sa9-006", 0x0000, 0x020000, CRC(d2f592e8) SHA1(37d9b42b54c6d1bca20f3316ff1df7c439aef235) )
	ROM_LOAD( "sa9_006", 0x0000, 0x010000, CRC(ce8f403f) SHA1(ec71a0abe659c88c31c66b8f189c34978ee35fa1) )
	ROM_LOAD( "sk991069", 0x0000, 0x020000, CRC(ecc6df02) SHA1(b86399b6cdb802caaf865bb8715b8a772398e733) )
	ROM_LOAD( "sk991070", 0x0000, 0x020000, CRC(c1416ca8) SHA1(c5333b107de8be5bff5c5b403dbdf82fe86a6c6c) )

	ROM_LOAD( "15otbfo", 0x0000, 0x020000, CRC(3d23c94b) SHA1(42a823e8b3e3e1e004a835a7c018369e30c4a4a6) )
	ROM_LOAD( "15otbro", 0x0000, 0x020000, CRC(6f962b7e) SHA1(98b56de3186d2de4fea7500bcd4dfd12f3367ce7) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "otbsnd1.bin", 0x000000, 0x080000, CRC(c1868f4a) SHA1(606c5dbbea6f681485922ea498846a8b2d94eb4f) )
	ROM_LOAD( "otbsnd2.bin", 0x080000, 0x080000, CRC(aa60f721) SHA1(030a5a39ed9212c63a025dbdd9e79a86c77f4921) )
ROM_END_M1A_MCU


ROM_START( m1ott )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-733", 0x0000, 0x020000, CRC(0c9a68dd) SHA1(1be9a9a91d5a00c07693777a92c312605f4cd5aa) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-734", 0x0000, 0x020000, CRC(9850d85f) SHA1(a155a13a4f240f455a990769140d8b2eabcb88ed) )
ROM_END_M1A_MCU


ROM_START( m1piggy )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-277.bin", 0x0000, 0x010000, CRC(699d9bba) SHA1(400c419d62bc204c66b71f8dcda71b6b77bbc274) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-278.bin", 0x0000, 0x010000, CRC(cfb5c996) SHA1(7798ce05f24e064786dd4866f16f4fdf79ca026e) )
	ROM_LOAD( "sa3-279.bin", 0x0000, 0x010000, CRC(ed6a592d) SHA1(de667c0da3f66ac84f8edf52a57de55e6f4a3409) )
	ROM_LOAD( "sa3-280.bin", 0x0000, 0x010000, CRC(290c9d41) SHA1(151cfea6b56954ae1f9e53aba68c6ec9d309411e) )
ROM_END_M1A_MCU


ROM_START( m1ppd )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "pinkpanther.bin", 0x0000, 0x010000, CRC(09040f9d) SHA1(25c545e599711bf5ff2361c51e6629b69673da33) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "ppsound.bin", 0x0000, 0x040000, CRC(8742981e) SHA1(1ba33c59ec5f878ebab111a77551213aad4b0993) )
ROM_END_M1A_MCU


ROM_START( m1pp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "pink2010", 0x0000, 0x010000, CRC(a098952f) SHA1(d62351d16aa6f34b20774dd6f38ffdada09b49be) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "pink58c", 0x0000, 0x010000, CRC(fb0ee333) SHA1(3af5362486de70971f606dd914f8e658015dcf82) )
	ROM_LOAD( "pink5p5bin", 0x0000, 0x010000, CRC(97f6cda4) SHA1(338362a2dc0538feea08c98c27af2aec1ec46c08) )
	ROM_LOAD( "pinkp.bin", 0x0000, 0x010000, CRC(659d8067) SHA1(86cdeab73dd610dc9a0c69b95e74cf2d9bb36830) )
	ROM_LOAD( "pinkp206", 0x0000, 0x010000, CRC(baf9dbe2) SHA1(5293294abd0713aff969012d818dd6e8b637b74a) )
	ROM_LOAD( "pinkp8s", 0x0000, 0x010000, CRC(aaf4c565) SHA1(e2d04852e3700b3d19e5245aec2ce57725c728cb) )
	ROM_LOAD( "pinkpanther8key20p.bin", 0x0000, 0x010000, CRC(6888e768) SHA1(8b1f076ad6905367956097f7ac2ca3d49a3ed97f) )
	ROM_LOAD( "ppanthersa3-026 nd.bin", 0x0000, 0x010000, CRC(189bc301) SHA1(2b9a190e3b5e008b291d0f4b14fa5874c6f867c4) )
	ROM_LOAD( "sa3-122.bin", 0x0000, 0x010000, CRC(3bc43552) SHA1(a8c6f314de3148b1b75fb89816e26ac07ee2ef93) )
	ROM_LOAD( "sa3-123.bin", 0x0000, 0x010000, CRC(d32ca8b4) SHA1(ebf64436e10f9eb40b938f55845b82e70753fef4) )
	ROM_LOAD( "sa4-060.bin", 0x0000, 0x010000, CRC(bae6af78) SHA1(2ac64db6cbbf41314787db0c6a14666d0a16aa63) )

	ROM_LOAD( "sa4-137.bin", 0x0000, 0x010000, CRC(64a22d91) SHA1(e1d08950da85bf5e1146633d53ab8149d639c3e1) )
	ROM_LOAD( "sa4-139", 0x0000, 0x010000, CRC(e7a170dc) SHA1(120b136b54401755a8ef169ec9f2a510a9b01899) )
	ROM_LOAD( "sa4-140", 0x0000, 0x010000, CRC(8701fb2d) SHA1(003f24baa72d56eae9027af557592ccf8c150fa2) )
	ROM_LOAD( "sa4-612", 0x0000, 0x010000, CRC(d7d4e529) SHA1(f41bf31b68421010d6ab507da250e8eefdda2376) )
	ROM_LOAD( "sa4-613", 0x0000, 0x010000, CRC(3f3c78cf) SHA1(bf2c84bf81c0bd9d32a2c3ca3eef132efed6baea) )
	ROM_LOAD( "sa4-614", 0x0000, 0x010000, CRC(632e1f05) SHA1(1fbae16a0addc8baf10f6332860bb98a56b0e94a) )
	ROM_LOAD( "sa4-615", 0x0000, 0x010000, CRC(8bc682e3) SHA1(39f2eb7e4e9c36f64b0efe2b347f93638d9b6e09) )
	// BWB Panther
	ROM_LOAD( "ppix6___.281", 0x0000, 0x010000, CRC(386c7d02) SHA1(67a52332a76d82909455174cce5067469654ccd0) )


	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "digi16.bin", 0x0000, 0x040000, CRC(ee8bc3ea) SHA1(b58fad236055db30a75bb12946e8ad76638865a0) )
ROM_END_M1A_MCU


ROM_START( m1ppc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "clubpp.bin", 0x0000, 0x010000, CRC(8df9165a) SHA1(cdd63e4824273eb242d6c579cc88c6b95cb319fa) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc6-396", 0x0000, 0x010000, CRC(4fa5db01) SHA1(ed253ebfa3dc5ff4ac3d547fc5068e358263367e) )
	ROM_LOAD( "sc6-397", 0x0000, 0x010000, CRC(a74d46e7) SHA1(b0438c45685ae8a6c46b4b99a1710d8428e009ff) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "clbppsd1.bin", 0x000000, 0x080000, CRC(bb3fe409) SHA1(9dafc470a0bc2d8a2b0e13c44fc81d7e3c905001) )
	ROM_LOAD( "clbppsd2.bin", 0x080000, 0x080000, CRC(abbe0f93) SHA1(1195c00a0ac917806316652c4ec5c0717d02876f) )
ROM_END_M1A_MCU


ROM_START( m1przclu )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-307.bin", 0x0000, 0x010000, CRC(c2b6567e) SHA1(08eee2400bee603dc4fbbe578f1d25481787024b) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-408", 0x0000, 0x010000, CRC(01ff19da) SHA1(5673aef67b017704fbeb4fe66def843db182728b) )
	ROM_LOAD( "sa4-409", 0x0000, 0x010000, CRC(e917843c) SHA1(9160588e2fa6e842a51b18b9b0d0c3579c5c637c) )
ROM_END_M1A_MCU


ROM_START( m1przee )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "prize eastenders 6.bin", 0x0000, 0x010000, CRC(3cc35697) SHA1(adaaafd6ae59497630660ec179212f0dd8dd3741) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "prizeeastend.bin", 0x0000, 0x010000, CRC(3cc35697) SHA1(adaaafd6ae59497630660ec179212f0dd8dd3741) )
	ROM_LOAD( "sa4-186", 0x0000, 0x010000, CRC(6e48b9e6) SHA1(bec5ad1ca06175c4217bbe3f6caae0061c7bfe99) )
	ROM_LOAD( "sa4-556", 0x0000, 0x010000, CRC(0f6aeadd) SHA1(ed613d064c9991eed913f9579ea18a9ac70c2cf7) )
	ROM_LOAD( "sa5-058", 0x0000, 0x010000, CRC(3cc35697) SHA1(adaaafd6ae59497630660ec179212f0dd8dd3741) )
	ROM_LOAD( "sa5-059", 0x0000, 0x010000, CRC(d42bcb71) SHA1(483078ada4dcc9b05d47c1ad6011bffc3a090c47) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "dig1-033 snd u2.bin", 0x000000, 0x080000, CRC(3eaa138c) SHA1(ad7d6e3ffc8fe19ea8cb9188998c75c90a77e09e) )
	ROM_LOAD( "dig1-033 snd u3.bin", 0x080000, 0x080000, CRC(89fde428) SHA1(f0942a2f1d3890ad18b01e8433333e5412c57644) )
ROM_END_M1A_MCU

ROM_START( m1reeldm )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-475.bin", 0x0000, 0x010000, CRC(ec0b7209) SHA1(e5e14ef87b9ef8035817af6c933809a972fa28f3) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-480.bin", 0x0000, 0x010000, CRC(3afd3a9a) SHA1(f96c9831cffeacff390ffacb85d4c80cb5a90d99) )
	ROM_LOAD( "sa4-498", 0x0000, 0x010000, CRC(55554107) SHA1(c0c0f4e7fe721d4bf86458090c03d712d5e86e16) )
	ROM_LOAD( "sa4-499", 0x0000, 0x010000, CRC(515d2a7a) SHA1(1e6e8364536fbb373a1c6ae818203eaa7f6494b1) )
	ROM_LOAD( "sa4-504", 0x0000, 0x010000, CRC(897517ab) SHA1(3d8ce2d96d37accf4fb7a2215882894185b53f30) )
	ROM_LOAD( "sa4-547", 0x0000, 0x010000, CRC(f89c3d41) SHA1(5ccf9958ff15aa2bc1584ec644cec157c7cf3379) )
	ROM_LOAD( "sa5-357", 0x0000, 0x010000, CRC(118615a8) SHA1(14966b061c90f1ec33972f6e00951a41638f65aa) )
	ROM_LOAD( "sa5-358", 0x0000, 0x010000, CRC(b7ae4784) SHA1(b1bcafc77d83fe9714ba1dfc81cc2ba95f923d2e) )
	ROM_LOAD( "sa5-359", 0x0000, 0x010000, CRC(458ee595) SHA1(8258986b2b8b969ec2b71e14f05b43cc4c394e0b) )
	ROM_LOAD( "sa5-360", 0x0000, 0x010000, CRC(f8d1f2e2) SHA1(399b6076e9e1b9f4f9abce48e6a3110b336524fa) )
	ROM_LOAD( "sa5-361", 0x0000, 0x010000, CRC(6c977f3e) SHA1(cb1da0bce76d60ffe1257585cd3225a4e19754f7) )
	ROM_LOAD( "sa6-286", 0x0000, 0x010000, CRC(af6e10f4) SHA1(2e2d4002347f6ace260aa753824bb438043cfd4c) )
	ROM_LOAD( "sa6-287", 0x0000, 0x010000, CRC(47868d12) SHA1(43f15f73187ac3de2dac0c317c4ce63b27de0509) )
	ROM_LOAD( "sa6-289", 0x0000, 0x010000, CRC(e9d5db67) SHA1(0e0535108e951f77e36d1d929c8e8ba4e3bc71b2) )
	ROM_LOAD( "sa6-537", 0x0000, 0x010000, CRC(53694375) SHA1(b35a70fdd4c9cbb81747f4c499d21097bed72a52) )
	ROM_LOAD( "sa6-538", 0x0000, 0x010000, CRC(f5411159) SHA1(4926d3755c7347e109c0f33bd29db7677eed81a5) )
ROM_END_M1A_MCU

ROM_START( m1retpp )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "pinkpstd.bin", 0x0000, 0x010000, CRC(92bb56d8) SHA1(e033578c693f0faf1e91b76392106f0e6850d0dc) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-519.bin", 0x0000, 0x010000, CRC(b8e04479) SHA1(3be940b433174623d177ffd892d8bc59170422b8) )
	ROM_LOAD( "sa3-520.bin", 0x0000, 0x010000, CRC(679b6b66) SHA1(a21e82221da54ba48a43c68d5c2c2f07ee9c2f34) )
	ROM_LOAD( "sa4-044.bin", 0x0000, 0x010000, CRC(9ea30e61) SHA1(1812582ac4f6069354e0f1b5a8f5bd1981cd6e8f) )
	ROM_LOAD( "sa4-230", 0x0000, 0x010000, CRC(42cd661a) SHA1(b5f5e3e9898155e8696eb97a7cf5e1855e190be1) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "rotppsnd.bin", 0x0000, 0x002000, CRC(a8c8ff9a) SHA1(8069cf08f3a8481ebc589ad0c25887ea316facd5) ) // looks like a bad dump of rom below
	ROM_LOAD( "roppsnd.bin", 0x0000, 0x040000, CRC(9f3484b3) SHA1(9d454644c967b22cf6583335807a0ed8495492cb) )
ROM_END_M1A_MCU


ROM_START( m1search )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "search.bin", 0x0000, 0x010000, CRC(e39e12e6) SHA1(e44a72a5fdb825be362a17a0db599a7579238423) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sl2p_3", 0x0000, 0x010000, CRC(14c5f609) SHA1(26f41ee6697ac22cb91e092303d1916bd2441514) )
	ROM_LOAD( "sl5p6ac", 0x0000, 0x010000, CRC(f92143fd) SHA1(c0e1645eb4dc247fc4ba1da3a915288ba3ba9798) )
ROM_END_M1A_MCU

ROM_START( m1sixspn )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sixspinner.bin", 0x0000, 0x010000, CRC(7cd08769) SHA1(2e3fa49b745173f1e9db7d6415e14acf6319dbf0) )
ROM_END_M1A_MCU

ROM_START( m1spid )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-013", 0x0000, 0x010000, CRC(e006350e) SHA1(26fb5839c6db0c8493450914157c986f41638184) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-014", 0x0000, 0x010000, CRC(ec1e6db7) SHA1(4c4e1f58d13703d1a16646b72328eb8eb96bc632) )
ROM_END_M1A_MCU

ROM_START( m1spid2 )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-024", 0x0000, 0x010000, CRC(15d44853) SHA1(e6c16dafde9f48c7c6956a6888d9877c63a17fa3) )
ROM_END_M1A_MCU

ROM_START( m1sptlgt )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa2-412.bin",0x0000, 0x10000,  CRC(17531aad) SHA1(decec517b89be9019913be59c5fC2aa2ee6e3f8f))

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa2-397", 0x0000, 0x010000, CRC(e5092767) SHA1(0205b6147e31ab0ff326c9b63d0bbc2fe5b57d20) )
	ROM_LOAD( "spt02__2.2.bin", 0x0000, 0x010000, CRC(2a04698c) SHA1(e2bbf91b699349f9a76bb2da7d4b47dc1d259a22) )
	ROM_LOAD( "spt2p", 0x0000, 0x010000, CRC(eb4fa923) SHA1(60d865fb81be33d4537dd5a24a44274a57ab582a) )

	ROM_LOAD( "spotbwb.bin", 0x0000, 0x010000, CRC(288a4462) SHA1(8ade1b87d586591a9543a0400f140dd38ae2206a) )
	ROM_LOAD( "spt05___.1_1", 0x0000, 0x010000, CRC(0e77cdc4) SHA1(7edfc1498768461883e943cf7b50869791a5e0d2) )
ROM_END_M1A_MCU

ROM_START( m1stair )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sth5.4", 0x0000, 0x010000, CRC(879c8dcb) SHA1(358d9bb567da4b7913434d29dcd8a81c51c5fe2e) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sth5.8c", 0x0000, 0x010000, CRC(7ce6b760) SHA1(c828689481d7e187c504dd072bd6714222888d33) )
	ROM_LOAD( "sth8c.bin", 0x0000, 0x010000, CRC(7ce6b760) SHA1(c828689481d7e187c504dd072bd6714222888d33) )
ROM_END_M1A_MCU

ROM_START( m1sudnim )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-177", 0x0000, 0x010000, CRC(25296856) SHA1(7e9a61c555709d443afb613b8cf646676e3a3e4a) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-178", 0x0000, 0x010000, CRC(83013a7a) SHA1(f54997e611cda9fd1ee4754e1c41b4eb38d1bd7f) )
	ROM_LOAD( "si2p", 0x0000, 0x010000, CRC(0ce156f1) SHA1(8f657a6226e81dd4ef26c4d58ac65c2f9d0951f0) )
	ROM_LOAD( "si5p", 0x0000, 0x010000, CRC(f1893852) SHA1(a942c3918179e03b01a04f3295dd6e0483061c8d) )
ROM_END_M1A_MCU

ROM_START( m1sprnov )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "supernovaprg.bin", 0x0000, 0x010000, CRC(6d533247) SHA1(475ecdcc10c9da558eb4d7c5e288d4724f65a5ce) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "supernovasnd.bin", 0x0000, 0x020000, CRC(f91e2c05) SHA1(e189c14214f4637d6a803893d79a41ad0fc8ebba) )
ROM_END_M1A_MCU

ROM_START( m1suppot )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-186", 0x0000, 0x010000, CRC(dfd15d88) SHA1(f493dab30f2c83f32de7cb209d6ba4e4412589ba) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-387", 0x0000, 0x010000, CRC(3fd18c6f) SHA1(9b2a1620bedb6e06009a36f7cfd491c024b36b7a) )
	ROM_LOAD( "sa6-388", 0x0000, 0x010000, CRC(99f9de43) SHA1(db0309cce34333d8cc8750c264d613a0ca316cc4) )
	ROM_LOAD( "sa7-206", 0x0000, 0x020000, CRC(f91b0dbd) SHA1(47865a675ddeaae3c8ec9ec5f0d81294320a0cf8) )
	ROM_LOAD( "sa7-207", 0x0000, 0x020000, CRC(a95af301) SHA1(e428fac9f10a10eba72839b83d4c4966cb4fb792) )
	ROM_LOAD( "sa7-444", 0x0000, 0x010000, CRC(6f3d11f5) SHA1(c0309436da11cd40ca012d18c2e519b9881ba698) )
	ROM_LOAD( "sa7-445", 0x0000, 0x010000, CRC(87d58c13) SHA1(ae80a217bc5cd566ceca675948f68976e22538aa) )
	ROM_LOAD( "sa7-484", 0x0000, 0x010000, CRC(11e7c53d) SHA1(1ab6734991b3b4dfe5191c343e68cd37b68aad93) )
	ROM_LOAD( "sa7-485", 0x0000, 0x010000, CRC(f90f58db) SHA1(669ad193b826fe299db79391a1f12cca8c2be633) )
	ROM_LOAD( "sa7-637", 0x0000, 0x010000, CRC(8f5992bd) SHA1(e70e090cf05d032660675652ca7d49e2d7ea64ce) )
	ROM_LOAD( "sa7-659", 0x0000, 0x010000, CRC(50565cc1) SHA1(c8a3c7daccb027583af6434c863a348aecf4a185) )
	ROM_LOAD( "sa8-077", 0x0000, 0x010000, CRC(127eb82c) SHA1(8d707851dada7ad346a43d9f02f002e535748913) )
	ROM_LOAD( "sa8-080", 0x0000, 0x010000, CRC(39cc4532) SHA1(ccb653e0d7003c56d892e6344ef0f0d861b3fb3b) )
	ROM_LOAD( "sa8-081", 0x0000, 0x010000, CRC(d124d8d4) SHA1(382f617a2bf258d5dec1fbefd3bd1590b1af3f4f) )
	ROM_LOAD( "sa8-276", 0x0000, 0x010000, CRC(16d22156) SHA1(b2e8fa3409bbd011052f4c28a54da02b00e0e319) )
	ROM_LOAD( "sa8-277", 0x0000, 0x010000, CRC(fe3abcb0) SHA1(852e1f96750ce6b8fd709c0625090bbb4965c1c5) )
	ROM_LOAD( "sa9-019", 0x0000, 0x020000, CRC(bdd89816) SHA1(cd1c730c99e526bc3a8ac177a9076aa5c072008d) )
	ROM_LOAD( "sa9-020", 0x0000, 0x020000, CRC(e6716af0) SHA1(881a608ba3c948d208db8ac0e55e6d972fc948a1) )
	ROM_LOAD( "sa9-021", 0x0000, 0x020000, CRC(9f56a6d8) SHA1(4831dec2bcaf1c1889a554dff1d36f77d3b52417) )
	ROM_LOAD( "sa9-022", 0x0000, 0x020000, CRC(8c919d8e) SHA1(7aa7e9ca69bbff93a4a6678e76239c58e392bde6) )
	ROM_LOAD( "spot5.8", 0x0000, 0x010000, CRC(35fdbd92) SHA1(3079a469be9f492af547dc239c6f5e98574b9156) )
	ROM_LOAD( "spot510", 0x0000, 0x010000, CRC(65273ae9) SHA1(9379784d65294c5df2f251db4bff84411ea6b1f9) )
	ROM_LOAD( "spot58", 0x0000, 0x010000, CRC(34be587e) SHA1(fa8578dec31bb25b89949f7425f5850210c89488) )
	ROM_LOAD( "spot5p8c.bin", 0x0000, 0x010000, CRC(ee41f99d) SHA1(52998c9fe6bc1839201c3269ce595df5d0b9468d) )
	ROM_LOAD( "spots10", 0x0000, 0x010000, CRC(50565cc1) SHA1(c8a3c7daccb027583af6434c863a348aecf4a185) )
	ROM_LOAD( "superpots", 0x0000, 0x010000, CRC(fa9625ca) SHA1(1f38f25db2937cb4270193f127772133367d644f) )
	ROM_LOAD( "superpots8pnd.bin", 0x0000, 0x010000, CRC(6b69835b) SHA1(7648c63d2a5ea05a4d3d7600e7310c116b1b58d1) )

	ROM_LOAD( "15spsfo", 0x0000, 0x020000, CRC(e09b2b09) SHA1(1d6183dcea7e99a8018d1ef1651488f65fe61858) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "superpots.p1", 0x000000, 0x080000, CRC(a17067d7) SHA1(5fc774251ca13e9d97559b68e560a456c0c364a8) )
	ROM_LOAD( "superpots.p2", 0x080000, 0x080000, CRC(4fa7759e) SHA1(7ba5099738ff3180eb9407b0772181c6cb6a81b8) )
ROM_END_M1A_MCU

ROM_START( m1taknot )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "takenote.bin", 0x008000, 0x008000, CRC(1ae63140) SHA1(809d04f43293cafc53d2a2ac697253e14b3b9608) ) // smaller than everything else?
ROM_END_M1A_MCU

ROM_START( m1tstunt )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ftu-but", 0x0000, 0x010000, CRC(ae9d315c) SHA1(bff9f49df6791f33d10b289526e0a00fe1dc9049) )
ROM_END_M1A_MCU


ROM_START( m1thatlf )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-327.bin", 0x0000, 0x010000, CRC(634fc46e) SHA1(84e166a182384b3b6f29653a0542af74c268d766) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-075.bin", 0x0000, 0x010000, CRC(b5922c08) SHA1(c71cd3629436576c381b4f1b45011c34a49c66b7) )
	ROM_LOAD( "sa4-076.bin", 0x0000, 0x010000, CRC(012af2db) SHA1(d6f1a24fe6674a423d190e9e04390cb01768bf4a) )
	ROM_LOAD( "thatslifegame", 0x0000, 0x010000, CRC(8dafbe30) SHA1(9a7e8a66b73ddf6564a34363342a8b7290e0dc4f) )
	ROM_LOAD( "thtslf5p", 0x0000, 0x010000, CRC(31507a65) SHA1(607b16cf3fde90f97e22247158f09d859a43c1ae) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "thatslifesound", 0x0000, 0x040000, CRC(5ac3a1f6) SHA1(5be73deb23d58fdc27dd41d210702b627e7ed324) )
ROM_END_M1A_MCU


ROM_START( m1topten )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-584", 0x0000, 0x010000, CRC(b2dea28a) SHA1(4d82e90a130ebcc2e6dca2b81a19d490f8d128bb) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-585", 0x0000, 0x010000, CRC(5a363f6c) SHA1(6760209e0a92e36b4f2ecd02129496313b4bcbc3) )
ROM_END_M1A_MCU


ROM_START( m1tp )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-297", 0x0000, 0x010000, CRC(b7a20f4d) SHA1(4b619f1ff26226304d86f69cfbf9d8e264af93b7) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-298", 0x0000, 0x010000, CRC(118a5d61) SHA1(41202c92710c5a9b2cbaea56e69a75366b6e3537) )
	ROM_LOAD( "sa7-503", 0x0000, 0x010000, CRC(56c8ee8e) SHA1(c1c0b9df26fecdca066cab2b4d7dadd9df8dbc2f) )
	ROM_LOAD( "sa7-505", 0x0000, 0x010000, CRC(3e7a3908) SHA1(a7a074db9e1c7101ec264d18450099e7b4718c26) )
	ROM_LOAD( "sa7-506", 0x0000, 0x010000, CRC(b9a7f407) SHA1(d57d75b1397efbc7dd76ef451247a1ae54287ea0) )
	ROM_LOAD( "sa7-507", 0x0000, 0x010000, CRC(514f69e1) SHA1(70c49c89a4a7c983aabae4f68e6d7a2f02b56f04) )
	ROM_LOAD( "sa7-747", 0x0000, 0x010000, CRC(d63ed4e3) SHA1(10fde5f826c3d3f9aa38f579590fe0f18001cb17) )
	ROM_LOAD( "sa7-748", 0x0000, 0x010000, CRC(701686cf) SHA1(68b2a48d895cda209091a1d6532342d183cc9f7d) )
	ROM_LOAD( "sa8-046", 0x0000, 0x010000, CRC(bd9d2a7f) SHA1(ec44f3867f36e9e2df786dd7a4e6a9babdbddabb) )
	ROM_LOAD( "sa8-047", 0x0000, 0x010000, CRC(5575b799) SHA1(1dd3240fb9d2a899a0a7baeb2c13fc03c1d2b77f) )
	ROM_LOAD( "sa8-100", 0x0000, 0x010000, CRC(caee1715) SHA1(a7d6758333778d77d48eaa1bf7a1cf36431ca60a) )
	ROM_LOAD( "sa8-101", 0x0000, 0x010000, CRC(22068af3) SHA1(35f918207e38bfe3aa6ddcc544d7977689be88a7) )
	ROM_LOAD( "sa8-212", 0x0000, 0x010000, CRC(130c355c) SHA1(f7a1bc3e37f331cb5fecd45c759a19284bd292f8) )
	ROM_LOAD( "sa8-213", 0x0000, 0x010000, CRC(fbe4a8ba) SHA1(ba762079d5839397a95d9be43ef2f982228a9867) )
	ROM_LOAD( "sa8-214", 0x0000, 0x010000, CRC(afa24e79) SHA1(e7a1e39c1060f590ee57dc7cdbb7a28a2399e573) )
	ROM_LOAD( "sa8-215", 0x0000, 0x010000, CRC(474ad39f) SHA1(6f7f38baebaa8c30d851eb57a63cc6902e72b36f) )
	ROM_LOAD( "triv15f", 0x0000, 0x010000, CRC(474ad39f) SHA1(6f7f38baebaa8c30d851eb57a63cc6902e72b36f) )
	ROM_LOAD( "triv15r", 0x0000, 0x010000, CRC(fbe4a8ba) SHA1(ba762079d5839397a95d9be43ef2f982228a9867) )
	ROM_LOAD( "triv5.10", 0x0000, 0x010000, CRC(9e838f82) SHA1(a51eb0c6d09fb68fea517caa73394f816c822693) )
	ROM_LOAD( "triv510", 0x0000, 0x010000, CRC(f98413b2) SHA1(472ec2c68239d408194ca5a8760cb1336a9a9170) )
	ROM_LOAD( "triv55", 0x0000, 0x010000, CRC(7e0affb2) SHA1(7611f288c766af8b6513b0788b87080b194e80a2) )
	ROM_LOAD( "triv58", 0x0000, 0x010000, CRC(a914852b) SHA1(1eaf9eab53369c4a764b77edb5133989157f9805) )
	ROM_LOAD( "triv5p8c.bin", 0x0000, 0x010000, CRC(ba452893) SHA1(2846af229ca43240ead9c2a8433ca2b4c5ffe1bc) )
	ROM_LOAD( "trivp10", 0x0000, 0x010000, CRC(b9bf4ed4) SHA1(80e423d9e893fe1cbdac23903f5d90c9ba7d0fca) )
	ROM_LOAD( "trivpgame.bin", 0x0000, 0x010000, CRC(be207368) SHA1(d7dddde544b28b0be7b5a02289e50ae50efb78c5) )
	ROM_LOAD( "trivpgame10", 0x0000, 0x010000, CRC(d692a4ee) SHA1(3d3081d02456c994be4fc3f7f4d2e8cc40985c8d) )
	ROM_LOAD( "trivtest", 0x0000, 0x010000, CRC(c73821df) SHA1(ec32dcfe87905f0f676b65561213afb591ac21a6) )

	ROM_LOAD( "15trivfo", 0x0000, 0x020000, CRC(3057c428) SHA1(5632bdf5c47abb4334cd3e4190a4c886a32d01bd) )


	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "trivpusuit.p1", 0x000000, 0x080000, CRC(3c4b4e2c) SHA1(db570035a3b9b7587501f342c80ec52cb1a79b49) )
	ROM_LOAD( "trivpusuit.p2", 0x080000, 0x080000, CRC(5a9808c1) SHA1(29c011ee2dc3e8bc87cb4c5ded61dfa3fdb9d7f7) )
ROM_END_M1A_MCU


ROM_START( m1tpclb )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "clubtp.bin", 0x0000, 0x020000, CRC(22556fe0) SHA1(6504ac7dbc6332972662c9abce0e4286f392c788) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc991080", 0x0000, 0x020000, CRC(22556fe0) SHA1(6504ac7dbc6332972662c9abce0e4286f392c788) )
	ROM_LOAD( "sc991081", 0x0000, 0x020000, CRC(2441b39d) SHA1(857eafabbae8d2cb7f01b9fe215f8c16aed32174) )
	ROM_LOAD( "trivclub", 0x0000, 0x020000, CRC(d9732839) SHA1(1c4393dbbfc399842ff4d35d3c95cad991eb8caf) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "clubtpsnd.p1", 0x000000, 0x080000, CRC(252fbb57) SHA1(aad7c833fbcbdcc2ff001df9f97e8ba3adf95cc1) )
	ROM_LOAD( "clubtpsnd.p2", 0x080000, 0x080000, CRC(6c391632) SHA1(0cf02463b52b6b25fbeae2e6bd278a1364ae594d) )
ROM_END_M1A_MCU


ROM_START( m1ultchl )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-135", 0x0000, 0x010000, CRC(ddca5e99) SHA1(e3092bd9f79fa70e851a3285061e2f77a7731e35) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa5-136", 0x0000, 0x010000, CRC(6972804a) SHA1(e313bbdb621f433c8c9314557bb2770afebe0800) )
	ROM_LOAD( "sa5-137", 0x0000, 0x010000, CRC(1dfc630f) SHA1(5f700e5960f323453cb2ee794da346f2bb591ff1) )
	ROM_LOAD( "sa5-138", 0x0000, 0x010000, CRC(bbd43123) SHA1(7a3bb67d5bee254cdb010273382e694838bad0bb) )
ROM_END_M1A_MCU


ROM_START( m1undsie )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-594", 0x0000, 0x020000, CRC(197cfd7d) SHA1(fa306cb96aa909d2c8b68284036b521ddc2a8921) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-595", 0x0000, 0x020000, CRC(493d03c1) SHA1(e161ca825d2f514c0e5185fd6d383c4fd6284a88) )
	ROM_LOAD( "sa7-596", 0x0000, 0x020000, CRC(9d842d4d) SHA1(eb4285902c887bcee0506a6f7806aa09fffd2a5e) )
	ROM_LOAD( "sa7-597", 0x0000, 0x020000, CRC(cdc5d3f1) SHA1(2a65b2567bc0fd4a443256817eea402a0887e668) )
ROM_END_M1A_MCU


ROM_START( m1vegas )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "veg_gam.cl", 0x0000, 0x020000, CRC(f1019a72) SHA1(2358d30bc6fa27b6daf2b6c63f031b46e28e3cf5) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sc991183", 0x0000, 0x020000, CRC(2ffe8e6e) SHA1(e3082f268293abe9a815240f2e8842acd3687653) )
	ROM_LOAD( "sc991184", 0x0000, 0x020000, CRC(c3eed4ea) SHA1(4ad365a523179fac1b4e325e299663686534470d) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "v_gam_sn.1", 0x000000, 0x080000, CRC(66520213) SHA1(fd855732e51225da4f459c4797e01f77f4836935) )
	ROM_LOAD( "v_gam_sn.2", 0x080000, 0x080000, CRC(3d19abe9) SHA1(cf8ab030fcca5a37c2c936566a2b7c77db1740f6) )
ROM_END_M1A_MCU


ROM_START( m1wagon )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-394", 0x0000, 0x020000, CRC(ac63dbd8) SHA1(b66592ef15c3e9eda208085040e41d14614da509) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-395", 0x0000, 0x020000, CRC(fc222564) SHA1(cf9c551b3c5844a33555c66f695f785fa163041d) )
	ROM_LOAD( "sa7-421", 0x0000, 0x020000, CRC(613fcdfb) SHA1(71cbc2ed3e677865b10672df496a22cd28ff5d5c) )
	ROM_LOAD( "sa7-422", 0x0000, 0x020000, CRC(72f8f6ad) SHA1(16d54afe4799d85cfcd686e355ea5a4cccebb54a) )
ROM_END_M1A_MCU

ROM_START( m1wotw )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-244", 0x0000, 0x020000, CRC(2bf62a60) SHA1(74f0cb4e33b7787b603e1e09066417f0d6700a68) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa8-245", 0x0000, 0x020000, CRC(7bb7d4dc) SHA1(516ebe329396d5c565a34994dece1ea232b1e177) )
	ROM_LOAD( "15wotwro", 0x0000, 0x020000, CRC(12fd4bbf) SHA1(0936c9299871d3d06050bb25f16a8bfd1757ce48) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "wotwsnd1", 0x000000, 0x080000, CRC(2a53544d) SHA1(46de438cfe5ca56886f10f293c59935036e08c72) )
	ROM_LOAD( "wotwsnd2", 0x080000, 0x080000, CRC(f5ffba54) SHA1(4b767236ee02a58f5430c77487b8f2e6ad2eccea) )
ROM_END_M1A_MCU

ROM_START( m1wldzne )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-522", 0x0000, 0x010000, CRC(814ab843) SHA1(e2b076fd69abbd0186d4748f59be22db1e017a23) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-527", 0x0000, 0x010000, CRC(eec99c71) SHA1(a142ea4929c3b9483aa87b076dfb192c520c16a8) )
	ROM_LOAD( "sa4-528", 0x0000, 0x010000, CRC(48e1ce5d) SHA1(90c5993fb5c519d7d5848b0a385c7bf3f41c8d6d) )
	ROM_LOAD( "sa4-546", 0x0000, 0x010000, CRC(b34c511d) SHA1(8c429f298ab383da6fbb082705f9580063475ff8) )
	ROM_LOAD( "sa4-623", 0x0000, 0x010000, CRC(ffe7a4d6) SHA1(fa40e830f714e5a5c54ee44cb7a52c99091d3296) )
	ROM_LOAD( "sa4-624", 0x0000, 0x010000, CRC(f3fffc6f) SHA1(b7697cb9751351bb368dd5a933b9d4a2687de054) )
	ROM_LOAD( "sa5-003", 0x0000, 0x010000, CRC(3e2ac4ff) SHA1(3b4cdc2170b4aeddb2efe1a334919d5d080e4119) )
	ROM_LOAD( "sa5-004", 0x0000, 0x010000, CRC(32329c46) SHA1(b58fabb94bc761a7a3538e55f3817a4123508159) )
	ROM_LOAD( "sa5-107", 0x0000, 0x010000, CRC(9324b657) SHA1(7b460db08b904be94e82f1054b48826ef56e13dd) )
	ROM_LOAD( "sa5-108", 0x0000, 0x010000, CRC(350ce47b) SHA1(6678552edd5a47c71abfd018cf8e594b869ae15f) )
	ROM_LOAD( "sa5-375", 0x0000, 0x010000, CRC(7c2da5db) SHA1(4a82fcbcf9586bca7a0a954e1b3ed79f5fe0a521) )
	ROM_LOAD( "sa5-376", 0x0000, 0x010000, CRC(c8957b08) SHA1(edf8f56c5c949af4ef7f2de79802c12fad9aef2e) )
	ROM_LOAD( "sa5-377", 0x0000, 0x010000, CRC(0143196d) SHA1(1f9d4bd1472e6b62c75c77d50cdbbdcdf7be4216) )
	ROM_LOAD( "sa5-378", 0x0000, 0x010000, CRC(a76b4b41) SHA1(df4597ab4bd5b7f36182db0f8143a063142e0d19) )
	ROM_LOAD( "sa5-379", 0x0000, 0x010000, CRC(03a508d8) SHA1(aa6e5c31ce0b331949865f0dc8cec10b777fb8b9) )
	ROM_LOAD( "sa5-380", 0x0000, 0x010000, CRC(c7c3ccb4) SHA1(64d4eb7961857a64cf7363004878a8048d02ea82) )
	ROM_LOAD( "sa6-417", 0x0000, 0x010000, CRC(68526fbc) SHA1(3f755dafe5c218602547b73c35965c313761e6f1) )
	ROM_LOAD( "sa6-418", 0x0000, 0x010000, CRC(ce7a3d90) SHA1(1dd7dd6993e33feba17c288a7f932f51a4b998eb) )
	ROM_LOAD( "wildzone(maygay)program2-1.bin", 0x0000, 0x010000, CRC(35f26690) SHA1(308eac37013cfd84970f04857fc8282efb58e210) )
	ROM_LOAD( "wzone10", 0x0000, 0x010000, CRC(001e604b) SHA1(f57015c10c9b7c955397a66251e489770a9620d0) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "wildzone(maygay)sound-p1.bin", 0x000000, 0x080000, CRC(9d512949) SHA1(65411c906f51f7d6d53ac897bedeab8fdd89d0a9) )
	ROM_LOAD( "wildzone(maygay)sound-p2.bin", 0x080000, 0x080000, CRC(78390d2c) SHA1(89c9d0f6deb74fee258bc614f61f0a82b3ef3959) )
ROM_END_M1A_MCU

ROM_START( m1winenc )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-371", 0x0000, 0x010000, CRC(ecba5b6b) SHA1(7b50d1a6d4ec287bb5159b6018282107d5594227) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa6-372", 0x0000, 0x010000, CRC(580285b8) SHA1(f0589184a60c73078c3cef9f89ca279fc67f9813) )
	ROM_LOAD( "sa7-573", 0x0000, 0x010000, CRC(a597cd98) SHA1(3d28306004b1937e7d04380fe3f9afc5ec321b7b) )
	ROM_LOAD( "sa7-574", 0x0000, 0x010000, CRC(a98f9521) SHA1(6fa2ba09dcadfb2164dbea376abc88d0187d02c8) )

	// these were from a set marked as 'John Francombe (Maygay)' which some have said is Epoch hardware,
	// however what we have here clearly isn't Epoch.  Apparently the John Francombe game actually had
	// 'Winners Enclosure' on the cabinet, along with a Dot Matrix Display.  The 'encdot' rom here
	// appears to be for a Dot Matrix Display, rather than being a main program rom, although I can't
	// recognize the CPU.  I'm tempted to say that these simply belong with this Winners Enclosure set
	// and it was never on Epoch hardware at all.
	ROM_REGION( 0x080000, "altroms", ROMREGION_ERASE00  )
	ROM_LOAD( "encdot.bin", 0x0000, 0x010000, CRC(3b707399) SHA1(9bc9522625e97c7d60cc104a96f7312b1d88ec01) )
	ROM_LOAD( "encsnd.u2", 0x0000, 0x080000, CRC(5568117a) SHA1(198aaead5a624902f31248e050231380b37167d4) )
	ROM_LOAD( "encsnd.u3", 0x0000, 0x080000, CRC(d78bee50) SHA1(dc4663efc795f7c518ebb9f17124f09d263d0585) )

ROM_END_M1A_MCU




ROM_START( m1bghou )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "bg39.bin", 0x0000, 0x020000, CRC(17889402) SHA1(3779e6f5ac7c2916e5e1af3a21af23a3b8923ef1) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "b_gool.4_3", 0x0000, 0x020000, CRC(eaa7d997) SHA1(061e4b8f4231018ef16200521ebb871b92df1f89) )
	ROM_LOAD( "b_goul.1_6", 0x0000, 0x020000, CRC(530c64ae) SHA1(caea76573e92f33c3c652e62f3a7b79db45feb07) )
	ROM_LOAD( "b_goul.3_0", 0x0000, 0x020000, CRC(d9cb0100) SHA1(bafea1d65066bdce5df46c6a12f4985a6ab0a187) )
	ROM_LOAD( "b_goul8.bin", 0x0000, 0x020000, CRC(ae58e0be) SHA1(9c6b54ab9a34a64492ce7c3e30aab27c7932ca11) )
	ROM_LOAD( "b_goul_f.3_4", 0x0000, 0x020000, CRC(58170ff4) SHA1(3623d01c56eb600f81041fd4d844fafd3389ed22) )
	ROM_LOAD( "b_gouli.2_1", 0x0000, 0x020000, CRC(234203f0) SHA1(5d477f70516dd3001587390ac8897328f4df339f) )
	ROM_LOAD( "bgool_ro.p1", 0x0000, 0x020000, CRC(959751c0) SHA1(7af9ec7f56675ed6ac013a5e0fa79df1ff50271f) )


	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "bgsnd1.bin", 0x000000, 0x080000, CRC(c0b13b6d) SHA1(d923cc71f9693a9321f984f51bd8f148cec1ac78) )
	ROM_LOAD( "bgsnd2.bin", 0x080000, 0x080000, CRC(1e19908b) SHA1(5d88e86798121d3355952daa3218925a00ef32fa) )
ROM_END_M1A_MCU


ROM_START( m1crzco )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "crazycobra.bin", 0x0000, 0x020000, CRC(31615cf8) SHA1(03ecef486350aa8ba0fbd0fbe0eb2c64b86c6848) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "cobra15.fo", 0x0000, 0x020000, CRC(2e2c08d6) SHA1(80faf478e5587742f20bd25a5f1828ec640ac4b6) )
	ROM_LOAD( "cobra15.foh", 0x0000, 0x020000, CRC(6852a8c7) SHA1(426d908f6aaaae6233af17d1c3bae95d70c73351) )
	ROM_LOAD( "cobra8.fo", 0x0000, 0x020000, CRC(61698fa6) SHA1(bf369173dfbbf365d6c584636b57ee5aa8f599ae) )
	ROM_LOAD( "cobra8.foh", 0x0000, 0x020000, CRC(2fa223d4) SHA1(1db0be23634d754513dc152eb708d50323f87af5) )
	ROM_LOAD( "cr_cobra.1_1", 0x0000, 0x020000, CRC(b0a0f91c) SHA1(965f7bb1fafbf326def22cb82ef1b0315795d973) )


	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "crazycobrasnd.p1", 0x000000, 0x080000, CRC(c5ce740c) SHA1(c8e59ec36aaa0b35fe31e4f178e5c23093488151) )
	ROM_LOAD( "crazycobrasnd.p2", 0x080000, 0x080000, CRC(c49e6889) SHA1(e678a0c0f7ca067281248b48935f80756d161b39) )
ROM_END_M1A_MCU


ROM_START( m1simps )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-412", 0x0000, 0x010000, CRC(6f025e05) SHA1(182111f3b030589fe1829c3c24fcee9937d206dd) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa4-413", 0x0000, 0x010000, CRC(87eac3e3) SHA1(d0b4cde28921870945e7d14642cd6f4abc699345) )
	ROM_LOAD( "sa4-431", 0x0000, 0x010000, CRC(6ff34645) SHA1(13b224453164c6aaaf19fa19cdef296af17ec076) )
	ROM_LOAD( "sa4-432", 0x0000, 0x010000, CRC(db4b9896) SHA1(2b7f361f928176075e02db5dd0d4edfc6ecf5757) )
	ROM_LOAD( "sa4-434", 0x0000, 0x010000, CRC(51d25358) SHA1(dbbb63a76b7c02a0c78ac7c06559a0b6e913496b) )
	ROM_LOAD( "sa4-448", 0x0000, 0x010000, CRC(e9afaa51) SHA1(fbce125e9874167b42a56f0ad38ecb21897f76b0) )
	ROM_LOAD( "sa4-460", 0x0000, 0x010000, CRC(17c1ad7a) SHA1(97ed56dbd2a926b92fbde12587984737c558c0f6) )
	ROM_LOAD( "si_x6__d.2_1", 0x0000, 0x010000, CRC(5dca0be1) SHA1(ade490360e70fb0c5184a72520735d31579893bd) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "si______.1_2", 0x000000, 0x080000, CRC(a1ac090e) SHA1(71133ac994dafa0993a600cbebb0cdfde5c09279) )
	ROM_LOAD( "si______.1_3", 0x080000, 0x080000, CRC(72cf719d) SHA1(707942c1b1beba3b3758cbf999b59e7a03bd137f) )
ROM_END_M1A_MCU





ROM_START( m1hiloc )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "hilo1-5n.p1", 0x0000, 0x020000, CRC(7a5010e5) SHA1(3a0bb854771b0b1e136932c6f78fac98114fda41) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "hilo1-5p.p1", 0x0000, 0x020000, CRC(826da69a) SHA1(8a0460d7ca20c34b3c63ceadfd79d7bbde40c566) )
ROM_END

ROM_START( m1jtjob )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "just2-0n.p1", 0x0000, 0x020000, CRC(d63d1710) SHA1(feccdd6dc242d32f04a080ab6a637cd6ec330c0d) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "j_t_job.1_9", 0x0000, 0x020000, CRC(4f6da2b2) SHA1(43b4c314d5ee934d8dc8972c3c0585096d78793f) )
	ROM_LOAD( "just2-0p.p1", 0x0000, 0x020000, CRC(8d809ba4) SHA1(a6fd142edb0a98c847cb376efe40756d19bccb02) )
	ROM_LOAD( "just2-1n.p1", 0x0000, 0x020000, CRC(7af617ab) SHA1(3288eb180bb0b6c742a70d6044ea7a8c92fd2835) )
	ROM_LOAD( "just2-1p.p1", 0x0000, 0x020000, CRC(e4543b42) SHA1(d3d378ae70088e55d8a0409a81cc7cd8c01c7856) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "jobsnd1.u2", 0x000000, 0x080000, CRC(69b3c5d6) SHA1(4fcf16686e0fc322a8c03e697d0292270ceccc5a) )
	ROM_LOAD( "jobsnd2.u3", 0x080000, 0x080000, CRC(2e7d049e) SHA1(ee58c795e7da3735827dd6af0b44c03166cd99dc) )
ROM_END

ROM_START( m1kingsw )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "kots.p1", 0x0000, 0x020000, CRC(7eff2b6b) SHA1(b9cade903c9d5723f4fc932033b5fbf77f6803d1) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "kos51-0n.p1", 0x0000, 0x020000, CRC(7132e13c) SHA1(113fb748a293ba30acb8845ba4a50a8016c0c0b6) )
	ROM_LOAD( "kots1-6p.p1", 0x0000, 0x020000, CRC(c1499640) SHA1(bed3d3bfa4d24ebd388d6f428b0d37dd9aeb5c18) )
	ROM_LOAD( "ko_swing.p1", 0x0000, 0x020000, CRC(f4704f01) SHA1(9e0ff678649472bb516f46e6060c45c83f85fc2c) )
ROM_END

ROM_START( m1lca )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "lca11-4n.p1", 0x0000, 0x020000, CRC(65d5efab) SHA1(5b2b6849bfc47360d47e93ca2605d4e18d18a760) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "lca11-4p.p1", 0x0000, 0x020000, CRC(c60bcf0f) SHA1(ea9a79b0a76e8102c1c483987d22411a78cc5525) )
	ROM_LOAD( "lght1-4n.p1", 0x0000, 0x020000, CRC(31bfdc29) SHA1(3e3efed25e07772ca4903aeca1cee94cf527c382) )
	ROM_LOAD( "lght1-4p.p1", 0x0000, 0x020000, CRC(f5d8ab2a) SHA1(6d948764bca2f00c7f41f4dbf9faf74da1e45b0b) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "lcasnd1.bin", 0x000000, 0x080000, CRC(56cc170c) SHA1(c491897b748921201489b38703a3b208e7fdd2f3) )
	ROM_LOAD( "lcasnd2.bin", 0x080000, 0x080000, CRC(73062bef) SHA1(a02f91306c46205ca518d90fdde27508e9d14f63) )
ROM_END

ROM_START( m1mb )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "monk0-9n.p1", 0x0000, 0x020000, CRC(6d51040c) SHA1(a97ecd2324622abe5298919d4bda298f6f736572) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "monkey.07", 0x0000, 0x020000, CRC(fb92ac29) SHA1(f066a4b5968fdac040dc65b6a7727a91ae41233b) )
	ROM_LOAD( "monkey business v0-4.bin", 0x0000, 0x020000, CRC(ead89920) SHA1(de7da735429956e8fdb0593937d3cf3dd2e4e7d8) )
	ROM_LOAD( "monkeybusiness.bin", 0x0000, 0x020000, CRC(6d06e255) SHA1(132e2d21768ac317edff7fa349ac9ce8112c317a) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "mb_snd_1.bin", 0x000000, 0x080000, CRC(05594e7c) SHA1(7caf32e4827b574a68cac6ad5cfae73ef228ae09) )
	ROM_LOAD( "mb_snd_2.bin", 0x080000, 0x080000, CRC(bd477c2c) SHA1(cbca3d637ce221fe2763e598afc93aabcf464c35) )
ROM_END

ROM_START( m1mbclb )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "mbcb1-1n.p1", 0x0000, 0x020000, CRC(32621c7f) SHA1(0cf1bf264712f8c042315d935c33f4ade3446542) )
ROM_END

ROM_START( m1sycc )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "stak1-6n.p1", 0x0000, 0x020000, CRC(cdeb76c2) SHA1(5b29c5e8e3ae9640e25caadab10ac9bad3be71dc) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "stak1-6p.p1", 0x0000, 0x020000, CRC(0aaedd71) SHA1(3c7610b327b506ba12bbb0a8804b7d374be5fab7) )
	ROM_LOAD( "st_y_cl.1_1", 0x0000, 0x020000, CRC(213d38a4) SHA1(a5af799a48c3a7eee61d84c2c25c380eb30628dc) )
ROM_END

ROM_START( m1thrill )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "thrillsandspills.bin", 0x0000, 0x020000, CRC(415d9c87) SHA1(3af0580c65a2242516c83d4208fc5b4ae0d8cf21) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "spil0-8n.p1", 0x0000, 0x020000, CRC(76297570) SHA1(b0aaa98cbc2e7331d96d27408214936133bf5726) )
	ROM_LOAD( "spil0-4n.p1", 0x0000, 0x020000, CRC(9d3a4936) SHA1(88073f46ebd9622643078561be00fcd98093eee2) )
	ROM_LOAD( "thrill0.7", 0x0000, 0x020000, CRC(f52a0367) SHA1(9cf3beb088a7b52a19bed5b85bd1394cd24ced10) )

	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "thrillsandspillssnd.p1", 0x000000, 0x080000, CRC(591653ba) SHA1(60eea91d57b82eec427a4e10746f272d42d99891) )
	ROM_LOAD( "thrillsandspillssnd.p2", 0x080000, 0x080000, CRC(0b715a55) SHA1(988034855ed337bdb3b360aae282f22b67de0c64) )
ROM_END

ROM_START( m1trtr )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "tric0-7p.p1", 0x0000, 0x020000, CRC(86a58a4d) SHA1(9930f0b1848359fffc31f2280a30bb7643263241) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "tric0-9n.p1", 0x0000, 0x020000, CRC(f002b852) SHA1(ad056a7ada28e40a17e977d871197afb1ecac678) )
ROM_END

ROM_START( m1trtrcl )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "trick or treat club v0-2.bin", 0x0000, 0x020000, CRC(af72431c) SHA1(a6060445527372606bcfab9eb9f0a40882d00520) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "ttcb0-2n.p0", 0x0000, 0x020000, CRC(af72431c) SHA1(a6060445527372606bcfab9eb9f0a40882d00520) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "trick or treat sound 0.bin", 0x000000, 0x080000, CRC(4900e4d3) SHA1(ce3342d076caadb793572411be8394e02a37cd11) )
	ROM_LOAD( "trick or treat sound 1.bin", 0x080000, 0x080000, CRC(42ae9bc3) SHA1(c82f3d8ac6004b827913e1940b0d06ed7c1584d0) )
ROM_END

ROM_START( m1vegcrw )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "vege0-2n.p1", 0x0000, 0x020000, CRC(71cc53b2) SHA1(1d0775aed64f1b622a958f99dd23d8fb86dad0de) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "vegesnd1.u2", 0x000000, 0x080000, CRC(9ce69785) SHA1(126e76ed1a8b7dc71df5c21c77d7daecbdeb2796) )
	ROM_LOAD( "vegesnd2.u3", 0x080000, 0x080000, CRC(341d43a6) SHA1(d10875e6b2c98afcc2244588d00602f0c26fb295) )
ROM_END


ROM_START( m1calyps )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "calypso.5", 0x0000, 0x020000, CRC(3e66644d) SHA1(1292ff92ae8c3aeb1f37cdfab27725feb8092969) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "calypso.bin", 0x0000, 0x020000, CRC(f97cb079) SHA1(1bb0741bd34e6937c5c4711331f43bc0aaaab852) )
	ROM_LOAD( "sa001008", 0x0000, 0x020000, CRC(f97cb079) SHA1(1bb0741bd34e6937c5c4711331f43bc0aaaab852) )
	ROM_LOAD( "sa001009", 0x0000, 0x020000, CRC(ff686c04) SHA1(dd935a6acc88e8d9738176c22200c36505b48f18) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "calypsosnd.p1", 0x0000, 0x080000, CRC(b29d170b) SHA1(1922a135ce2841716cd5fd16bbdb89a9fbb23b22) )
	ROM_LOAD( "calypsosnd.p2", 0x0000, 0x080000, CRC(69cea85d) SHA1(8b114bd0a4ed9bee0c62652fe77ee41164b6e420) )
ROM_END



ROM_START( m1cashln )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "cl10p1", 0x0000, 0x020000, CRC(b993ff3d) SHA1(67359e2076bb84001744d13a78c960fc587ecb39) )
ROM_END



ROM_START( m1races )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-085", 0x0000, 0x020000, CRC(6b70dd13) SHA1(6059fc560061dc1a5342e94f1932552a4e4ddfa1) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa7-086", 0x0000, 0x020000, CRC(78b7e645) SHA1(715ae98c8cde095735094cd2c908e464d90f4a46) )
	ROM_LOAD( "sa7-532", 0x0000, 0x020000, CRC(7361bdb5) SHA1(38826149029d64173694c21da4322e3814299f29) )
	ROM_LOAD( "sa7-533", 0x0000, 0x020000, CRC(23204309) SHA1(04e7ee3b59e73b570eac7bd82ceff0af55cb26c4) )
ROM_END



ROM_START( m1digdel )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "digg3-2p.p1", 0x0000, 0x020000, CRC(63d7037d) SHA1(3e29c3df2f4ff77bb4ac7f52a71c4fa2d4a8c66a) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "diggersdelight.p1", 0x0000, 0x020000, CRC(9beab502) SHA1(59773f49206497070ebffff99c3375177f352aff) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "diggdelightsound3.bin", 0x0000, 0x080000, CRC(ff56068a) SHA1(cfd4cea5fc4f9278a01d0953ece92d3e6c59a8a4) )
	ROM_LOAD( "diggdelightsnd4.bin", 0x0000, 0x080000, CRC(1d7ea3c5) SHA1(902bc00be62b3106337cb7fe4d3e9a4d5a5533f4) )
ROM_END




ROM_START( m1startr )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "mdmstartrekkingprogram3_1.rom", 0x0000, 0x010000, CRC(a911ebc7) SHA1(7e74df1ca0fd5e0d04ece5ca307f4b1ab817c044) )

	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "st58dt", 0x0000, 0x010000, CRC(29b73d2d) SHA1(f4bfcce7b8f158e8ec964936c365a2c6f27f7945) )
	ROM_LOAD( "st58st", 0x0000, 0x010000, CRC(b4b4c3f3) SHA1(7179d970c0e903ae3b4ba925fba29b9777bf969d) )
	ROM_LOAD( "startrekking-20p_8_5.bin", 0x0000, 0x010000, CRC(6455ae3b) SHA1(9521b69cdd4a0d6ed306ad713fd9a3924eae1e8d) )
	ROM_LOAD( "tk05d3_1.bin", 0x0000, 0x010000, CRC(b53552ca) SHA1(7b3fd9c5e858a0e1462c755ebfbc089879001c7c) )
	ROM_LOAD( "tk05d5_0.bin", 0x0000, 0x010000, CRC(629a4c8c) SHA1(766933e4c6d352273c72e40ac0f2af73896d513d) )
	ROM_LOAD( "tk05s3_1.bin", 0x0000, 0x010000, CRC(871db7ce) SHA1(4aac90a6ecab6d2c6b5d8b7af69059ef260c9c0c) )
	ROM_LOAD( "tk05s5_0.bin", 0x0000, 0x010000, CRC(6a600631) SHA1(1c85cd63db5225ca55ef0f26fc65008cb4af340b) )
	ROM_LOAD( "tk10d3_2.bin", 0x0000, 0x010000, CRC(5cebc22d) SHA1(6e6dd4c98ceb0c3cf541c6f8d00e3928f43dc763) )
	ROM_LOAD( "tk10d5_0.bin", 0x0000, 0x010000, CRC(349855d0) SHA1(e83d764169e85b1f24b3cb7a0d9b1ce3228148c3) )
	ROM_LOAD( "tk10s3_2.bin", 0x0000, 0x010000, CRC(7955d544) SHA1(1151e2b5dd3bd60846d28fa0fb49fe6bee06b765) )
	ROM_LOAD( "tk10s5_0.bin", 0x0000, 0x010000, CRC(6e871dcb) SHA1(cb74b94537f7cdbba6e254042f2b59409eb3b00c) )
	ROM_LOAD( "tk20d3_1.bin", 0x0000, 0x010000, CRC(a911ebc7) SHA1(7e74df1ca0fd5e0d04ece5ca307f4b1ab817c044) )
	ROM_LOAD( "tk20d5_0.bin", 0x0000, 0x010000, CRC(ca3d3faa) SHA1(f19465212d3bc094a61d04c6c1c20e524a36dcf8) )
	ROM_LOAD( "tk20s3_1.bin", 0x0000, 0x010000, CRC(0193af35) SHA1(40094dd44da8d1d0d38ac95d4e951e6c88516eee) )
	ROM_LOAD( "tk20s5_0.bin", 0x0000, 0x010000, CRC(b0532d71) SHA1(d3e111a4bc5638788bb67faebdd046224895cbb1) )
	ROM_LOAD( "tk3.1c", 0x0000, 0x010000, CRC(affb3ea4) SHA1(64a670b074cd2f151e820428e0f7f485ec710efd) )
	ROM_LOAD( "tk5-5_0x.bin", 0x0000, 0x010000, CRC(1d32480d) SHA1(670c9d371755c500c29a31a350b3447bd2788c62) )
	ROM_LOAD( "tk5_0x.bin", 0x0000, 0x010000, CRC(20a796f2) SHA1(1d46dbf3693294733595cd601e5be6f16ff685d3) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "mdmstartrekkingsound.rom", 0x0000, 0x040000, CRC(4b673184) SHA1(dd90719ebc8644b4aa50091dc9ddd79f5d0f3395) )
ROM_END


ROM_START( m1ttcash )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "ttc2-01.bin", 0x0000, 0x010000, CRC(fa3a1d75) SHA1(055aac3bd82892e30efb6f0a359f53045f8d226e) )
	ROM_REGION( 0x100000, "oki", ROMREGION_ERASE00  )
	ROM_LOAD( "ttcsnd.bin", 0x0000, 0x080000, CRC(a191218e) SHA1(d89c33538d1f1804b2f5acac713e760d089fbac0) )
ROM_END

ROM_START( m1topstr )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa1-184", 0x0000, 0x010000, CRC(93518981) SHA1(51e5f4e665c7b3cf7d62036e4267216c36726d3f) )
ROM_END


ROM_START( m1sirich )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-265.bin", 0x0000, 0x010000, CRC(1b674002) SHA1(9f8d839371290ceb8bdc936d82f1e6180783d169) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "sa3-269.bin", 0x0000, 0x010000, CRC(d490b5fa) SHA1(fc86cad55387f3cc4e0803f1669d670998247dc5) )
	ROM_LOAD( "sa4-104.bin", 0x0000, 0x010000, CRC(298e0d6e) SHA1(969825784c9e793bf88cf355a70bbc1907126a2f) )
	ROM_LOAD( "sa4-105.bin", 0x0000, 0x010000, CRC(c1669088) SHA1(96ede015a26d75f2b85d615df0ffbce4a5f9f8d4) )
ROM_END

ROM_START( m1gskill )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "greekskill.bin", 0x0000, 0x010000, CRC(ac5b7f65) SHA1(9fd73c53173b3291684de3d1067a115e5f78a336) )
ROM_END

ROM_START( m1luxor )
	ROM_REGION( 0x20000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "luxor_pound5f.3v3", 0x0000, 0x020000, CRC(75a95629) SHA1(1366f1bf1f88feee14af306d15fe74c2c9d1dff8) )
	ROM_REGION( 0x20000, "altrevs", ROMREGION_ERASE00  )
	ROM_LOAD( "luxor_pound8f.4_0", 0x0000, 0x020000, CRC(3a0fdf4a) SHA1(4a3797ea5440df8a0d40ea187d0b41a77407e9ef) )
	ROM_LOAD( "lux_pound15", 0x0000, 0x020000, CRC(94e8d1c2) SHA1(7a50477cf2d4bb404d2e33dd545c51e62bf4031d) )
	ROM_LOAD( "lux_pound5", 0x0000, 0x020000, CRC(a5d78869) SHA1(0b76345374554d467c78751083a6b3ce2499f795) )
ROM_END


// I'm not 100% convinced this is a real game, or a collection of random roms
ROM_START( m1atunk )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "rndm7802.u1", 0x0000, 0x010000, CRC(d88ec13b) SHA1(3f32a2c77c1bdab016f582fa40099a6ffb6427f5) )
	ROM_LOAD( "rndm7802.u2", 0x0000, 0x010000, CRC(0439b0f9) SHA1(fb3941d80ad64b3f442d11697c30f9c63131b5c4) )
	ROM_LOAD( "rrun4.1t", 0x0000, 0x010000, CRC(4c95f6c1) SHA1(112c4b39fa9654824e47c1da360c1ce3e1ee750a) )
	ROM_LOAD( "rrun4.2t", 0x0000, 0x010000, CRC(c423978e) SHA1(6a8b88c931ed95946226244d8bd88323412d0e1c) )

	ROM_REGION( 0x200000, "other1", 0 )
	// these look like something else? check!
	ROM_LOAD( "dnj1.6", 0x0000, 0x010000, CRC(29d1ce09) SHA1(ed90ce869c71d06a2f6d1cc7becbeff53d00b3c2) ) // has a cotswald copyright??
	ROM_LOAD( "jjok", 0x0000, 0x010000, CRC(9821978b) SHA1(e327c59c2f47774986a5e22cbd119a7f75aeddb1) ) // Jolly Joker? (German?)
	ROM_LOAD( "stest01.bin", 0x0000, 0x010000, CRC(0e2a4906) SHA1(6d3c5ed2fd8f78b63289dc96d7153740937a9584) ) // Test ROM?

	ROM_REGION( 0x200000, "other2", 0 )
	// these look like 68k pairs instead? check!
	ROM_LOAD( "grm17011.bin", 0x0000, 0x010000, CRC(b1aa6eca) SHA1(31c5995eb19be4772413e3dc3f690f609c123673) )
	ROM_LOAD( "grm17012.bin", 0x0000, 0x010000, CRC(a26a6345) SHA1(70ededa58b92d32e0968bb326d6579146c3bd6cb) )
	ROM_LOAD( "grmc2001.u1", 0x0000, 0x010000, CRC(6301ebdb) SHA1(b6290fccbfa31ddf7f972dfb493a94b8b844fc17) )
	ROM_LOAD( "grmc2001.u2", 0x0000, 0x010000, CRC(6332520f) SHA1(f6b2cc29ee947f5421593922cb4b7619d832e156) )
	ROM_LOAD( "grm_p1.bin", 0x0000, 0x010000, CRC(43375584) SHA1(4d9342ea5ef3fe882f51f37771bbf033363fca6c) )
	ROM_LOAD( "grm_p2.bin", 0x0000, 0x010000, CRC(e8809b05) SHA1(9002642cbecaed61746cd25de02fe6efbc5c2ca2) )
	ROM_LOAD( "grm_pl_1.bin", 0x0000, 0x010000, CRC(b1aa6eca) SHA1(31c5995eb19be4772413e3dc3f690f609c123673) )
	ROM_LOAD( "grm_pl_2.bin", 0x0000, 0x010000, CRC(a26a6345) SHA1(70ededa58b92d32e0968bb326d6579146c3bd6cb) )

	ROM_REGION( 0x100000, "msm6376", ROMREGION_ERASE00 )
ROM_END

static WRITE8_HANDLER( m1ab_no_oki_w )
{
	popmessage("write to OKI, but no OKI rom");
}

static DRIVER_INIT( m1 )
{

	//AM_RANGE(0x2420, 0x2421) AM_WRITE( latch_ch2_w ) // oki
	// if there is no OKI region disable writes here, the rom might be missing, so alert user

	UINT8 *okirom = machine.region( "oki" )->base();

	if (!okirom)
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x2420, 0x2421, FUNC(m1ab_no_oki_w));

	// print out the rom id / header info to give us some hints
	// note this isn't always correct, alley cat has 'Calpsyo' still in the ident string?
	{
		UINT8 *cpu = machine.region( "maincpu" )->base();
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


GAME( 1993, m1albsq		,0			,m1,m1,m1, ROT0, "Maygay", "Albert Square (Maygay) v4.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1albsqp	,m1albsq	,m1,m1,m1, ROT0, "Maygay", "Albert Square (Maygay) v4.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1albsq3	,m1albsq	,m1,m1,m1, ROT0, "Maygay", "Albert Square (Maygay) v3.0 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1albsq2	,m1albsq	,m1,m1,m1, ROT0, "Maygay", "Albert Square (Maygay) v2.2 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1albsq1	,m1albsq	,m1,m1,m1, ROT0, "Maygay", "Albert Square (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1albsq1p	,m1albsq	,m1,m1,m1, ROT0, "Maygay", "Albert Square (Maygay) v1.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

GAME( 199?, m1alley		,0			,m1,m1,m1, ROT0, "Maygay", "Alley Cat (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

GAME( 1997, m1apollo	,0			,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) vA.1 (Newer) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollop	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) vA.1 (Newer) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apolloh	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) vA.1 (Newer) (Hack?) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apolloa	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) vA.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apolloap	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) vA.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apolloao	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) vA.1 (Older, 15GBP) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo9	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v9.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo9p	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v9.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo4	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v4.1 (Newer) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo4p	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v4.1 (Newer) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo11	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v11? (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo11b	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v11? (BwB Rebuild) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo11p	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v11? (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo8	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v8.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo8p	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v8.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo7	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v7.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo7p	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v7.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo5	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v5.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo5p	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v5.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo4o	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v4.1 (Older, Token)(M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo3	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v3.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo3p	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v3.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo2	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v2.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1apollo2p	,m1apollo	,m1,m1,m1, ROT0, "Maygay", "Apollo 9 (Maygay) v2.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

GAME( 199?, m1bankbs	,0			,m1,m1,m1, ROT0, "Maygay", "Bank Buster Club (Maygay) v2.9 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1bankbsp	,m1bankbs	,m1,m1,m1, ROT0, "Maygay", "Bank Buster Club (Maygay) v2.9 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1bankbso	,m1bankbs	,m1,m1,m1, ROT0, "Maygay", "Bank Buster Club (Maygay) v2.8 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1bankrl	,0			,m1,m1,m1, ROT0, "Maygay", "Bank Roll (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1bankrlp	,m1bankrl	,m1,m1,m1, ROT0, "Maygay", "Bank Roll (Maygay) v1.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1bankrl2p	,m1bankrl	,m1,m1,m1, ROT0, "Maygay", "Bank Roll (Maygay) v2.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1bargn		,0			,m1,m1,m1, ROT0, "Maygay", "Bar-gain (Maygay) v7.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1bargnp	,m1bargn	,m1,m1,m1, ROT0, "Maygay", "Bar-gain (Maygay) v7.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1bargnc	,m1bargn	,m1,m1,m1, ROT0, "Maygay", "Casino Bar-gain (Maygay) v5.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1bargncp	,m1bargn	,m1,m1,m1, ROT0, "Maygay", "Casino Bar-gain (Maygay) v5.1 (Protocol)(M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 2000, m1bigdel	,0			,m1,m1,m1, ROT0, "Maygay", "Big Deal (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1blkhol	,0			,m1,m1,m1, ROT0, "Maygay", "Black Hole (Dutch) (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1992, m1bluemx	,0			,m1,m1,m1, ROT0, "Maygay", "Blue Max (Maygay) v2.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1992, m1bluemxp	,m1bluemx	,m1,m1,m1, ROT0, "Maygay", "Blue Max (Maygay) v2.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi		,0			,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v1.1 (Newer) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondip	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v1.1 (Newer) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi4	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v4.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi4p	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v4.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi3	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v3.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi2	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v2.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi2p	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v2.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi2po	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v2.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi1	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bondi1p	,m1bondi	,m1,m1,m1, ROT0, "Maygay", "Bondi Beach (Maygay) v1.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1bounty	,0			,m1,m1,m1, ROT0, "Maygay", "Bounty Hunter (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1bountc	,0			,m1,m1,m1, ROT0, "Maygay", "Bounty Hunter Club (Maygay) v1.3 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1bountcp	,m1bountc	,m1,m1,m1, ROT0, "Maygay", "Bounty Hunter Club (Maygay) v1.3 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik		,0			,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v11? (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cikp		,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v11? (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cikh		,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v11? (Hack?) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik51		,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v5.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik51p	,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v5.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik51o	,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v5.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik41		,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v4.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik41p	,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v4.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik31		,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v3.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik31p	,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v3.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik21		,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v2.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik21p	,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v2.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik12		,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v1.2 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik11		,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik11p	,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v1.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik11n	,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v1.1 (alternate) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1997, m1cik11np	,m1cik		,m1,m1,m1, ROT0, "Maygay", "Cash Is King (Maygay) v1.1 (alternate,Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1casclb	,0			,m1,m1,m1, ROT0, "Maygay", "Casino Club (Maygay) v1.2 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1casclbp	,m1casclb	,m1,m1,m1, ROT0, "Maygay", "Casino Club (Maygay) v1.2 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1casclb1	,m1casclb	,m1,m1,m1, ROT0, "Maygay", "Casino Club (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1casgcl	,0			,m1,m1,m1, ROT0, "Maygay", "Casino Gambler Club (Maygay) v1.2 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1casgclp	,m1casgcl	,m1,m1,m1, ROT0, "Maygay", "Casino Gambler Club (Maygay) v1.2 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1casroy	,0			,m1,m1,m1, ROT0, "Maygay", "Casino Royale Club (Maygay) v1.2 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1casroyp	,m1casroy	,m1,m1,m1, ROT0, "Maygay", "Casino Royale Club (Maygay) v1.2 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1990, m1casroy1	,m1casroy	,m1,m1,m1, ROT0, "Maygay", "Casino Royale Club (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1chain		,0			,m1,m1,m1, ROT0, "Maygay", "Chain Reaction (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1clbfvr	,0			,m1,m1,m1, ROT0, "Maygay", "Club Fever (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1clbfvrp	,m1clbfvr	,m1,m1,m1, ROT0, "Maygay", "Club Fever (Maygay) v1.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedo	,0			,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v6.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedop	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v6.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedo5	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v5.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedo5p	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v5.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedoi	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v7.2 (Isle of Man) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedoip	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v7.2 (Isle of Man) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedon	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v1.2 (Newer) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedonp	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v1.2 (Newer) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedob2	,m1cluedo	,m1,m1,m1, ROT0, "Maygay/BwB", "Cluedo (Maygay/BwB) v2.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedob2p	,m1cluedo	,m1,m1,m1, ROT0, "Maygay/BwB", "Cluedo (Maygay/BwB) v2.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedob2h	,m1cluedo	,m1,m1,m1, ROT0, "Maygay/BwB", "Cluedo (Maygay/BwB) v2.1 (Hack?) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedob1	,m1cluedo	,m1,m1,m1, ROT0, "Maygay/BwB", "Cluedo (Maygay/BwB) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedob1p	,m1cluedo	,m1,m1,m1, ROT0, "Maygay/BwB", "Cluedo (Maygay/BwB) v1.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1995, m1cluedob1h	,m1cluedo	,m1,m1,m1, ROT0, "Maygay/BwB", "Cluedo (Maygay/BwB) v1.1 (Hack?) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluedo4	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v4.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluedo4p	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v4.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluedo3	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v3.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluedo3p	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v3.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluedo3h	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v3.1 (Hack?) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluedo1	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluedo1p	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v1.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluedo1h	,m1cluedo	,m1,m1,m1, ROT0, "Maygay", "Cluedo (Maygay) v1.1 (Hack?) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

GAME( 1993, m1cluecb	,0			,m1,m1,m1, ROT0, "Maygay", "Cluedo Club (Maygay) v3.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluecbp	,m1cluecb	,m1,m1,m1, ROT0, "Maygay", "Cluedo Club (Maygay) v3.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluecb2	,m1cluecb	,m1,m1,m1, ROT0, "Maygay", "Cluedo Club (Maygay) v2.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluecb2p	,m1cluecb	,m1,m1,m1, ROT0, "Maygay", "Cluedo Club (Maygay) v2.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluecb1	,m1cluecb	,m1,m1,m1, ROT0, "Maygay", "Cluedo Club (Maygay) v1.1 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluecb1p	,m1cluecb	,m1,m1,m1, ROT0, "Maygay", "Cluedo Club (Maygay) v1.1 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluesh	,0			,m1,m1,m1, ROT0, "Maygay", "Super Cluedo Showcase (Maygay) v1.2 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1clueshp	,m1cluesh	,m1,m1,m1, ROT0, "Maygay", "Super Cluedo Showcase (Maygay) v1.2 (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1cluesho	,m1cluesh	,m1,m1,m1, ROT0, "Maygay", "Super Cluedo Showcase (Maygay) v1.2 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1993, m1clueshop	,m1cluesh	,m1,m1,m1, ROT0, "Maygay", "Super Cluedo Showcase (Maygay) v1.2 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluess	,0			,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.3 (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessh	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.3 (Newer) (Hack) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.3 (Newer) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessa	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v1.2 (Newer) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessap	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v1.2 (Newer) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessb	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v7.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessbp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v7.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessc	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v6.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluesscp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v6.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessd	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v5.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessdp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v5.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluesse	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessep	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessf	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v1.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessfp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v1.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessg	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v7.1 (15GBP Jackpot) (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessi	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.1 (10GBP Jackpot) (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessj	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.3 (5GBP Jackpot) (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessk	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v1.2 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessl	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v4.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluesslp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v4.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessm	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v3.1 (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessmp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v3.1 (Older) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessn	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v1.1 (10GBP Jackpot) (Older) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluesso	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.1 (Older, alternate) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessop	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v2.1 (Older, alternate) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessq	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v5.1 (Older, alternate) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessqp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v5.1 (Older, alternate) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessr	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v3.1 (Older, alternate) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluessrp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v3.1 (Older, alternate) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluesss	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v4.1? (Older, alternate) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 1996, m1cluesssp	,m1cluess	,m1,m1,m1, ROT0, "Maygay", "Cluedo Super Sleuth (Maygay) v4.1? (Older, alternate) (Protocol) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

GAME( 199?, m1coderd	,0			,m1,m1,m1, ROT0, "Maygay", "Code Red Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1coro		,0			,m1,m1,m1, ROT0, "Maygay", "Coronation Street (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1cororr	,0			,m1,m1,m1, ROT0, "Maygay", "Coronation Street - Rovers Return (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1corocb	,0			,m1,m1,m1, ROT0, "Maygay", "Coronation Street Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1corosh	,0			,m1,m1,m1, ROT0, "Maygay", "Coronation Street Showcase (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1criss		,0			,m1,m1,m1, ROT0, "Maygay", "Criss Cross Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1dxmono	,0			,m1,m1,m1, ROT0, "Maygay", "Deluxe Monopoly (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1dmnhrt	,0			,m1,m1,m1, ROT0, "Maygay", "Diamond Hearts (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1dm4ev		,0			,m1,m1,m1, ROT0, "Maygay", "Diamonds Are Forever Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1dkong		,0			,m1,m1,m1, ROT0, "Maygay", "Donkey Kong (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1ee		,0			,m1,m1,m1, ROT0, "Maygay", "Eastenders (Bwb) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1eea		,m1ee		,m1,m1,m1, ROT0, "Maygay", "Eastenders (Maygay - Bwb) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1eeb		,m1ee		,m1,m1,m1, ROT0, "Maygay", "Eastenders (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1eeqv		,0			,m1,m1,m1, ROT0, "Maygay", "Eastenders - Queen Vic (Maygay - Whitbread) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1eeqva		,m1eeqv		,m1,m1,m1, ROT0, "Maygay", "Eastenders - Queen Vic (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1expclb	,0			,m1,m1,m1, ROT0, "Maygay", "Explorer Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1fantfb	,0			,m1,m1,m1, ROT0, "Maygay", "Fantasy Football (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1fight		,0			,m1,m1,m1, ROT0, "Maygay", "Fight Night (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1fivest	,0			,m1,m1,m1, ROT0, "Maygay", "Five Star (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1frexpl	,0			,m1,m1,m1, ROT0, "Maygay", "Fruit Explosion (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1glad		,0			,m1,m1,m1, ROT0, "Maygay", "Gladiators (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1glada		,0			,m1,m1,m1, ROT0, "Maygay", "Gladiators Arcade (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1gladsh	,0			,m1,m1,m1, ROT0, "Maygay", "Gladiators Showcase (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1goldsv	,0			,m1,m1,m1, ROT0, "Maygay", "Gold & Silver (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1gold10	,0			,m1,m1,m1, ROT0, "Maygay", "Golden 10 (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1goldng	,0			,m1,m1,m1, ROT0, "Maygay", "Golden Nugget Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1gresc		,0			,m1,m1,m1, ROT0, "Maygay", "Great Escape, The (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1guvnor	,0			,m1,m1,m1, ROT0, "Maygay", "The Guvnor (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1htclb		,0			,m1,m1,m1, ROT0, "Maygay", "Hi Tension Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1hotpot	,0			,m1,m1,m1, ROT0, "Maygay", "Hot Pots (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1infern	,0			,m1,m1,m1, ROT0, "Maygay", "Inferno (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1imclb		,0			,m1,m1,m1, ROT0, "Maygay", "Instant Millionaire Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1inwin		,0			,m1,m1,m1, ROT0, "Maygay", "Instant Win (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1itsko		,0			,m1,m1,m1, ROT0, "Maygay", "It's A Knockout (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1itjob		,0			,m1,m1,m1, ROT0, "Maygay", "Italian Job (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1jpmult	,0			,m1,m1,m1, ROT0, "Maygay", "Jackpot Multiplier (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1jbond		,0			,m1,m1,m1, ROT0, "Maygay", "James Bond (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1jdwins	,0			,m1,m1,m1, ROT0, "Maygay", "Jim Davison's Winning Streak (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1liveam	,0			,m1,m1,m1, ROT0, "Maygay", "Living In America (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1lotmil	,0			,m1,m1,m1, ROT0, "Maygay", "Lottery Millionaire Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1luckno	,0			,m1,m1,m1, ROT0, "Maygay", "Lucky Numbers (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1magic		,0			,m1,m1,m1, ROT0, "Maygay", "Magic Squares (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1manhat	,0			,m1,m1,m1, ROT0, "Maygay", "Manhattan Skylines (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1bignit	,0			,m1,m1,m1, ROT0, "Maygay", "Mike Reid's Big Night Out (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1mongam	,0			,m1,m1,m1, ROT0, "Maygay", "Money Game Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1monmon	,0			,m1,m1,m1, ROT0, "Maygay", "Money Money Money (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1monod		,0			,m1,m1,m1, ROT0, "Maygay", "Monopoly (Maygay) [Dutch] (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1mono		,0			,m1,m1,m1, ROT0, "Maygay", "Monopoly (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1moncls	,0			,m1,m1,m1, ROT0, "Maygay", "Monopoly Classic (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1monclb	,0			,m1,m1,m1, ROT0, "Maygay", "Monopoly Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1monstr	,0			,m1,m1,m1, ROT0, "Maygay", "Monster Cash (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1nhp		,0			,m1,m1,m1, ROT0, "Maygay", "Noel's House Party (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1nudbnk	,0			,m1,m1,m1, ROT0, "Maygay", "Nudge Banker (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1nudunl	,0			,m1,m1,m1, ROT0, "Maygay", "Nudges Unlimited (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1omega		,0			,m1,m1,m1, ROT0, "Maygay", "Omega (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1onbus		,0			,m1,m1,m1, ROT0, "Maygay", "On The Buses (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1ott		,0			,m1,m1,m1, ROT0, "Maygay", "Over The Top (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1piggy		,0			,m1,m1,m1, ROT0, "Maygay", "Piggy Bank (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1ppd		,0			,m1,m1,m1, ROT0, "Maygay", "Pink Panther (Maygay) [Dutch] (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1pp		,0			,m1,m1,m1, ROT0, "Maygay", "Pink Panther (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1ppc		,0			,m1,m1,m1, ROT0, "Maygay", "Pink Panther Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1przclu	,0			,m1,m1,m1, ROT0, "Maygay", "Prize Cluedo (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1przee		,0			,m1,m1,m1, ROT0, "Maygay", "Prize Eastenders (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1reeldm	,0			,m1,m1,m1, ROT0, "Maygay", "Reel Diamonds (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1retpp		,0			,m1,m1,m1, ROT0, "Maygay", "Return Of The Pink Panther (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1search	,0			,m1,m1,m1, ROT0, "Maygay", "Search Light (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1sixspn	,0			,m1,m1,m1, ROT0, "Maygay", "Six Spinner (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1spid		,0			,m1,m1,m1, ROT0, "Maygay", "Spiderman (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1spid2		,0			,m1,m1,m1, ROT0, "Maygay", "Spiderman 2 (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1sptlgt	,0			,m1,m1,m1, ROT0, "Maygay", "Spotlight (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1stair		,0			,m1,m1,m1, ROT0, "Maygay", "Stairway To Heaven (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1sudnim	,0			,m1,m1,m1, ROT0, "Maygay", "Sudden Impact (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1sprnov	,0			,m1,m1,m1, ROT0, "Maygay", "Super Nova (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1suppot	,0			,m1,m1,m1, ROT0, "Maygay", "Super Pots (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1taknot	,0			,m1,m1,m1, ROT0, "Maygay", "Take Note (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1tstunt	,0			,m1,m1,m1, ROT0, "Maygay", "Test Unit (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1thatlf	,0			,m1,m1,m1, ROT0, "Maygay", "That's Life (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1topten	,0			,m1,m1,m1, ROT0, "Maygay", "Top Tenner (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1tp		,0			,m1,m1,m1, ROT0, "Maygay", "Trivial Pursuit (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1tpclb		,0			,m1,m1,m1, ROT0, "Maygay", "Trivial Pursuit Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1ultchl	,0			,m1,m1,m1, ROT0, "Maygay", "Ultimate Challenge (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1undsie	,0			,m1,m1,m1, ROT0, "Maygay", "Under Siege (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1vegas		,0			,m1,m1,m1, ROT0, "Maygay", "Vegas Gambler Club (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1wagon		,0			,m1,m1,m1, ROT0, "Maygay", "Wagon Trail (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1wotw		,0			,m1,m1,m1, ROT0, "Maygay", "War Of The Worlds (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1wldzne	,0			,m1,m1,m1, ROT0, "Maygay", "Wild Zone (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1winenc	,0			,m1,m1,m1, ROT0, "Maygay", "Winners Enclosure (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1simps		,0			,m1,m1,m1, ROT0, "Maygay", "The Simpsons (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

GAME( 199?, m1bghou		,0			,m1,m1,m1, ROT0, "Gemini", "Big Ghoulies (Gemini) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1crzco		,0			,m1,m1,m1, ROT0, "Gemini", "Crazy Cobra (Gemini) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1luxor		,0			,m1,m1,m1, ROT0, "Gemini", "Luxor Casino (Gemini) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

// guessing these are M1A/B based on data/code starting at 0x2800
GAME( 199?, m1hiloc		,0			,m1,m1,m1, ROT0, "Global", "Hi Lo Casino (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1jtjob		,0			,m1,m1,m1, ROT0, "Global", "Just The Job (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1kingsw	,0			,m1,m1,m1, ROT0, "Global", "King Of The Swingers (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1lca		,0			,m1,m1,m1, ROT0, "Global", "Lights Camera Action (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1mb		,0			,m1,m1,m1, ROT0, "Global", "Monkey Business (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1mbclb		,0			,m1,m1,m1, ROT0, "Global", "Monkey Business Club (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1sycc		,0			,m1,m1,m1, ROT0, "Global", "Stake Yer Claim Club (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1thrill	,0			,m1,m1,m1, ROT0, "Global", "Thrills 'n' Spills (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1trtr		,0			,m1,m1,m1, ROT0, "Global", "Trick Or Treat (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1trtrcl	,0			,m1,m1,m1, ROT0, "Global", "Trick Or Treat Club (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1vegcrw	,0			,m1,m1,m1, ROT0, "Global", "Vegetable Crew (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

GAME( 199?, m1calyps	,0			,m1,m1,m1, ROT0, "Maygay", "Calypso (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1cashln	,0			,m1,m1,m1, ROT0, "Maygay", "Cash Lines (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1races		,0			,m1,m1,m1, ROT0, "Maygay", "A Day At The Races (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1digdel	,0			,m1,m1,m1, ROT0, "Global", "Diggers Delight (Global) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1startr	,0			,m1,m1,m1, ROT0, "Mdm", "Star Trekking (Mdm) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1ttcash	,0			,m1,m1,m1, ROT0, "Empire", "Tick Tock Cash (Empire) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1topstr	,0			,m1,m1,m1, ROT0, "Maygay", "Top Strike (Maygay - Bwb) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1sirich	,0			,m1,m1,m1, ROT0, "Maygay", "Strike It Rich (Maygay) (M1A/B)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )
GAME( 199?, m1gskill	,0			,m1,m1,m1, ROT0, "Hitech Amusement", "Greek Skill (Hitech Amusement)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL ) // no idea if this is m1ab but the code starts at 2800

GAME( 19??, m1atunk		,0			,m1,m1,m1, ROT0, "Avantime?", "Random Runner (Avantime?)",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK|GAME_MECHANICAL )

