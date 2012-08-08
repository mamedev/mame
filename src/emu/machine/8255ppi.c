/*********************************************************************

    8255ppi.c

    Intel 8255 PPI I/O chip


    NOTE: When port is input, then data present on the ports
    outputs is 0xff

    The 8255 PPI has three basic modes:

        Mode 0: Basic Input/Output
        Mode 1: Strobed Input/Output
        Mode 2: Strobed Bi-directional Bus

    Control Word:

        bit 7   - Mode set flag (1=active)
        bit 6-5 - Group A Mode selection
                    00 - Mode 0
                    01 - Mode 1
                    1x - Mode 2
        bit 4   - Port A direction (1=input 0=output)
        bit 3   - Port C upper direction (1=input 0=output)
        bit 2   - Group B Mode selection
                    0 - Mode 0
                    1 - Mode 1
        bit 1   - Port B direction (1=input 0=output)
        bit 0   - Port C lower direction (1=input 0=output)

        Port A and Port C upper are in group A, and Port B and Port C lower
        are in group B


    Mode 0: Basic Input/Output
        In Mode 0, each of the ports (A, B and C) operate as independent
        ports for whom direction can be set independently.

        Port C Usage In Mode 0:

            bits 7-4    Input/Output A (direction specified by ctrl bit 3)
            bits 3-0    Input/Output B (direction specified by ctrl bit 0)

    Mode 1: Strobed Input/Output
        In Mode 1, Port A and Port B use their resepective parts of Port C to
        either generate or accept handshaking signals.  The STB (strobe) input
        "loads" data into the port, and the IBF (input buffer full) output is
        then asserted, and the INTR (interrupt request) output is triggered if
        interrupts are enabled.  Bits 7-6 of Port C remain usable as
        conventional IO.

        Group A Port C Usage In Mode 1:

            bits 7-6    Input/Output (direction specified by ctrl bit 3)
            bit 5       IBFa (input buffer full A) output
            bit 4       !STBa (strobe A) input
            bit 3       INTRa (interrupt request A) output

        Group B Port C Usage In Mode 1:

            bit 2       !STBb (strobe B) input
            bit 1       IBFb (input buffer full B) output
            bit 0       INTRb (interrupt request B) output


    Mode 2: Strobed Bi-directional Bus
        Mode 2 is used to implement a two way handshaking bus.

        When data is written to port A, the OBF (output buffer full) output
        will be asserted by the PPI.  However, port A will not be asserted
        unless the ACK input is asserted, otherwise port A will be high
        impedence.

        The STB input and IBF output behaves similar to how it does under mode
        1.  Bits 2-0 of Port C remain usable as conventional IO.

        Port C Usage In Mode 2:

            bit 7       !OBFa (output buffer full A) output
            bit 6       !ACKa (acknowledge A) input
            bit 5       IBFa (interrupt buffer full A) output
            bit 4       !STBa (strobe A) input
            bit 3       INTRa (interrupt A) output
            bit 2-0     Reserved by Group B

    KT 10/01/2000 - Added bit set/reset feature for control port
                  - Added more accurate port i/o data handling
                  - Added output reset when control mode is programmed

*********************************************************************/

#include "emu.h"
#include "8255ppi.h"
#include "devhelpr.h"


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

// device type definition
const device_type PPI8255 = &device_creator<ppi8255_device>;

//-------------------------------------------------
//  ppi8255_device - constructor
//-------------------------------------------------

