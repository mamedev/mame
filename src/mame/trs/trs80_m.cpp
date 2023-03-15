// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller, Robbbert
//***************************************************************************

#include "emu.h"
#include "trs80.h"


#define IRQ_M1_RTC      0x80    /* RTC on Model I */
#define IRQ_M1_FDC      0x40    /* FDC on Model I */


TIMER_CALLBACK_MEMBER(trs80_state::cassette_data_callback)
{
// This does all baud rates. 250 baud (trs80), and 500 baud (all others) set bit 7 of "cassette_data".

	double new_val = (m_cassette->input());

	/* Check for HI-LO transition */
	if ( m_old_cassette_val > -0.2 && new_val < -0.2 )
		m_cassette_data = true;

	m_old_cassette_val = new_val;
}


/*************************************
 *
 *              Port handlers.
 *
 *************************************/


u8 trs80_state::port_e8_r()
{
/* not emulated
    d7 Clear-to-Send (CTS), Pin 5
    d6 Data-Set-Ready (DSR), pin 6
    d5 Carrier Detect (CD), pin 8
    d4 Ring Indicator (RI), pin 22
    d3,d2,d0 Not used
    d1 UART Receiver Input, pin 20 (pin 20 is also DTR) */

	return 0;
}

u8 trs80_state::port_ea_r()
{
/* UART Status Register
    d7 Data Received ('1'=condition true)
    d6 Transmitter Holding Register empty ('1'=condition true)
    d5 Overrun Error ('1'=condition true)
    d4 Framing Error ('1'=condition true)
    d3 Parity Error ('1'=condition true)
    d2..d0 Not used */

	u8 data=7;
	m_uart->write_swe(0);
	data |= m_uart->tbmt_r() ? 0x40 : 0;
	data |= m_uart->dav_r( ) ? 0x80 : 0;
	data |= m_uart->or_r(  ) ? 0x20 : 0;
	data |= m_uart->fe_r(  ) ? 0x10 : 0;
	data |= m_uart->pe_r(  ) ? 0x08 : 0;
	m_uart->write_swe(1);

	return data;
}

void trs80_state::port_e8_w(u8 data)
{
	m_reg_load = BIT(data, 1);
}

void trs80_state::port_ea_w(u8 data)
{
	if (m_reg_load)

/* d2..d0 not emulated
    d7 Even Parity Enable ('1'=even, '0'=odd)
    d6='1',d5='1' for 8 bits
    d6='0',d5='1' for 7 bits
    d6='1',d5='0' for 6 bits
    d6='0',d5='0' for 5 bits
    d4 Stop Bit Select ('1'=two stop bits, '0'=one stop bit)
    d3 Parity Inhibit ('1'=disable; No parity, '0'=parity enabled)
    d2 Break ('0'=disable transmit data; continuous RS232 'SPACE' condition)
    d1 Request-to-Send (RTS), pin 4
    d0 Data-Terminal-Ready (DTR), pin 20 */

	{
		m_uart->write_cs(0);
		m_uart->write_nb1(BIT(data, 6));
		m_uart->write_nb2(BIT(data, 5));
		m_uart->write_tsb(BIT(data, 4));
		m_uart->write_eps(BIT(data, 7));
		m_uart->write_np(BIT(data, 3));
		m_uart->write_cs(1);
	}
	else
	{
/* not emulated
    d7,d6 Not used
    d5 Secondary Unassigned, pin 18
    d4 Secondary Transmit Data, pin 14
    d3 Secondary Request-to-Send, pin 19
    d2 Break ('0'=disable transmit data; continuous RS232 'SPACE' condition)
    d1 Data-Terminal-Ready (DTR), pin 20
    d0 Request-to-Send (RTS), pin 4 */

	}
}


u8 trs80_state::sys80_f9_r()
{
/* UART Status Register - d6..d4 not emulated
    d7 Transmit buffer empty (inverted)
    d6 CTS pin
    d5 DSR pin
    d4 CD pin
    d3 Parity Error
    d2 Framing Error
    d1 Overrun
    d0 Data Available */

	u8 data = 0x70;
	m_uart->write_swe(0);
	data |= m_uart->tbmt_r() ? 0 : 0x80;
	data |= m_uart->dav_r( ) ? 0x01 : 0;
	data |= m_uart->or_r(  ) ? 0x02 : 0;
	data |= m_uart->fe_r(  ) ? 0x04 : 0;
	data |= m_uart->pe_r(  ) ? 0x08 : 0;
	m_uart->write_swe(1);

	return data;
}

