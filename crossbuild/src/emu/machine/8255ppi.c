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

#include "driver.h"
#include "8255ppi.h"
#include "memconv.h"


static int num;

typedef struct
{
	read8_handler port_read[3];
	write8_handler port_write[3];

	/* mode flags */
	UINT8 groupA_mode;
	UINT8 groupB_mode;
	UINT8 portA_dir;
	UINT8 portB_dir;
	UINT8 portCH_dir;
	UINT8 portCL_dir;

	/* handshake signals (1=asserted; 0=non-asserted) */
	UINT8 obf_a;
	UINT8 obf_b;
	UINT8 ibf_a;
	UINT8 ibf_b;
	UINT8 inte_a;
	UINT8 inte_b;

	UINT8 in_mask[3];		/* input mask */
	UINT8 out_mask[3];	/* output mask */
	UINT8 read[3];		/* data read from ports */
	UINT8 latch[3];		/* data written to ports */
	UINT8 output[3];	/* actual output data */
} ppi8255;

static ppi8255 chips[MAX_8255];

static void set_mode(int which, int data, int call_handlers);
static void ppi8255_write_port(ppi8255 *chip, int port);


void ppi8255_init( const ppi8255_interface *intfce )
{
	int i;

	num = intfce->num;

	for (i = 0; i < num; i++)
	{
		ppi8255 *chip = &chips[i];

		memset(chip, 0, sizeof(*chip));

		chip->port_read[0] = intfce->portAread[i];
		chip->port_read[1] = intfce->portBread[i];
		chip->port_read[2] = intfce->portCread[i];
		chip->port_write[0] = intfce->portAwrite[i];
		chip->port_write[1] = intfce->portBwrite[i];
		chip->port_write[2] = intfce->portCwrite[i];

		set_mode(i, 0x1b, 0);	/* Mode 0, all ports set to input */

		state_save_register_item("ppi8255", i, chip->groupA_mode);
		state_save_register_item("ppi8255", i, chip->groupB_mode);
		state_save_register_item("ppi8255", i, chip->portA_dir);
		state_save_register_item("ppi8255", i, chip->portB_dir);
		state_save_register_item("ppi8255", i, chip->portCH_dir);
		state_save_register_item("ppi8255", i, chip->portCL_dir);
		state_save_register_item("ppi8255", i, chip->obf_a);
		state_save_register_item("ppi8255", i, chip->obf_b);
		state_save_register_item("ppi8255", i, chip->ibf_a);
		state_save_register_item("ppi8255", i, chip->ibf_b);
		state_save_register_item("ppi8255", i, chip->inte_a);
		state_save_register_item("ppi8255", i, chip->inte_b);
		state_save_register_item_array("ppi8255", i, chip->in_mask);
		state_save_register_item_array("ppi8255", i, chip->out_mask);
		state_save_register_item_array("ppi8255", i, chip->read);
		state_save_register_item_array("ppi8255", i, chip->latch);
	}
}



static void ppi8255_get_handshake_signals(ppi8255 *chip, int is_read, UINT8 *result)
{
	UINT8 handshake = 0x00;
	UINT8 mask = 0x00;

	/* group A */
	if (chip->groupA_mode == 1)
	{
		if (chip->portA_dir)
		{
			handshake |= chip->ibf_a ? 0x20 : 0x00;
			handshake |= (chip->ibf_a && chip->inte_a) ? 0x08 : 0x00;
			mask |= 0x28;
		}
		else
		{
			handshake |= chip->obf_a ? 0x00 : 0x80;
			handshake |= (chip->obf_a && chip->inte_a) ? 0x08 : 0x00;
			mask |= 0x88;
		}
	}
	else if (chip->groupA_mode == 2)
  	{
		handshake |= chip->inte_a ? 0x08 : 0x00;
		handshake |= chip->obf_a ? 0x00 : 0x80;
		handshake |= chip->ibf_a ? 0x20 : 0x00;
		mask |= 0xA8;
	}

	/* group B */
	if (chip->groupB_mode == 1)
  	{
		if (chip->portA_dir)
		{
			handshake |= chip->ibf_b ? 0x02 : 0x00;
			handshake |= (chip->ibf_b && chip->inte_b) ? 0x01 : 0x00;
			mask |= 0x03;
  		}
		else
  		{
			handshake |= chip->obf_b ? 0x00 : 0x02;
			handshake |= (chip->obf_b && chip->inte_b) ? 0x01 : 0x00;
			mask |= 0x03;
		}
  	}

	*result &= ~mask;
	*result |= handshake & mask;
}



