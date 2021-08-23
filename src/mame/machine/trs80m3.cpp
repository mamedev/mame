// license:BSD-3-Clause
// copyright-holders:Robbbert
//**************************************************************************************************************************

#include "emu.h"
#include "includes/trs80m3.h"


#define IRQ_M4_RTC      0x04    /* RTC on Model 4 */
#define CASS_RISE       0x01    /* high speed cass on Model III/4) */
#define CASS_FALL       0x02    /* high speed cass on Model III/4) */
#define MODEL4_MASTER_CLOCK 20275200


TIMER_CALLBACK_MEMBER(trs80m3_state::cassette_data_callback)
{
/* This does all baud rates. 250 baud (trs80), and 500 baud (all others) set bit 7 of "cassette_data".
    1500 baud (trs80m3, trs80m4) is interrupt-driven and uses bit 0 of "cassette_data" */

	double new_val = (m_cassette->input());

	/* Check for HI-LO transition */
	if ( m_old_cassette_val > -0.2 && new_val < -0.2 )
	{
		m_cassette_data |= 0x80;        /* 500 baud */
		if (m_mask & CASS_FALL) /* see if 1500 baud */
		{
			m_cassette_data = 0;
			m_irq |= CASS_FALL;
			m_maincpu->set_input_line(0, HOLD_LINE);
		}
	}
	else
	if ( m_old_cassette_val < -0.2 && new_val > -0.2 )
	{
		if (m_mask & CASS_RISE) /* 1500 baud */
		{
			m_cassette_data = 1;
			m_irq |= CASS_RISE;
			m_maincpu->set_input_line(0, HOLD_LINE);
		}
	}

	m_old_cassette_val = new_val;
}


/*************************************
 *
 *              Port handlers.
 *
 *************************************/


uint8_t trs80m3_state::port_e0_r()
{
/* Indicates which devices are interrupting - d6..d3 not emulated.
    Whenever an interrupt occurs, this port is immediately read
    to find out which device requires service. Lowest-numbered
    bit takes precedence. We take this opportunity to clear the
    cpu INT line.

    d6 RS232 Error (Any of {FE, PE, OR} errors has occurred)
    d5 RS232 Rcv (DAV indicates a char ready to be picked up from uart)
    d4 RS232 Xmit (TBMT indicates ready to accept another char from cpu)
    d3 I/O Bus
    d2 RTC
    d1 Cass 1500 baud Falling
    d0 Cass 1500 baud Rising */

	m_maincpu->set_input_line(0, CLEAR_LINE);
	return ~(m_mask & m_irq);
}

uint8_t trs80m3_state::port_e4_r()
{
/* Indicates which devices are interrupting - d6..d5 not emulated.
    Whenever an NMI occurs, this port is immediately read
    to find out which device requires service. Lowest-numbered
    bit takes precedence. We take this opportunity to clear the
    cpu NMI line.

    d7 status of FDC INTREQ (0=true)
    d6 status of Motor Timeout (0=true)
    d5 status of Reset signal (0=true - this will reboot the computer) */

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	u8 data = m_nmi_data;
	m_nmi_data = 0;
	return ~(m_nmi_mask & data);
}

uint8_t trs80m3_state::port_e8_r()
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

uint8_t trs80m3_state::port_ea_r()
{
/* UART Status Register
    d7 Data Received ('1'=condition true)
    d6 Transmitter Holding Register empty ('1'=condition true)
    d5 Overrun Error ('1'=condition true)
    d4 Framing Error ('1'=condition true)
    d3 Parity Error ('1'=condition true)
    d2..d0 Not used */

	uint8_t data=7;
	m_uart->write_swe(0);
	data |= m_uart->tbmt_r() ? 0x40 : 0;
	data |= m_uart->dav_r( ) ? 0x80 : 0;
	data |= m_uart->or_r(  ) ? 0x20 : 0;
	data |= m_uart->fe_r(  ) ? 0x10 : 0;
	data |= m_uart->pe_r(  ) ? 0x08 : 0;
	m_uart->write_swe(1);

	return data;
}

uint8_t trs80m3_state::port_ec_r()
{
/* Reset the RTC interrupt */
	m_irq &= ~IRQ_M4_RTC;
	return 0;
}

uint8_t trs80m3_state::port_ff_r()
{
/* Return of cassette data stream from tape
    d7 Low-speed data
    d6..d1 info from write of port EC
    d0 High-speed data */

	m_irq &= 0xfc;  /* clear cassette interrupts */

	return m_port_ec | m_cassette_data;
}