u8 trs80_state::port_ff_r()
{
/* ModeSel and cassette data
    d7 cassette data from tape
    d6 modesel setting */

	return (m_cpl ? 0 : 0x40) | (m_cassette_data ? 0x80 : 0) | 0x3f;
}

void trs80_state::sys80_f8_w(u8 data)
{
/* not emulated
    d2 reset UART (XR pin)
    d1 DTR
    d0 RTS */
}

void trs80_state::sys80_fe_w(u8 data)
{
/* not emulated
    d4 select internal or external cassette player */

	m_tape_unit = BIT(data, 4) ? 2 : 1;
}

void trs80_state::port_ff_w(u8 data)
{
/* Standard output port of Model I
    d3 ModeSel bit
    d2 Relay
    d1, d0 Cassette output */

	static const double levels[4] = { 0.0, 1.0, -1.0, 0.0 };

	m_cassette->change_state(BIT(data, 2) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR );
	m_cassette->output(levels[data & 3]);
	m_cassette_data = false;

	m_cpl = BIT(data, 3);

	/* Speaker for System-80 MK II - only sounds if relay is off */
	if (!(BIT(data, 2)))
	{
		m_speaker->set_levels(4, levels);
		m_speaker->level_w(data & 3);
	}
}

/*************************************
 *
 *      Interrupt handlers.
 *
 *************************************/

INTERRUPT_GEN_MEMBER(trs80_state::rtc_interrupt)
{
/* This enables the processing of interrupts for the clock and the flashing cursor.
    The OS counts one tick for each interrupt. It is called 40 times per second. */

	m_irq |= IRQ_M1_RTC;
	m_maincpu->set_input_line(0, HOLD_LINE);

	// While we're here, let's countdown the motor timeout too.
	// Let's not... LDOS often freezes
//  if (m_timeout)
//  {
//      m_timeout--;
//      if (m_timeout == 0)
//          if (m_fdd)
//              m_fdd->mon_w(1);  // motor off
//  }
}


WRITE_LINE_MEMBER(trs80_state::intrq_w)
{
	if (state)
	{
		m_irq |= IRQ_M1_FDC;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}
	else
		m_irq &= ~IRQ_M1_FDC;
}


/*************************************
 *                                   *
 *      Memory handlers              *
 *                                   *
 *************************************/

u8 trs80_state::fdc_r(offs_t offset)
{
	if ((offset == 0) && (!BIT(m_io_config->read(), 7)))
		return 0xff;
	else
		return m_fdc->read(offset) ^ 0xff;
}

void trs80_state::fdc_w(offs_t offset, u8 data)
{
	m_fdc->write(offset, data ^ 0xff);
}

u8 trs80_state::printer_r()
{
	return m_cent_status_in->read();
}

void trs80_state::printer_w(u8 data)
{
	m_cent_data_out->write(data);
	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}

void trs80_state::cassunit_w(u8 data)
{
/* not emulated
    01 for unit 1 (default)
    02 for unit 2 */

	m_tape_unit = data;
}

u8 trs80_state::irq_status_r()
{
/* (trs80l2) Whenever an interrupt occurs, 37E0 is read to see what devices require service.
    d7 = RTC
    d6 = FDC
    d2 = Communications (not emulated)
    All interrupting devices are serviced in a single interrupt. There is a mask byte,
    which is dealt with by the DOS. We take the opportunity to reset the cpu INT line. */

	u8 result = m_irq;
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_irq = 0;
	return result;
}


void trs80_state::motor_w(u8 data)
{
	m_fdd = nullptr;

	for (u8 i = 0; i < 4; i++)
	if (BIT(data, i))
		m_fdd = m_floppy[i]->get_device();

	m_fdc->set_floppy(m_fdd);

	if (m_fdd)
	{
		m_fdd->mon_w(0);
		m_fdd->ss_w(BIT(data, 4));
		m_timeout = 200;
	}

	// switch to fm
	m_fdc->dden_w(1);
}

/*************************************
 *      Keyboard         *
 *************************************/
u8 trs80_state::keyboard_r(offs_t offset)
{
	u8 i, result = 0;

	for (i = 0; i < 8; i++)
		if (BIT(offset, i))
			result |= m_io_keyboard[i]->read();

	return result;
}