static void ppi8255_input(ppi8255 *chip, int port, UINT8 data)
{
	int changed = 0;

	chip->read[port] = data;

	/* port C is special */
	if (port == 2)
	{
		if (((chip->groupA_mode == 1) && (chip->portA_dir == 0)) || (chip->groupA_mode == 2))
		{
			/* is !ACKA asserted? */
			if (chip->obf_a && !(data & 0x40))
			{
				chip->obf_a = 0;
				changed = 1;
			}
		}

		if ((chip->groupB_mode == 1) && (chip->portB_dir == 0))
		{
			/* is !ACKB asserted? */
			if (chip->obf_b && !(data & 0x04))
			{
				chip->obf_b = 0;
				changed = 1;
			}
		}

		if (changed)
			ppi8255_write_port(chip, 2);
	}
}



static UINT8 ppi8255_read_port(ppi8255 *chip, int port)
{
	UINT8 result = 0x00;

	if (chip->in_mask[port])
	{
		if (chip->port_read[port])
			ppi8255_input(chip, port, chip->port_read[port](0));

		result |= chip->read[port] & chip->in_mask[port];
	}
	result |= chip->latch[port] & chip->out_mask[port];

	/* read special port 2 signals */
	if (port == 2)
		ppi8255_get_handshake_signals(chip, 1, &result);

	return result;
}



UINT8 ppi8255_r(int which, offs_t offset)
{
	ppi8255 *chip = &chips[which];
	UINT8 result = 0;

	/* some bounds checking */
	if (which > num)
	{
		logerror("Attempting to access an unmapped 8255 chip.  PC: %04X\n", activecpu_get_pc());
		return 0xff;
	}

	offset %= 4;

	switch(offset)
	{
		case 0: /* Port A read */
		case 1: /* Port B read */
		case 2: /* Port C read */
			result = ppi8255_read_port(chip, offset);
			break;

		case 3: /* Control word */
			result = 0xFF;
			break;
	}

	return result;
}



static void ppi8255_write_port(ppi8255 *chip, int port)
{
	UINT8 write_data;

	write_data = chip->latch[port] & chip->out_mask[port];
	write_data |= 0xFF & ~chip->out_mask[port];

	/* write out special port 2 signals */
	if (port == 2)
		ppi8255_get_handshake_signals(chip, 0, &write_data);

	chip->output[port] = write_data;
	if (chip->port_write[port])
		chip->port_write[port](0, write_data);
}



void ppi8255_w(int which, offs_t offset, UINT8 data)
{
	ppi8255	*chip = &chips[which];

	/* Some bounds checking */
	if (which > num)
	{
		logerror("Attempting to access an unmapped 8255 chip.  PC: %04X\n", activecpu_get_pc());
		return;
	}

	offset %= 4;

  	switch( offset )
  	{
	  	case 0: /* Port A write */
		case 1: /* Port B write */
  		case 2: /* Port C write */
			chip->latch[offset] = data;
			ppi8255_write_port(chip, offset);

			switch(offset)
			{
				case 0:
					if (!chip->portA_dir && (chip->groupA_mode != 0))
					{
						chip->obf_a = 1;
						ppi8255_write_port(chip, 2);
					}
					break;

				case 1:
					if (!chip->portB_dir && (chip->groupB_mode != 0))
					{
						chip->obf_b = 1;
						ppi8255_write_port(chip, 2);
					}
					break;
			}
		  	break;

		case 3: /* Control word */
			if (data & 0x80)
			{
				set_mode(which, data & 0x7f, 1);
			}
			else
			{
	  			/* bit set/reset */
	  			int bit;

	  			bit = (data >> 1) & 0x07;

	  			if (data & 1)
					chip->latch[2] |= (1<<bit);		/* set bit */
	  			else
					chip->latch[2] &= ~(1<<bit);	/* reset bit */

				ppi8255_write_port(chip, 2);
			}
			break;
	}
}