uint8_t trs80m3_state::cp500_port_f4_r()
{
	/* The A11 flipflop is used for enabling access to
	       the system monitor code at the EPROM address range 3800-3fff */
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *bootrom = memregion("bootrom")->base();

	m_a11_flipflop ^= 1; //toggle the flip-flop at every read at io addresses 0xf4-f7

	for (u8 block=0; block<8; block++)
		memcpy(&rom[block * 0x800], &bootrom[(block | m_a11_flipflop) * 0x800], 0x800);

	return 0x00; //really?!
}


void trs80m3_state::port_84_w(uint8_t data) // Model 4 & 4P only
{
/* Memory banking control, video mode control
    d7 Video Page Control
    d6 Despage (see p129 of service manual)
    d5 Enable page mapping (1=enabled)
    d4 Srcpage (see p129 of service manual)
    d3 Invert Video
    d2 80/64 width
    d1 Select bit 1
    d0 Select bit 0 */

	if (!(m_model4 & 6)) // Model 3 leave now
		return;

	m_mode = (m_mode & 0x73) | (data & 0x8c);

	m_model4 &= 0xce;
	m_model4 |= (data & 3) << 4;

	if (BIT(m_model4, 1)) // Model 4
	{
		if (m_mainram->size() >= (64 * 1024))
		{
			m_m4_bank->set_bank(data & 0x03);
			m_32kbanks[0]->set_entry((data >> 4) & 0x07);
			m_32kbanks[1]->set_entry((data >> 4) & 0x07);
			m_16kbank->set_entry((data >> 4) & 0x07);
		}
		m_vidbank->set_entry(BIT(data, 7));
		return;
	}

	if (BIT(m_model4, 2)) // Model 4P
	{
		m_32kbanks[0]->set_entry((data >> 4) & 0x07);
		m_32kbanks[1]->set_entry((data >> 4) & 0x07);
		m_16kbank->set_entry((data >> 4) & 0x07);
		m_vidbank->set_entry(BIT(data, 7));

		switch (data & 3)
		{
			case 0: /* normal operation */
				if (BIT(m_model4, 3))
					m_m4p_bank->set_bank(0);
				else
					m_m4p_bank->set_bank(4);
				break;

			case 1: /* write-only ram backs up the rom */
				if (BIT(m_model4, 3))
					m_m4p_bank->set_bank(1);
				else
					m_m4p_bank->set_bank(5);
				break;

			case 2: /* keyboard and video are moved to high memory, and the rest is ram */
				m_m4p_bank->set_bank(2);
				m_model4 |= 1;
				break;

			case 3: /* 64k of ram */
				m_m4p_bank->set_bank(3);
				break;
		}
	}
}

void trs80m3_state::port_90_w(uint8_t data)
{
	m_speaker->level_w(!(BIT(data, 0)));
}

void trs80m3_state::port_9c_w(uint8_t data)     /* model 4P only - swaps the ROM with read-only RAM */
{
	/* Meaning of model4 variable:
	    d5..d4 memory mode (as described in section above)
	    d3 rom switch (1=enabled) only effective in mode0 and 1
	    d2 this is a Model 4P
	    d1 this is a Model 4
	    d0 Video banking exists yes/no (1=not banked) */

	if (!(BIT(m_model4, 2))) // If not Model 4P, leave now
		return;

	m_model4 &= 0xf7;
	m_model4 |= (data << 3);

	switch (m_model4 & 0x38)
	{
		case 0x00:
			m_m4p_bank->set_bank(4);
			break;

		case 0x08:
			m_m4p_bank->set_bank(0);
			break;

		case 0x10:
			m_m4p_bank->set_bank(5);
			break;

		case 0x18:
			m_m4p_bank->set_bank(1);
			break;

		default:
			break;
	}
}

void trs80m3_state::port_e0_w(uint8_t data)
{
/* Interrupt settings - which devices are allowed to interrupt - bits align with read of E0
    d6 Enable Rec Err
    d5 Enable Rec Data
    d4 Enable Xmit Emp
    d3 Enable I/O int
    d2 Enable RT int
    d1 C fall Int
    d0 C Rise Int */

	m_mask = data;
}

void trs80m3_state::port_e4_w(uint8_t data)
{
/* Disk to NMI interface
    d7 1=enable disk INTRQ to generate NMI
    d6 1=enable disk Motor Timeout to generate NMI */

	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	m_nmi_mask = data;
}

void trs80m3_state::port_e8_w(uint8_t data)
{
/* d1 when '1' enables control register load (see below) */

	m_reg_load = BIT(data, 1);
}

void trs80m3_state::port_ea_w(uint8_t data)
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