/*************************************
 *  Machine              *
 *************************************/

void trs80_state::machine_start()
{
	save_item(NAME(m_cpl));
	save_item(NAME(m_irq));
	save_item(NAME(m_mask));
	save_item(NAME(m_tape_unit));
	save_item(NAME(m_reg_load));
	save_item(NAME(m_cassette_data));
	save_item(NAME(m_old_cassette_val));
	save_item(NAME(m_cols));
	save_item(NAME(m_timeout));

	m_tape_unit = 1;
	m_reg_load = 1;

	m_cassette_data_timer = timer_alloc(FUNC(trs80_state::cassette_data_callback), this);
	m_cassette_data_timer->adjust( attotime::zero, 0, attotime::from_hz(11025) );
}

void trs80_state::machine_reset()
{
	m_cassette_data = false;
	m_cols = 0xff;
	m_cpl = 0;
	// if machine has a uart but no brg, the baud is determined by dipswitch
	if (m_io_baud)
	{
		const uint16_t s_bauds[8]={ 110, 300, 600, 1200, 2400, 4800, 9600, 19200 };
		u16 s_clock = s_bauds[m_io_baud->read()] << 4;
		m_uart_clock->set_unscaled_clock(s_clock);
	}
}


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG 1

#define CMD_TYPE_OBJECT_CODE                            0x01
#define CMD_TYPE_TRANSFER_ADDRESS                       0x02
#define CMD_TYPE_END_OF_PARTITIONED_DATA_SET_MEMBER     0x04
#define CMD_TYPE_LOAD_MODULE_HEADER                     0x05
#define CMD_TYPE_PARTITIONED_DATA_SET_HEADER            0x06
#define CMD_TYPE_PATCH_NAME_HEADER                      0x07
#define CMD_TYPE_ISAM_DIRECTORY_ENTRY                   0x08
#define CMD_TYPE_END_OF_ISAM_DIRECTORY_ENTRY            0x0a
#define CMD_TYPE_PDS_DIRECTORY_ENTRY                    0x0c
#define CMD_TYPE_END_OF_PDS_DIRECTORY_ENTRY             0x0e
#define CMD_TYPE_YANKED_LOAD_BLOCK                      0x10
#define CMD_TYPE_COPYRIGHT_BLOCK                        0x1f

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// TODO: If you get "Attempting to write outside of RAM" enough times in succession, MAME will exit unexpectedly (no error).
QUICKLOAD_LOAD_MEMBER(trs80_state::quickload_cb)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	u8 type, length;
	u8 data[0x100];
	u8 addr[2];
	void *ptr;

	while (!image.image_feof())
	{
		image.fread( &type, 1);
		image.fread( &length, 1);

		switch (type)
		{
			case CMD_TYPE_OBJECT_CODE:  // 01 - block of data
			{
				length -= 2;
				u16 block_length = length ? length : 256;
				image.fread( &addr, 2);
				u16 address = (addr[1] << 8) | addr[0];
				if (LOG) logerror("/CMD object code block: address %04x length %u\n", address, block_length);
				ptr = program.get_write_ptr(address);
				if (!ptr)
				{
					image.message("Attempting to write outside of RAM");
					return image_init_result::FAIL;
				}
				image.fread( ptr, block_length);
			}
			break;

			case CMD_TYPE_TRANSFER_ADDRESS: // 02 - go address
			{
				image.fread( &addr, 2);
				u16 address = (addr[1] << 8) | addr[0];
				if (LOG) logerror("/CMD transfer address %04x\n", address);
				m_maincpu->set_state_int(Z80_PC, address);
			}
			return image_init_result::PASS;

		case CMD_TYPE_LOAD_MODULE_HEADER: // 05 - name
			image.fread( &data, length);
			if (LOG) logerror("/CMD load module header '%s'\n", data);
			break;

		case CMD_TYPE_COPYRIGHT_BLOCK: // 1F - copyright info
			image.fread( &data, length);
			if (LOG) logerror("/CMD copyright block '%s'\n", data);
			break;

		default:
			image.fread( &data, length);
			logerror("/CMD unsupported block type %u!\n", type);
			image.message("Unsupported or invalid block type");
			return image_init_result::FAIL;
		}
	}

	return image_init_result::PASS;
}
