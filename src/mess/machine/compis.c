/******************************************************************************

    compis.c
    machine driver

    Per Ola Ingvarsson
    Tomas Karlsson

 ******************************************************************************/

/*-------------------------------------------------------------------------*/
/* Include files                                                           */
/*-------------------------------------------------------------------------*/

#include "includes/compis.h"

/* Main emulation */

/*-------------------------------------------------------------------------*/
/* Bit 0: J5-4                                                             */
/* Bit 1: J5-5                                                     */
/* Bit 2: J6-3 Cassette read                                               */
/* Bit 3: J2-6 DSR / S8-4 Test                                             */
/* Bit 4: J4-6 DSR / S8-3 Test                                             */
/* Bit 5: J7-11 Centronics BUSY                                            */
/* Bit 6: J7-13 Centronics SELECT                              */
/* Bit 7: Tmr0                                                     */
/*-------------------------------------------------------------------------*/
READ8_MEMBER( compis_state::compis_ppi_port_b_r )
{
	UINT8 data;

	/* DIP switch - Test mode */
	data = ioport("DSW0")->read();

	// cassette
	data |= (m_cassette->input() > 0.0) << 2;

	/* Centronics busy */
	data |= m_centronics->busy_r() << 5;
	data |= m_centronics->vcc_r() << 6;

	// TMR0
	data |= m_tmr0 << 7;

	return data;
}

/*-------------------------------------------------------------------------*/
/* Bit 0: J5-1                                                         */
/* Bit 1: J5-2                                                         */
/* Bit 2: Select: 1=time measure, DSR from J2/J4 pin 6. 0=read cassette    */
/* Bit 3: Datex: Tristate datex output (low)                   */
/* Bit 4: V2-5 Floppy motor on/off                                     */
/* Bit 5: J7-1 Centronics STROBE                                       */
/* Bit 6: V2-4 Floppy Soft reset                               */
/* Bit 7: V2-3 Floppy Terminal count                                   */
/*-------------------------------------------------------------------------*/
WRITE8_MEMBER( compis_state::compis_ppi_port_c_w )
{
	m_isbx0->opt1_w(BIT(data, 4));

	m_centronics->strobe_w(BIT(data, 5));

	if (BIT(data, 6))
		m_isbx0->reset();

	m_isbx0->opt0_w(BIT(data, 7));
}

I8255A_INTERFACE( compis_ppi_interface )
{
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER("centronics", centronics_device, write),
	DEVCB_DRIVER_MEMBER(compis_state, compis_ppi_port_b_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(compis_state, compis_ppi_port_c_w)
};


/*-------------------------------------------------------------------------*/
/*  PIT 8253                                                               */
/*-------------------------------------------------------------------------*/

WRITE_LINE_MEMBER( compis_state::tmr3_w )
{
	m_mpsc->rxtxcb_w(state);
}

WRITE_LINE_MEMBER( compis_state::tmr4_w )
{
}

WRITE_LINE_MEMBER( compis_state::tmr5_w )
{
	m_mpsc->rxca_w(state);
	m_mpsc->txca_w(state);
}

const struct pit8253_interface compis_pit8253_config =
{
	{
		/* Timer0 */
		{XTAL_16MHz/8, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(compis_state, tmr3_w) },
		/* Timer1 */
		{XTAL_16MHz/8, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(compis_state, tmr4_w) },
		/* Timer2 */
		{XTAL_16MHz/8, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(compis_state, tmr5_w) }
	}
};

WRITE_LINE_MEMBER( compis_state::tmr2_w )
{
	m_uart->rxc_w(state);
	m_uart->txc_w(state);
}

const struct pit8253_interface compis_pit8254_config =
{
	{
		/* Timer0 */
		{XTAL_16MHz/2, DEVCB_LINE_VCC, DEVCB_NULL /*DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir3_w)*/ }, // SYSTICK
		/* Timer1 */
		{XTAL_16MHz/2, DEVCB_LINE_VCC, DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir7_w) }, // DELAY
		/* Timer2 */
		{7932659, DEVCB_LINE_VCC, DEVCB_DRIVER_LINE_MEMBER(compis_state, tmr2_w) } // BAUD
	}
};


/*-------------------------------------------------------------------------*/
/*  USART 8251                                                             */
/*-------------------------------------------------------------------------*/
const i8251_interface compis_usart_interface=
{
	DEVCB_DEVICE_LINE_MEMBER(COMPIS_KEYBOARD_TAG, compis_keyboard_device, so_r),
	DEVCB_DEVICE_LINE_MEMBER(COMPIS_KEYBOARD_TAG, compis_keyboard_device, si_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER("pic8259_master", pic8259_device, ir2_w),
	DEVCB_NULL, //DEVCB_DEVICE_LINE_MEMBER("maincpu", i80186_cpu_device, int1_w),
	DEVCB_NULL,
	DEVCB_NULL
};

/*-------------------------------------------------------------------------*/
/* Name: compis                                                            */
/* Desc: Driver - Init                                                     */
/*-------------------------------------------------------------------------*/

/*************************************************************
 *
 * pic8259 configuration
 *
 *************************************************************/

READ8_MEMBER(compis_state::compis_irq_callback)
{
	return m_8259m->inta_r();
}

WRITE_LINE_MEMBER( compis_state::tmr0_w )
{
	m_tmr0 = state;
	
	m_cassette->output(m_tmr0 ? -1 : 1);
}

WRITE8_MEMBER( compis_state::tape_mon_w )
{
	cassette_state state = BIT(data, 0) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED;

	m_cassette->change_state(state, CASSETTE_MASK_MOTOR);
}

TIMER_DEVICE_CALLBACK_MEMBER( compis_state::tape_tick )
{
	m_maincpu->tmrin0_w(m_cassette->input() > 0.0);
}

void compis_state::machine_start()
{
}

/*-------------------------------------------------------------------------*/
/* Name: compis                                                            */
/* Desc: Machine - Init                                                    */
/*-------------------------------------------------------------------------*/
void compis_state::machine_reset()
{
	m_uart->reset();
	m_mpsc->reset();
	m_8255->reset();
	m_isbx0->reset();
	m_isbx1->reset();
}
