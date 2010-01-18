/*****************************************************************************************

    Maygay M1 A/B driver, (under heavy construction !!!)

    A.G.E Code Copyright J. Wallace and the AGEMAME Development Team.
    Visit http://www.mameworld.net/agemame/ for more information.

    M.A.M.E Core Copyright Nicola Salmoria and the MAME Team,
    used under license from http://mamedev.org

    This only loads the basic stuff - there needs to be more done to make this run.
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

#define VERBOSE 1
#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define M1_MASTER_CLOCK (XTAL_8MHz)
#define M1_DUART_CLOCK  (XTAL_3_6864MHz)

static struct
{
	running_device *duart68681;
} maygaym1_devices;

static UINT8  lamppos;
static int  alpha_clock;
static int  RAMEN;
static int  ALARMEN;
//static int  m1_enable;
static int  PSUrelay;
static int  WDOG;
static int  SRSEL;

static UINT8 Lamps[256];      // 256 multiplexed lamps
static int optic_pattern;

typedef struct _i8279_state i8279_state;
struct _i8279_state
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

static i8279_state i8279[2];

static void m1_draw_lamps(int data,int strobe, int col)
{
	int i;
	int scramble[8] = { 0x10, 0x20, 0x40, 0x80, 0x01, 0x02, 0x04, 0x08 };

	lamppos = strobe + col * 8;

	for ( i = 0; i < 8; i++ )
	{
		Lamps[lamppos] = ( data & scramble[i] );
		output_set_lamp_value(lamppos, Lamps[lamppos]);
    }
	lamppos++;
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
	i8279_state *chip = i8279 + 0;
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
				result = input_port_read(space->machine,"SW1");
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
			result = input_port_read(space->machine,portnames[chip->sense_address]);
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
	i8279_state *chip = i8279 + 0;
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

static READ8_HANDLER( m1_8279_2_r )
{
	i8279_state *chip = i8279 + 1;
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
	i8279_state *chip = i8279 + 1;
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

static void m1_stepper_reset(void)
{
	int pattern = 0,i;
	for ( i = 0; i < 6; i++)
	{
		stepper_reset_position(i);
		if ( stepper_optic_state(i) ) pattern |= 1<<i;
	}
	optic_pattern = pattern;
}

static MACHINE_RESET( m1 )
{
	ROC10937_reset(0);	// reset display1
	maygaym1_devices.duart68681 = devtag_get_device( machine, "duart68681" );
	m1_stepper_reset();
}

///////////////////////////////////////////////////////////////////////////

static void duart_irq_handler(running_device *device, UINT8 state)
{
	cputag_set_input_line(device->machine, "maincpu", M6809_IRQ_LINE, state?ASSERT_LINE:CLEAR_LINE);
	LOG(("6809 irq%d \n",state));
}

#if 0
static void cpu0_firq(int state)
{
	cpunum_set_input_line(Machine, 0, M6809_FIRQ_LINE, state?ASSERT_LINE:CLEAR_LINE);
	LOG(("6809 firq%d \n",state));
}

static void cpu0_nmi(int state)
{
	cpunum_set_input_line(Machine, 0, INPUT_LINE_NMI, state?ASSERT_LINE:CLEAR_LINE);
	LOG(("6809 nmi%d \n",state));
}
#endif

/***************************************************************************
    6821 PIA
***************************************************************************/

static WRITE8_DEVICE_HANDLER( m1_pia_porta_w )
{
	if ( data & 0x40 ) ROC10937_reset(0);

	if ( !alpha_clock && (data & 0x20) )
	{
		ROC10937_shift_data(0, ( data & 0x10 )?0:1);
	}

	alpha_clock = data & 0x20;

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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Hi")
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
	Mechmtr_init(8);

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
	stepper_update(0, data & 0x0F );
	stepper_update(1, (data>>4) & 0x0F );

	if ( stepper_optic_state(0) ) optic_pattern |=  0x01;
	else                          optic_pattern &= ~0x01;
	if ( stepper_optic_state(1) ) optic_pattern |=  0x02;
	else                          optic_pattern &= ~0x02;

	awp_draw_reel(0);
	awp_draw_reel(1);
}

static WRITE8_HANDLER( reel34_w )
{
	stepper_update(2, data & 0x0F );
	stepper_update(3, (data>>4) & 0x0F );

	if ( stepper_optic_state(2) ) optic_pattern |=  0x04;
	else                          optic_pattern &= ~0x04;
	if ( stepper_optic_state(3) ) optic_pattern |=  0x08;
	else                          optic_pattern &= ~0x08;

	awp_draw_reel(2);
	awp_draw_reel(3);
}

