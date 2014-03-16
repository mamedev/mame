

#include "includes/kaypro.h"




/***********************************************************

    PIO

    Port B is unused on both PIOs

************************************************************/

WRITE_LINE_MEMBER(kaypro_state::kaypro_interrupt)
{
	m_maincpu->set_input_line(0, state);
}

WRITE_LINE_MEMBER( kaypro_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

READ8_MEMBER( kaypro_state::pio_system_r )
{
	UINT8 data = 0;

	/* centronics busy */
	data |= m_centronics_busy << 3;

	/* PA7 is pulled high */
	data |= 0x80;

	return data;
}

WRITE8_MEMBER( kaypro_state::kayproii_pio_system_w )
{
/*  d7 bank select
    d6 disk drive motors - (0=on)
    d5 double-density enable (0=double density)
    d4 Centronics strobe
    d2 side select (1=side 1)
    d1 drive B
    d0 drive A */

	membank("bankr0")->set_entry(BIT(data, 7));
	membank("bank3")->set_entry(BIT(data, 7));

	m_floppy = NULL;
	if (BIT(data, 0))
		m_floppy = m_floppy0->get_device();
	else
	if (BIT(data, 1))
		m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);
	m_fdc->dden_w(BIT(data, 5));

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(data, 6)); // motor on
	}

	output_set_value("ledA", BIT(data, 0));     /* LEDs in artwork */
	output_set_value("ledB", BIT(data, 1));

	m_centronics->write_strobe(BIT(data, 4));

	m_system_port = data;
}

WRITE8_MEMBER( kaypro_state::kaypro4_pio_system_w )
{
	kayproii_pio_system_w(space, offset, data);

	/* side select */
	m_floppy->ss_w(BIT(data, 2));
}

const z80pio_interface kayproii_pio_g_intf =
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("cent_data_out", output_latch_device, write),
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL          /* portB ready active callback */
};

const z80pio_interface kayproii_pio_s_intf =
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_DRIVER_MEMBER(kaypro_state, pio_system_r),    /* read printer status */
	DEVCB_DRIVER_MEMBER(kaypro_state, kayproii_pio_system_w),   /* activate various internal devices */
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL          /* portB ready active callback */
};

const z80pio_interface kaypro4_pio_s_intf =
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0),
	DEVCB_DRIVER_MEMBER(kaypro_state, pio_system_r),    /* read printer status */
	DEVCB_DRIVER_MEMBER(kaypro_state, kaypro4_pio_system_w),    /* activate various internal devices */
	DEVCB_NULL,         /* portA ready active callback */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL          /* portB ready active callback */
};

/***********************************************************

    KAYPRO2X SYSTEM PORT

    The PIOs were replaced by a few standard 74xx chips

************************************************************/

READ8_MEMBER( kaypro_state::kaypro2x_system_port_r )
{
	UINT8 data = m_centronics_busy << 6;
	return (m_system_port & 0xbf) | data;
}

WRITE8_MEMBER( kaypro_state::kaypro2x_system_port_w )
{
/*  d7 bank select
    d6 alternate character set (write only)
    d5 double-density enable
    d4 disk drive motors (1=on)
    d3 Centronics strobe
    d2 side select (appears that 0=side 1?)
    d1 drive B
    d0 drive A */

	membank("bankr0")->set_entry(BIT(data, 7));
	membank("bank3")->set_entry(BIT(data, 7));

	m_floppy = NULL;
	if (!BIT(data, 0))
		m_floppy = m_floppy0->get_device();
	else
	if (!BIT(data, 1))
		m_floppy = m_floppy1->get_device();

	m_fdc->set_floppy(m_floppy);
	//m_fdc->dden_w(BIT(data, 5)); // not connected

	if (m_floppy)
	{
		m_floppy->mon_w(!BIT(data, 4)); // motor on
		m_floppy->ss_w(!BIT(data, 2));
	}

	output_set_value("ledA", BIT(data, 0));     /* LEDs in artwork */
	output_set_value("ledB", BIT(data, 1));

	m_centronics->write_strobe(BIT(data, 3));

	m_system_port = data;
}



/***********************************************************************

    SIO

    On Kaypro2x, Channel B on both SIOs is hardwired to 300 baud.

    Both devices on sio2 (printer and modem) are not emulated.

************************************************************************/