#ifdef MESS
UINT8 ppi8255_peek( int which, offs_t offset )
{
	ppi8255 *chip;


	/* Some bounds checking */
	if (which > num)
	{
		logerror("Attempting to access an unmapped 8255 chip.  PC: %04X\n", activecpu_get_pc());
		return 0xff;
	}

	chip = &chips[which];


	if (offset > 2)
	{
		logerror("Attempting to access an invalid 8255 port.  PC: %04X\n", activecpu_get_pc());
		return 0xff;
	}


	chip = &chips[which];

	return chip->latch[offset];
}
#endif


void ppi8255_set_portAread(int which, read8_handler portAread)
{
	chips[which].port_read[0] = portAread;
}

void ppi8255_set_portBread(int which, read8_handler portBread)
{
	chips[which].port_read[1] = portBread;
}

void ppi8255_set_portCread(int which, read8_handler portCread)
{
	chips[which].port_read[2] = portCread;
}


void ppi8255_set_portAwrite(int which, write8_handler portAwrite)
{
	chips[which].port_write[0] = portAwrite;
}

void ppi8255_set_portBwrite(int which, write8_handler portBwrite)
{
	chips[which].port_write[1] = portBwrite;
}

void ppi8255_set_portCwrite(int which, write8_handler portCwrite)
{
	chips[which].port_write[2] = portCwrite;
}


static void set_mode(int which, int data, int call_handlers)
{
	ppi8255 *chip = &chips[which];
	int i;

	/* parse out mode */
	chip->groupA_mode = (data >> 5) & 3;
	chip->groupB_mode = (data >> 2) & 1;
	chip->portA_dir = (data >> 4) & 1;
	chip->portB_dir = (data >> 1) & 1;
	chip->portCH_dir = (data >> 3) & 1;
	chip->portCL_dir = (data >> 0) & 1;

	/* normalize groupA_mode */
	if (chip->groupA_mode == 3)
		chip->groupA_mode = 2;

  	/* Port A direction */
	if (chip->portA_dir)
		chip->in_mask[0] = 0xFF, chip->out_mask[0] = 0x00;	/* input */
    else
		chip->in_mask[0] = 0x00, chip->out_mask[0] = 0xFF; 	/* output */

  	/* Port B direction */
	if (chip->portB_dir)
		chip->in_mask[1] = 0xFF, chip->out_mask[1] = 0x00;	/* input */
	else
		chip->in_mask[1] = 0x00, chip->out_mask[1] = 0xFF; 	/* output */

	/* Port C upper direction */
	if (chip->portCH_dir)
		chip->in_mask[2] = 0xF0, chip->out_mask[2] = 0x00;	/* input */
	else
		chip->in_mask[2] = 0x00, chip->out_mask[2] = 0xF0;	/* output */

  	/* Port C lower direction */
	if (chip->portCL_dir)
		chip->in_mask[2] |= 0x0F;	/* input */
	else
		chip->out_mask[2] |= 0x0F;	/* output */

	/* now depending on the group modes, certain Port C lines may be replaced
     * with varying control signals */
	switch(chip->groupA_mode)
	{
		case 0:	/* Group A mode 0 */
			/* no changes */
			break;

		case 1:	/* Group A mode 1 */
			/* bits 5-3 are reserved by Group A mode 1 */
			chip->in_mask[2] &= ~0x38;
			chip->out_mask[2] &= ~0x38;
			break;

		case 2: /* Group A mode 2 */
			/* bits 7-3 are reserved by Group A mode 2 */
			chip->in_mask[2] &= ~0xF8;
			chip->out_mask[2] &= ~0xF8;
			break;
	}

	switch(chip->groupB_mode)
	{
		case 0:	/* Group B mode 0 */
			/* no changes */
			break;

		case 1:	/* Group B mode 1 */
			/* bits 2-0 are reserved by Group B mode 1 */
			chip->in_mask[2] &= ~0x07;
			chip->out_mask[2] &= ~0x07;
			break;
		}

	/* KT: 25-Dec-99 - 8255 resets latches when mode set */
	chip->latch[0] = chip->latch[1] = chip->latch[2] = 0;

	if (call_handlers)
	{
		for (i = 0; i < 3; i++)
			ppi8255_write_port(chip, i);
	}
}


void ppi8255_set_portA( int which, UINT8 data ) { ppi8255_input(&chips[which], 0, data); }
void ppi8255_set_portB( int which, UINT8 data ) { ppi8255_input(&chips[which], 1, data); }
void ppi8255_set_portC( int which, UINT8 data ) { ppi8255_input(&chips[which], 2, data); }

