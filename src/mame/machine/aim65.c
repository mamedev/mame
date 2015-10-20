// license:GPL-2.0+
// copyright-holders:Peter Trauner, Dan Boris, Dirk Best, Robbbert
/******************************************************************************

 AIM65

******************************************************************************/


#include "includes/aim65.h"


/******************************************************************************
 Interrupt handling
******************************************************************************/

/* STEP/RUN
 *
 * Switch S2 (STEP/RUN) causes AIM 65 to operate either in the RUN mode or the
 * single STEP mode. In the STEP mode, the NMI interrupt line is driven low
 * when SYNC and O2 go high during instruction execution if the address lines
 * are outside the A000-FFFF range. The NMI interrupt occurs on the high to
 * low transition of the NMI line. The Monitor software will trace instructions
 * and register, outside the Monitor instruction address range if the trace
 * modes are selected and the NMI Interrupt Routine is not bypassed.
 */



/******************************************************************************
 6821 PIA
******************************************************************************/

/* PA0: A0 (Address)
 * PA1: A1 (Address)
 * PA2: CE1 (Chip enable)
 * PA3: CE2 (Chip enable)
 * PA4: CE3 (Chip enable)
 * PA5: CE4 (Chip enable)
 * PA6: CE5 (Chip enable)
 * PA7: W (Write enable)
 * PB0-6: D0-D6 (Data)
 * PB7: CU (Cursor)
 */

void aim65_state::dl1416_update(dl1416_device *device, int index)
{
	device->ce_w(m_pia_a & (0x04 << index));
	device->wr_w(BIT(m_pia_a, 7));
	device->cu_w(BIT(m_pia_b, 7));
	device->data_w(generic_space(), m_pia_a & 0x03, m_pia_b & 0x7f);
}

void aim65_state::aim65_pia()
{
	dl1416_update(m_ds1, 0);
	dl1416_update(m_ds2, 1);
	dl1416_update(m_ds3, 2);
	dl1416_update(m_ds4, 3);
	dl1416_update(m_ds5, 4);
}


WRITE8_MEMBER( aim65_state::aim65_pia_a_w )
{
	m_pia_a = data;
	aim65_pia();
}


WRITE8_MEMBER( aim65_state::aim65_pia_b_w )
{
	m_pia_b = data;
	aim65_pia();
}


WRITE16_MEMBER( aim65_state::aim65_update_ds1)
{
	output_set_digit_value(0 + (offset ^ 3), data);
}

WRITE16_MEMBER( aim65_state::aim65_update_ds2)
{
	output_set_digit_value(4 + (offset ^ 3), data);
}

WRITE16_MEMBER( aim65_state::aim65_update_ds3)
{
	output_set_digit_value(8 + (offset ^ 3), data);
}

WRITE16_MEMBER( aim65_state::aim65_update_ds4)
{
	output_set_digit_value(12 + (offset ^ 3), data);
}

WRITE16_MEMBER( aim65_state::aim65_update_ds5)
{
	output_set_digit_value(16 + (offset ^ 3), data);
}



/******************************************************************************
 6532 RIOT
******************************************************************************/


READ8_MEMBER( aim65_state::aim65_riot_b_r )
{
	static const char *const keynames[] =
	{
		"keyboard_0", "keyboard_1", "keyboard_2", "keyboard_3",
		"keyboard_4", "keyboard_5", "keyboard_6", "keyboard_7"
	};

	UINT8 row, data = 0xff;

	/* scan keyboard rows */
	for (row = 0; row < 8; row++)
	{
		if (!BIT(m_riot_port_a, row))
			data &= ioport(keynames[row])->read();
	}

	return data;
}


WRITE8_MEMBER( aim65_state::aim65_riot_a_w )
{
	m_riot_port_a = data;
}



/***************************************************************************
    DRIVER INIT
***************************************************************************/

void aim65_state::machine_start()
{
	ram_device *ram = m_ram;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	// Init ROM sockets
	if (m_z24->exists())
		space.install_read_handler(0xd000, 0xdfff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_z24));
	if (m_z25->exists())
		space.install_read_handler(0xc000, 0xcfff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_z25));
	if (m_z26->exists())
		space.install_read_handler(0xb000, 0xbfff, read8_delegate(FUNC(generic_slot_device::read_rom),(generic_slot_device*)m_z26));

	// Init RAM
	space.install_ram(0x0000, ram->size() - 1, ram->pointer());

	m_pb_save = 0;
}


/* RESET
 *
 * Pushbutton switch S1 initiates RESET of the AIM65 hardware and software.
 * Timer Z4 holds the RES low for at least 15 ms from the time the pushbutton
 * is released. RES is routed to the R6502 CPU, the Monitor R6522 (Z32), the
 * Monitor R6532 RIOT (Z33), the user R6522 VIA (Z1), and the display R6520 PIA
 * (U1). To initiate the device RESET function is also routed to the expansion
 * connector for off-board RESET functions. The Monitor performs a software
 * reset when the RES line goes high.
 */


/******************************************************************************
 Cassette
******************************************************************************/

WRITE8_MEMBER( aim65_state::aim65_pb_w )
{
/*
    d7 = cass out (both decks)
    d5 = cass2 motor
    d4 = cass1 motor
    d2 = tty out (not emulated)
    d0/1 = printer data (not emulated)
*/

	UINT8 bits = data ^ m_pb_save;
	m_pb_save = data;

	if (BIT(bits, 7))
	{
		m_cassette1->output(BIT(data, 7) ? -1.0 : +1.0);
		m_cassette2->output(BIT(data, 7) ? -1.0 : +1.0);
	}

	if (BIT(bits, 5))
	{
		if (BIT(data, 5))
			m_cassette2->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		else
			m_cassette2->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
	}

	if (BIT(bits, 4))
	{
		if (BIT(data, 4))
			m_cassette1->change_state(CASSETTE_MOTOR_DISABLED,CASSETTE_MASK_MOTOR);
		else
			m_cassette1->change_state(CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR);
	}
}