ppi8255_device::ppi8255_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, PPI8255, "Intel PPI8255", tag, owner, clock),
      m_control(0)
{

}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ppi8255_device::device_config_complete()
{
	// inherit a copy of the static data
	const ppi8255_interface *intf = reinterpret_cast<const ppi8255_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<ppi8255_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_port_a_read, 0, sizeof(m_port_a_read));
    	memset(&m_port_b_read, 0, sizeof(m_port_b_read));
    	memset(&m_port_c_read, 0, sizeof(m_port_c_read));
    	memset(&m_port_a_write, 0, sizeof(m_port_a_write));
    	memset(&m_port_b_write, 0, sizeof(m_port_b_write));
    	memset(&m_port_c_write, 0, sizeof(m_port_c_write));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ppi8255_device::device_start()
{
	m_port_read[0].resolve(m_port_a_read, *this);
	m_port_read[1].resolve(m_port_b_read, *this);
	m_port_read[2].resolve(m_port_c_read, *this);

	m_port_write[0].resolve(m_port_a_write, *this);
	m_port_write[1].resolve(m_port_b_write, *this);
	m_port_write[2].resolve(m_port_c_write, *this);

	/* register for state saving */
	save_item(NAME(m_group_a_mode));
	save_item(NAME(m_group_b_mode));
	save_item(NAME(m_port_a_dir));
	save_item(NAME(m_port_b_dir));
	save_item(NAME(m_port_ch_dir));
	save_item(NAME(m_port_cl_dir));
	save_item(NAME(m_obf_a));
	save_item(NAME(m_obf_b));
	save_item(NAME(m_ibf_a));
	save_item(NAME(m_ibf_b));
	save_item(NAME(m_inte_a));
	save_item(NAME(m_inte_b));
	save_item(NAME(m_inte_1));
	save_item(NAME(m_inte_2));
	save_item(NAME(m_in_mask));
	save_item(NAME(m_out_mask));
	save_item(NAME(m_read));
	save_item(NAME(m_latch));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void ppi8255_device::device_reset()
{
	m_group_a_mode = 0;
	m_group_b_mode = 0;
	m_port_a_dir = 0;
	m_port_b_dir = 0;
	m_port_ch_dir = 0;
	m_port_cl_dir = 0;
	m_obf_a = m_ibf_a = 0;
	m_obf_b = m_ibf_b = 0;
	m_inte_a = m_inte_b = m_inte_1 = m_inte_2 = 0;

	for (int i = 0; i < 3; i++)
	{
		m_in_mask[i] = m_out_mask[i] = m_read[i] = m_latch[i] = m_output[i] = 0;
	}

	set_mode(0x9b, 0);   /* Mode 0, all ports set to input */
}


void ppi8255_device::get_handshake_signals(bool is_read, UINT8 &result)
{
	UINT8 handshake = 0x00;
	UINT8 mask = 0x00;

	/* group A */
	if (m_group_a_mode == 1)
	{
		if (m_port_a_dir)
		{
			handshake |= m_ibf_a ? 0x20 : 0x00;
			handshake |= (m_ibf_a && m_inte_a) ? 0x08 : 0x00;
			mask |= 0x28;
		}
		else
		{
			handshake |= m_obf_a ? 0x00 : 0x80;
			handshake |= (m_obf_a && m_inte_a) ? 0x08 : 0x00;
			mask |= 0x88;
		}
	}
	else if (m_group_a_mode == 2)
	{
		handshake |= m_obf_a ? 0x00 : 0x80;
		handshake |= m_ibf_a ? 0x20 : 0x00;
		handshake |= ((m_obf_a && m_inte_1) || (m_ibf_a && m_inte_2)) ? 0x08 : 0x00;
		mask |= 0xA8;
	}

	/* group B */
	if (m_group_b_mode == 1)
	{
		if (m_port_b_dir)
		{
			handshake |= m_ibf_b ? 0x02 : 0x00;
			handshake |= (m_ibf_b && m_inte_b) ? 0x01 : 0x00;
			mask |= 0x03;
		}
		else
		{
			handshake |= m_obf_b ? 0x00 : 0x02;
			handshake |= (m_obf_b && m_inte_b) ? 0x01 : 0x00;
			mask |= 0x03;
		}
	}

	result &= ~mask;
	result |= handshake & mask;
}



void ppi8255_device::input(int port, UINT8 data)
{
	int changed = 0;

	m_read[port] = data;

	/* port C is special */
	if (port == 2)
	{
		if (((m_group_a_mode == 1) && (m_port_a_dir == 0)) || (m_group_a_mode == 2))
		{
			/* is !ACKA asserted? */
			if (m_obf_a && !(data & 0x40))
			{
				m_obf_a = 0;
				changed = 1;
			}
		}

		if (((m_group_a_mode == 1) && (m_port_a_dir == 1)) || (m_group_a_mode == 2))
		{
			/* is !STBA asserted? */
			if (!m_ibf_a && !(data & 0x10))
			{
				m_ibf_a = 1;
				changed = 1;
			}
		}

		if ((m_group_b_mode == 1) && (m_port_b_dir == 0))
		{
			/* is !ACKB asserted? */
			if (m_obf_b && !(data & 0x04))
			{
				m_obf_b = 0;
				changed = 1;
			}
		}

		if ((m_group_b_mode == 1) && (m_port_b_dir == 1))
		{
			/* is !STBB asserted? */
			if (!m_ibf_b && !(data & 0x04))
			{
				m_ibf_b = 1;
				changed = 1;
			}
		}

		if (changed)
		{
			write_port(2);
		}
	}
}



UINT8 ppi8255_device::read_port(int port)
{
	UINT8 result = 0x00;

	if (m_in_mask[port])
	{
		input(port, m_port_read[port](0));
		result |= m_read[port] & m_in_mask[port];
	}
	result |= m_latch[port] & m_out_mask[port];

	switch (port)
	{
	case 0:
		/* clear input buffer full flag */
		m_ibf_a = 0;
		break;

	case 1:
		/* clear input buffer full flag */
		m_ibf_b = 0;
		break;

	case 2:
		/* read special port 2 signals */
		get_handshake_signals(true, result);
		break;
	}

	return result;
}


READ8_MEMBER( ppi8255_device::read )
{
	UINT8 result = 0;

	offset %= 4;

	switch(offset)
	{
		case 0: /* Port A read */
		case 1: /* Port B read */
		case 2: /* Port C read */
			result = read_port(offset);
			break;

		case 3: /* Control word */
			result = m_control;
			break;
	}

	return result;
}



void ppi8255_device::write_port(int port)
{
	UINT8 write_data = m_latch[port] & m_out_mask[port];
	write_data |= 0xFF & ~m_out_mask[port];

	/* write out special port 2 signals */
	if (port == 2)
	{
		get_handshake_signals(false, write_data);
	}

	m_output[port] = write_data;
	m_port_write[port](0, write_data);
}



WRITE8_MEMBER( ppi8255_device::write )
{
	offset %= 4;

	switch( offset )
	{
		case 0: /* Port A write */
		case 1: /* Port B write */
		case 2: /* Port C write */
			m_latch[offset] = data;
			write_port(offset);

			switch(offset)
			{
				case 0:
					if (!m_port_a_dir && (m_group_a_mode != 0))
					{
						m_obf_a = 1;
						write_port(2);
					}
					break;

				case 1:
					if (!m_port_b_dir && (m_group_b_mode != 0))
					{
						m_obf_b = 1;
						write_port(2);
					}
					break;
			}
			break;

		case 3: /* Control word */
			if (data & 0x80)
			{
				set_mode(data & 0x7f, 1);
			}
			else
			{
				/* bit set/reset */
				int bit = (data >> 1) & 0x07;

				if (data & 1)
				{
					m_latch[2] |= (1 << bit);	/* set bit */
				}
				else
				{
					m_latch[2] &= ~(1 << bit);	/* reset bit */
				}

				if (m_group_b_mode == 1)
				{
					if (bit == 2)
					{
						m_inte_b = data & 1;
					}
				}

				if (m_group_a_mode == 1)
				{
					if (bit == 4 && m_port_a_dir)
					{
						m_inte_a = data & 1;
					}
					if (bit == 6 && !m_port_a_dir)
					{
						m_inte_a = data & 1;
					}
				}

				if (m_group_a_mode == 2)
				{
					if (bit == 4)
					{
						m_inte_2 = data & 1;
					}
					if (bit == 6)
					{
						m_inte_1 = data & 1;
					}
				}

				write_port(2);
			}
			break;
	}
}


void ppi8255_device::set_mode(int data, int call_handlers)
{
	/* parse out mode */
	m_group_a_mode = (data >> 5) & 3;
	m_group_b_mode = (data >> 2) & 1;
	m_port_a_dir = (data >> 4) & 1;
	m_port_b_dir = (data >> 1) & 1;
	m_port_ch_dir = (data >> 3) & 1;
	m_port_cl_dir = (data >> 0) & 1;

	/* normalize group_a_mode */
	if (m_group_a_mode == 3)
	{
		m_group_a_mode = 2;
	}

	/* Port A direction */
	if (m_group_a_mode == 2)
	{
		m_in_mask[0] = 0xFF;
		m_out_mask[0] = 0xFF;	/* bidirectional */
	}
	else
	{
		if (m_port_a_dir)
		{
			m_in_mask[0] = 0xFF;
			m_out_mask[0] = 0x00;	/* input */
		}
		else
		{
			m_in_mask[0] = 0x00;
			m_out_mask[0] = 0xFF;	/* output */
		}
	}

	/* Port B direction */
	if (m_port_b_dir)
	{
		m_in_mask[1] = 0xFF;
		m_out_mask[1] = 0x00;	/* input */
	}
	else
	{
		m_in_mask[1] = 0x00;
		m_out_mask[1] = 0xFF;	/* output */
	}

	/* Port C upper direction */
	if (m_port_ch_dir)
	{
		m_in_mask[2] = 0xF0;
		m_out_mask[2] = 0x00;	/* input */
	}
	else
	{
		m_in_mask[2] = 0x00;
		m_out_mask[2] = 0xF0;	/* output */
	}

	/* Port C lower direction */
	if (m_port_cl_dir)
	{
		m_in_mask[2] |= 0x0F;	/* input */
	}
	else
	{
		m_out_mask[2] |= 0x0F;	/* output */
	}

	/* now depending on the group modes, certain Port C lines may be replaced
     * with varying control signals */
	switch(m_group_a_mode)
	{
		case 0:	/* Group A mode 0 */
			/* no changes */
			break;

		case 1:	/* Group A mode 1 */
			/* bits 5-3 are reserved by Group A mode 1 */
			m_in_mask[2] &= ~0x38;
			m_out_mask[2] &= ~0x38;
			break;

		case 2: /* Group A mode 2 */
			/* bits 7-3 are reserved by Group A mode 2 */
			m_in_mask[2] &= ~0xF8;
			m_out_mask[2] &= ~0xF8;
			break;
	}

	switch(m_group_b_mode)
	{
		case 0:	/* Group B mode 0 */
			/* no changes */
			break;

		case 1:	/* Group B mode 1 */
			/* bits 2-0 are reserved by Group B mode 1 */
			m_in_mask[2] &= ~0x07;
			m_out_mask[2] &= ~0x07;
			break;
	}

	/* KT: 25-Dec-99 - 8255 resets latches when mode set */
	m_latch[0] = m_latch[1] = m_latch[2] = 0;

	if (call_handlers)
	{
		for (int i = 0; i < 3; i++)
		{
			write_port(i);
		}
	}

	/* reset flip-flops */
	m_obf_a = m_ibf_a = 0;
	m_obf_b = m_ibf_b = 0;
	m_inte_a = m_inte_b = m_inte_1 = m_inte_2 = 0;

	/* store control word */
	m_control = data;
}