UINT8 ppi8255_get_portA( int which ) { return chips[which].output[0]; }
UINT8 ppi8255_get_portB( int which ) { return chips[which].output[1]; }
UINT8 ppi8255_get_portC( int which ) { return chips[which].output[2]; }

/* Helpers */
READ8_HANDLER( ppi8255_0_r ) { return ppi8255_r( 0, offset ); }
READ8_HANDLER( ppi8255_1_r ) { return ppi8255_r( 1, offset ); }
READ8_HANDLER( ppi8255_2_r ) { return ppi8255_r( 2, offset ); }
READ8_HANDLER( ppi8255_3_r ) { return ppi8255_r( 3, offset ); }
READ8_HANDLER( ppi8255_4_r ) { return ppi8255_r( 4, offset ); }
READ8_HANDLER( ppi8255_5_r ) { return ppi8255_r( 5, offset ); }
READ8_HANDLER( ppi8255_6_r ) { return ppi8255_r( 6, offset ); }
READ8_HANDLER( ppi8255_7_r ) { return ppi8255_r( 7, offset ); }
WRITE8_HANDLER( ppi8255_0_w ) { ppi8255_w( 0, offset, data ); }
WRITE8_HANDLER( ppi8255_1_w ) { ppi8255_w( 1, offset, data ); }
WRITE8_HANDLER( ppi8255_2_w ) { ppi8255_w( 2, offset, data ); }
WRITE8_HANDLER( ppi8255_3_w ) { ppi8255_w( 3, offset, data ); }
WRITE8_HANDLER( ppi8255_4_w ) { ppi8255_w( 4, offset, data ); }
WRITE8_HANDLER( ppi8255_5_w ) { ppi8255_w( 5, offset, data ); }
WRITE8_HANDLER( ppi8255_6_w ) { ppi8255_w( 6, offset, data ); }
WRITE8_HANDLER( ppi8255_7_w ) { ppi8255_w( 7, offset, data ); }

READ16_HANDLER( ppi8255_16le_0_r ) { return read16le_with_read8_handler(ppi8255_0_r, offset, mem_mask); }
READ16_HANDLER( ppi8255_16le_1_r ) { return read16le_with_read8_handler(ppi8255_1_r, offset, mem_mask); }
READ16_HANDLER( ppi8255_16le_2_r ) { return read16le_with_read8_handler(ppi8255_2_r, offset, mem_mask); }
READ16_HANDLER( ppi8255_16le_3_r ) { return read16le_with_read8_handler(ppi8255_3_r, offset, mem_mask); }
READ16_HANDLER( ppi8255_16le_4_r ) { return read16le_with_read8_handler(ppi8255_4_r, offset, mem_mask); }
READ16_HANDLER( ppi8255_16le_5_r ) { return read16le_with_read8_handler(ppi8255_5_r, offset, mem_mask); }
READ16_HANDLER( ppi8255_16le_6_r ) { return read16le_with_read8_handler(ppi8255_6_r, offset, mem_mask); }
READ16_HANDLER( ppi8255_16le_7_r ) { return read16le_with_read8_handler(ppi8255_7_r, offset, mem_mask); }
WRITE16_HANDLER( ppi8255_16le_0_w ) { write16le_with_write8_handler(ppi8255_0_w, offset, data, mem_mask); }
WRITE16_HANDLER( ppi8255_16le_1_w ) { write16le_with_write8_handler(ppi8255_1_w, offset, data, mem_mask); }
WRITE16_HANDLER( ppi8255_16le_2_w ) { write16le_with_write8_handler(ppi8255_2_w, offset, data, mem_mask); }
WRITE16_HANDLER( ppi8255_16le_3_w ) { write16le_with_write8_handler(ppi8255_3_w, offset, data, mem_mask); }
WRITE16_HANDLER( ppi8255_16le_4_w ) { write16le_with_write8_handler(ppi8255_4_w, offset, data, mem_mask); }
WRITE16_HANDLER( ppi8255_16le_5_w ) { write16le_with_write8_handler(ppi8255_5_w, offset, data, mem_mask); }
WRITE16_HANDLER( ppi8255_16le_6_w ) { write16le_with_write8_handler(ppi8255_6_w, offset, data, mem_mask); }
WRITE16_HANDLER( ppi8255_16le_7_w ) { write16le_with_write8_handler(ppi8255_7_w, offset, data, mem_mask); }