void trs80m3_state::port_ec_w(uint8_t data)
{
/* Hardware settings - d5..d4 not emulated
    d6 CPU fast (1=4MHz, 0=2MHz)
    d5 1=Enable Video Wait
    d4 1=Enable External I/O bus
    d3 1=Enable Alternate Character Set
    d2 Mode Select (0=64 chars, 1=32chars)
    d1 Cassette Motor (1=On) */

	m_maincpu->set_unscaled_clock(data & 0x40 ? MODEL4_MASTER_CLOCK/5 : MODEL4_MASTER_CLOCK/10);

	m_mode = (m_mode & 0xde) | (BIT(data, 2) ? 1 : 0) | (BIT(data, 3) ? 0x20 : 0);

	if (!BIT(m_model4, 2))     // Model 4P has no cassette hardware
		m_cassette->change_state(( data & 2 ) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR );

	m_port_ec = data & 0x7e;
}

/* Selection of drive and parameters - d6..d5 not emulated.
 A write also causes the selected drive motor to turn on for about 3 seconds.
 When the motor turns off, the drive is deselected.
    d7 1=MFM, 0=FM
    d6 1=Wait
    d5 1=Write Precompensation enabled
    d4 0=Side 0, 1=Side 1
    d3 1=select drive 3
    d2 1=select drive 2
    d1 1=select drive 1
    d0 1=select drive 0 */
void trs80m3_state::port_f4_w(uint8_t data)
{
	if (BIT(data, 6))
	{
		if (m_drq_off && m_intrq_off)
		{
			m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, ASSERT_LINE);
			m_wait = true;
		}
	}
	else
	{
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
		m_wait = false;
	}

	m_fdd = nullptr;

	if (BIT(data, 0)) m_fdd = m_floppy[0]->get_device();
	if (BIT(data, 1)) m_fdd = m_floppy[1]->get_device();

	m_fdc->set_floppy(m_fdd);

	if (m_fdd)
	{
		m_fdd->mon_w(0);
		m_fdd->ss_w(BIT(data, 4));
		m_timeout = 1600;
	}

	m_fdc->dden_w(!BIT(data, 7));
}

void trs80m3_state::port_ff_w(uint8_t data)
{
/* Cassette port
    d1, d0 Cassette output */

	static const double levels[4] = { 0.0, 1.0, -1.0, 0.0 };

	// Model 4P has no cassette hardware, and only one bit for sound
	if (BIT(m_model4, 2))
	{
		if (!(BIT(m_port_ec, 1)))
			m_speaker->level_w(BIT(data, 0));
	}
	else
	// Others have the same old unofficial sound via the cassette port
	{
		m_cassette->output(levels[data & 3]);
		m_cassette_data &= ~0x80;
		if (!(BIT(m_port_ec, 1)))
		{
			m_speaker->set_levels(4, levels);
			m_speaker->level_w(data & 3);
		}
	}
}


/*************************************
 *
 *      Interrupt handlers.
 *
 *************************************/

INTERRUPT_GEN_MEMBER(trs80m3_state::rtc_interrupt)
{
/* This enables the processing of interrupts for the clock and the flashing cursor.
    The OS counts one tick for each interrupt. It is called 30 times per second. */

	if (m_mask & IRQ_M4_RTC)
	{
		m_irq |= IRQ_M4_RTC;
		m_maincpu->set_input_line(0, HOLD_LINE);
	}

	// While we're here, let's countdown the motor timeout too.
	if (m_timeout)
	{
		m_timeout--;
		if (m_timeout == 0)
			if (m_fdd)
				m_fdd->mon_w(1);  // motor off
	}
	// Also, if cpu is in wait, unlock it and trigger NMI
	// Don't, it breaks disk loading
//  if (m_wait)
//  {
//      m_wait = false;
//      m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
//      if (BIT(m_nmi_mask, 6))
//      {
//          m_nmi_data |= 0x40;
//          m_maincpu->set_input_line(INPUT_LINE_NMI, HOLD_LINE);
//      }
//  }
}

// The floppy sector has been read. Enable CPU and NMI.
WRITE_LINE_MEMBER(trs80m3_state::intrq_w)
{
	m_intrq_off = state ? false : true;
	if (state)
	{
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
		m_wait = false;
		if (BIT(m_nmi_mask, 7))
		{
			m_nmi_data |= 0x80;
			//m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			m_maincpu->set_input_line(INPUT_LINE_NMI, HOLD_LINE);
		}
	}
}

// The next byte from floppy is available. Enable CPU so it can get the byte.
WRITE_LINE_MEMBER(trs80m3_state::drq_w)
{
	m_drq_off = state ? false : true;
	if (state)
	{
		m_maincpu->set_input_line(Z80_INPUT_LINE_WAIT, CLEAR_LINE);
		m_wait = false;
	}
}


/*************************************
 *                                   *
 *      Memory handlers              *
 *                                   *
 *************************************/