static WRITE8_HANDLER( reel56_w )
{
	stepper_update(4, data & 0x0F );
	stepper_update(5, (data>>4) & 0x0F );

	if ( stepper_optic_state(4) ) optic_pattern |=  0x10;
	else                          optic_pattern &= ~0x10;
	if ( stepper_optic_state(5) ) optic_pattern |=  0x20;
	else                          optic_pattern &= ~0x20;

	awp_draw_reel(4);
	awp_draw_reel(5);
}

static UINT8 m1_duart_r (running_device *device)
{
	return (optic_pattern);
}

static WRITE8_DEVICE_HANDLER( m1_meter_w )
{
	int i;
	UINT64 cycles  = cputag_get_total_cycles(device->machine, "maincpu");

	for (i=0; i<8; i++)
	if ( data & (1 << i) )	Mechmtr_update(i, cycles, data & (1 << i) );
}

static WRITE8_HANDLER( m1_latch_w )
{
	switch ( offset )
	{
		case 0: // RAMEN
		RAMEN = (data & 1);
		break;
		case 1: // AlarmEn
        ALARMEN = (data & 1);
		break;
		case 2: // Enable
//      if ( m1_enable == 0 && ( data & 1 ) && Vmm )
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
		PSUrelay = (data & 1);
		break;
		case 5: // WDog
		WDOG = (data & 1);
		break;
		case 6: // Srsel
		SRSEL = (data & 1);
		break;
	}
}


static ADDRESS_MAP_START( m1_memmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)

	AM_RANGE(0x2000, 0x2000) AM_WRITE(reel12_w)
	AM_RANGE(0x2010, 0x2010) AM_WRITE(reel34_w)
	AM_RANGE(0x2020, 0x2020) AM_WRITE(reel56_w)

	AM_RANGE(0x2030, 0x2031) AM_READWRITE(m1_8279_r,m1_8279_w)
	AM_RANGE(0x2040, 0x2041) AM_READWRITE(m1_8279_2_r,m1_8279_2_w)
	AM_RANGE(0x2050, 0x2050)// SCAN on M1B

	AM_RANGE(0x2070, 0x207f) AM_DEVREADWRITE( "duart68681", duart68681_r, duart68681_w )

	AM_RANGE(0x2090, 0x2091) AM_DEVWRITE("aysnd", ay8910_address_data_w)
	AM_RANGE(0x20B0, 0x20B0) AM_DEVREAD("aysnd", ay8910_r)

	AM_RANGE(0x20A0, 0x20A3) AM_DEVWRITE("pia", pia6821_w)
	AM_RANGE(0x20A0, 0x20A3) AM_DEVREAD("pia", pia6821_r)

	AM_RANGE(0x20C0, 0x20C7) AM_WRITE(m1_latch_w)

	AM_RANGE(0x2400, 0x2401) AM_DEVWRITE( "ymsnd", ym2413_w )

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

static MACHINE_DRIVER_START( m1 )

	MDRV_MACHINE_START(m1)
	MDRV_MACHINE_RESET(m1)
	MDRV_CPU_ADD("maincpu", M6809, M1_MASTER_CLOCK/2)
	MDRV_CPU_PROGRAM_MAP(m1_memmap)

	MDRV_DUART68681_ADD("duart68681", M1_DUART_CLOCK, maygaym1_duart68681_config)
	MDRV_PIA6821_ADD("pia", m1_pia_intf)

	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("aysnd",AY8913, M1_MASTER_CLOCK)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("ymsnd", YM2413, M1_MASTER_CLOCK/4)

	MDRV_SOUND_ADD("msm6376", OKIM6376, M1_MASTER_CLOCK/4) //?
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_DEFAULT_LAYOUT(layout_awpvid16)
MACHINE_DRIVER_END

ROM_START( m_sptlgt )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASE00  )
	ROM_LOAD( "sa2-412.bin",0x0000, 0x10000,  CRC(17531aad) SHA1(decec517b89be9019913be59c5fC2aa2ee6e3f8f))
ROM_END

GAME( 199?, m_sptlgt,0,  m1,m1,0, ROT0, "Maygay Machines Ltd.", "Spotlight",GAME_NOT_WORKING|GAME_NO_SOUND|GAME_REQUIRES_ARTWORK )