/* Set baud rate. bits 0..3 Rx and Tx are tied together. Baud Rate Generator is a AY-5-8116, SMC8116, WD1943, etc.
    00h    50
    11h    75
    22h    110
    33h    134.5
    44h    150
    55h    300
    66h    600
    77h    1200
    88h    1800
    99h    2000
    AAh    2400
    BBh    3600
    CCh    4800
    DDh    7200
    EEh    9600
    FFh    19200 */


const z80sio_interface kaypro_sio_intf =
{
	DEVCB_DRIVER_LINE_MEMBER(kaypro_state,kaypro_interrupt),    /* interrupt handler */
	DEVCB_NULL,         /* DTR changed handler */
	DEVCB_NULL,         /* RTS changed handler */
	DEVCB_NULL,         /* BREAK changed handler */
	DEVCB_NULL,         /* transmit handler - which channel is this for? */
	DEVCB_NULL          /* receive handler - which channel is this for? */
};

READ8_MEMBER(kaypro_state::kaypro_sio_r)
{
	if (!offset)
		return dynamic_cast<z80sio_device*>(machine().device("z80sio"))->data_read(0);
	else
	if (offset == 1)
//      return z80sio_d_r(machine().device("z80sio"), 1);
		return kay_kbd_d_r(machine());
	else
	if (offset == 2)
		return dynamic_cast<z80sio_device*>(machine().device("z80sio"))->control_read(0);
	else
//      return z80sio_c_r(machine().device("z80sio"), 1);
		return kay_kbd_c_r(machine());
}

WRITE8_MEMBER(kaypro_state::kaypro_sio_w)
{
	if (!offset)
		dynamic_cast<z80sio_device*>(machine().device("z80sio"))->data_write(0, data);
	else
	if (offset == 1)
//      z80sio_d_w(machine().device("z80sio"), 1, data);
		kay_kbd_d_w(machine(), data);
	else
	if (offset == 2)
		dynamic_cast<z80sio_device*>(machine().device("z80sio"))->control_write(0, data);
	else
		dynamic_cast<z80sio_device*>(machine().device("z80sio"))->control_write(1, data);
}


/*************************************************************************************

    Floppy DIsk

    If DRQ or IRQ is set, and cpu is halted, the NMI goes low.
    Since the HALT occurs last (and has no callback mechanism), we need to set
    a short delay, to give time for the processor to execute the HALT before NMI
    becomes active.

*************************************************************************************/

void kaypro_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_FLOPPY:
		if (m_maincpu->state_int(Z80_HALT))
			m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
		break;
	default:
		assert_always(FALSE, "Unknown id in kaypro_state::device_timer");
	}
}

WRITE_LINE_MEMBER( kaypro_state::fdc_intrq_w )
{
	if (state)
		timer_set(attotime::zero, TIMER_FLOPPY);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

WRITE_LINE_MEMBER( kaypro_state::fdc_drq_w )
{
	if (state)
		timer_set(attotime::zero, TIMER_FLOPPY);
	else
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

}


/***********************************************************

    Machine

************************************************************/
MACHINE_START_MEMBER( kaypro_state,kayproii )
{
	m_pio_s->strobe_a(0);
}

MACHINE_RESET_MEMBER( kaypro_state,kaypro )
{
	MACHINE_RESET_CALL_MEMBER(kay_kbd);
	membank("bankr0")->set_entry(1); // point at rom
	membank("bankw0")->set_entry(0); // always write to ram
	membank("bank3")->set_entry(1); // point at video ram
	m_system_port = 0x80;
	m_maincpu->reset();
}


/***********************************************************

    Quickload

    This loads a .COM file to address 0x100 then jumps
    there. Sometimes .COM has been renamed to .CPM to
    prevent windows going ballistic. These can be loaded
    as well.

************************************************************/

QUICKLOAD_LOAD_MEMBER( kaypro_state, kaypro )
{
	UINT8 *RAM = memregion("rambank")->base();
	UINT16 i;
	UINT8 data;

	/* Load image to the TPA (Transient Program Area) */
	for (i = 0; i < quickload_size; i++)
	{
		if (image.fread( &data, 1) != 1) return IMAGE_INIT_FAIL;

		RAM[i+0x100] = data;
	}

	membank("bankr0")->set_entry(0);
	membank("bank3")->set_entry(0);
	RAM[0x80]=0;                            // clear out command tail
	RAM[0x81]=0;
	m_maincpu->set_pc(0x100);                // start program
	return IMAGE_INIT_PASS;
}