uint8_t trs80m3_state::wd179x_r()
{
	uint8_t data = 0xff;
	if (BIT(m_io_config->read(), 7))
		data = m_fdc->status_r();

	return data;
}

uint8_t trs80m3_state::printer_r()
{
	return m_cent_status_in->read();
}

void trs80m3_state::printer_w(uint8_t data)
{
	m_cent_data_out->write(data);
	m_centronics->write_strobe(0);
	m_centronics->write_strobe(1);
}

/*************************************
 *      Keyboard                     *
 *************************************/
uint8_t trs80m3_state::keyboard_r(offs_t offset)
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

void trs80m3_state::machine_start()
{
	save_item(NAME(m_model4));
	save_item(NAME(m_mode));
	save_item(NAME(m_irq));
	save_item(NAME(m_mask));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_port_ec));
	save_item(NAME(m_reg_load));
	save_item(NAME(m_nmi_data));
	save_item(NAME(m_cassette_data));
	save_item(NAME(m_old_cassette_val));
	save_item(NAME(m_start_address));
	save_item(NAME(m_crtc_reg));
	save_item(NAME(m_size_store));
	save_item(NAME(m_a11_flipflop));
	save_item(NAME(m_timeout));
	save_item(NAME(m_wait));
	save_item(NAME(m_drq_off));
	save_item(NAME(m_intrq_off));

	m_mode = 0;
	m_reg_load = 1;
	m_nmi_data = 0;
	m_timeout = 1;
	m_wait = 0;
	m_start_address = 0;
	m_old_cassette_val = 0;

	if (!BIT(m_model4, 2))     // Model 4P has no cassette hardware
	{
		m_cassette_data_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(trs80m3_state::cassette_data_callback),this));
		m_cassette_data_timer->adjust( attotime::zero, 0, attotime::from_hz(11025) );
	}

	if (!(m_model4 & 6))   // Model 3 leave now
		return;

	if (m_mainram->size() < (64 * 1024))
	{
		if (BIT(m_model4, 1))     // Model 4
		{
			m_m4_bank->set_stride(0x00000);
			m_m4_bank->space(0).unmap_readwrite(0x08000, 0x0ffff);
		}
		else
		if (BIT(m_model4, 2))     // Model 4P
		{
			m_m4p_bank->set_stride(0x00000);
			m_m4p_bank->space(0).unmap_readwrite(0x08000, 0x0ffff);
		}
		m_16kbank->configure_entries(0, 8, m_mainram->pointer() + 0x00000, 0x00000);
	}
	else
	if (m_mainram->size() < (128 * 1024))
	{
		m_32kbanks[0]->configure_entries(0, 8, m_mainram->pointer() + 0x00000, 0x00000);
		m_32kbanks[1]->configure_entries(0, 8, m_mainram->pointer() + 0x08000, 0x00000);
		m_16kbank->configure_entries(0, 8, m_mainram->pointer() + 0x04000, 0x00000);
	}
	else
	{
		m_32kbanks[0]->configure_entries(0, 4, m_mainram->pointer() + 0x00000, 0x00000);
		m_32kbanks[0]->configure_entries(4, 2, m_mainram->pointer() + 0x00000, 0x00000);
		m_32kbanks[0]->configure_entries(6, 2, m_mainram->pointer() + 0x10000, 0x08000);

		m_32kbanks[1]->configure_entries(0, 2, m_mainram->pointer() + 0x08000, 0x00000);
		m_32kbanks[1]->configure_entries(2, 2, m_mainram->pointer() + 0x10000, 0x08000);
		m_32kbanks[1]->configure_entries(4, 4, m_mainram->pointer() + 0x08000, 0x00000);

		m_16kbank->configure_entries(0, 4, m_mainram->pointer() + 0x04000, 0x00000);
		m_16kbank->configure_entries(4, 2, m_mainram->pointer() + 0x04000, 0x00000);
		m_16kbank->configure_entries(6, 2, m_mainram->pointer() + 0x14000, 0x08000);
	}
	m_vidbank->configure_entries(0, 2, &m_p_videoram[0], 0x0400);
}

void trs80m3_state::machine_reset()
{
	m_a11_flipflop = 0; // for cp500
	m_cassette_data = 0;
	m_size_store = 0xff;
	m_drq_off = true;
	m_intrq_off = true;

	if (m_model4 & 4)
		port_9c_w(1);    // 4P - enable rom
	if (m_model4 & 6)
		port_84_w(0);    // 4 & 4P - switch in devices

	m_fdd = nullptr;
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

QUICKLOAD_LOAD_MEMBER(trs80m3_state::quickload_cb)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	uint8_t type, length;
	uint8_t data[0x100];
	uint8_t addr[2];
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