READ8_MEMBER( aim65_state::aim65_pb_r )
{
/*
    d7 = cassette in (deck 1)
    d6 = tty in (not emulated)
    d3 = kb/tty switch
*/

	UINT8 data = ioport("switches")->read();
	data |= (m_cassette1->input() > +0.03) ? 0x80 : 0;
	data |= 0x40; // TTY must be H if not used.
	data |= m_pb_save & 0x37;
	return data;
}


/******************************************************************************
 Printer
******************************************************************************/

#ifdef UNUSED_FUNCTION
/*

2012-01-24 Printer code removed [Robbbert]

From here to the end is not compiled. It is the remnants of the old printer
code. Have a look at version 0.114 or 0.115 for the original working code.

I've left it here for the idly curious.

The system sends out the pattern of dots to the 'dot-matrix' print-head.

This is why we can't simply replace it with a Centronics-like 'Printer'
device - the output will be gibberish.
*/



/*
  aim65 thermal printer (20 characters)
  10 heat elements (place on 1 line, space between 2 characters(about 14dots))
  (pa0..pa7,pb0,pb1 1 heat element on)

  cb2 0 motor, heat elements on
  cb1 output start!?
  ca1 input

  normally printer 5x7 characters
  (horizontal movement limits not known, normally 2 dots between characters)

  3 dots space between lines?
*/


/* Part of aim65_pb_w

    data &= 0x03;

    if (m_flag_b == 0)
    {
        printerRAM[(m_printer_y * 20) + m_printer_x ] |= (data << 8);
        m_flag_b = 1;
    }
*/


/* Items for driver state

    emu_timer *m_print_timer;
    int m_printer_x;
    int m_printer_y;
    bool m_printer_dir;
    bool m_flag_a;
    bool m_flag_b;
    bool m_printer_level;
    UINT16 *m_printerRAM;
*/


/* Other items from H file

VIDEO_UPDATE( aim65 );
*/


/* From 6522 config
DRIVER_MEMBER(aim65_state, aim65_pa_w), // out port A
DRIVER_MEMBER(aim65_state, aim65_printer_on), // out CB2
*/


/* From Machine Config
    MCFG_VIDEO_START_OVERRIDE(aim65_state,aim65)
    MCFG_VIDEO_UPDATE(aim65)
*/


TIMER_CALLBACK_MEMBER(aim65_state::aim65_printer_timer)
{
	via6522_device *via_0 = machine().device<via6522_device>("via6522_0");

	via_0->write_cb1(m_printer_level);
	via_0->write_cb1(!m_printer_level);
	m_printer_level ^= 1;

	if (m_printer_dir)
	{
		if (m_printer_x > 0)
			m_printer_x--;
		else
		{
			m_printer_dir = 0;
			m_printer_x++;
			m_printer_y++;
		}
	}
	else
	{
		if (m_printer_x < 9)
			m_printer_x++;
		else
		{
			m_printer_dir = 1;
			m_printer_x--;
			m_printer_y++;
		}
	}

	if (m_printer_y > 500) m_printer_y = 0;

	m_flag_a=0;
	m_flag_b=0;
}


WRITE8_MEMBER( aim65_state::aim65_printer_on )
{
	via6522_device *via_0 = machine().device<via6522_device>("via6522_0");
	if (!data)
	{
		m_printer_x=0;
		m_printer_y++;
		if (m_printer_y > 500) m_printer_y = 0;
		m_flag_a = m_flag_b=0;
		via_0->write_cb1(0);
		m_print_timer->adjust(attotime::zero, 0, attotime::from_usec(10));
		m_printer_level = 1;
	}
	else
		m_print_timer->reset();
}


WRITE8_MEMBER( aim65_state::aim65_pa_w )
{
// All bits are for printer data (not emulated)
	if (m_flag_a == 0)
	{
		m_printerRAM[(m_printer_y * 20) + m_printer_x] |= data;
		m_flag_a = 1;
	}
}

VIDEO_START_MEMBER(aim65_state,aim65)
{
	m_print_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(aim65_state::aim65_printer_timer),this));
	m_printerRAM = auto_alloc_array(machine(), UINT16, (600 * 10 * 2) / 2);
	memset(m_printerRAM, 0, videoram_size);
	VIDEO_START_CALL_MEMBER(generic);
	m_printer_x = 0;
	m_printer_y = 0;
	m_printer_dir = 0;
	m_flag_a = 0;
	m_flag_b = 0;
	m_printer_level = 0;
}

SCREEN_UPDATE( aim65 )
{
	aim65_state *state = screen.machine().driver_data<aim65_state>();
	/* Display printer output */
	bool dir = 1;
	UINT8 b,x,pen;
	UINT16 y;

	for (y = 0; y<500; y++)
	{
		for(x = 0; x< 10; x++)
		{
			if (dir == 1)
				data = state->m_printerRAM[y * 10  + x];
			else
				data = state->m_printerRAM[(y * 10) + (9 - x)];


			for (b = 0; b<10; b++)
			{
				pen = screen.m_palette->pen(BIT(data, 0) ? 2 : 0);
				plot_pixel(bitmap,700 - ((b * 10) + x), y, pen);
				data >>= 1;
			}
		}

		dir ^= 1;
	}
	return 0;
}


#endif
