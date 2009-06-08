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

typedef struct _ppi8255 ppi8255_t;

struct _ppi8255
{
	const ppi8255_interface	*intf;

	devcb_resolved_read8 port_read[3];
	devcb_resolved_write8 port_write[3];

	/* mode flags */
	UINT8 group_a_mode;
	UINT8 group_b_mode;
	UINT8 port_a_dir;
	UINT8 port_b_dir;
	UINT8 port_ch_dir;
	UINT8 port_cl_dir;

	/* handshake signals (1=asserted; 0=non-asserted) */
	UINT8 obf_a;
	UINT8 obf_b;
	UINT8 ibf_a;
	UINT8 ibf_b;
	UINT8 inte_a;
	UINT8 inte_b;
	UINT8 inte_1;
	UINT8 inte_2;

	UINT8 in_mask[3];	/* input mask */
	UINT8 out_mask[3];	/* output mask */
	UINT8 read[3];		/* data read from ports */
	UINT8 latch[3];		/* data written to ports */
	UINT8 output[3];	/* actual output data */
	UINT8 control;		/* mode control word */
};


static void set_mode(const device_config *device, int data, int call_handlers);
static void ppi8255_write_port(const device_config *device, int port);


INLINE ppi8255_t *get_safe_token(const device_config *device) {
	assert( device != NULL );
	assert( device->token != NULL );
	assert( device->type == DEVICE_GET_INFO_NAME(ppi8255) );
	return ( ppi8255_t * ) device->token;
}


INLINE void ppi8255_get_handshake_signals(ppi8255_t *ppi8255, int is_read, UINT8 *result)
{
	UINT8 handshake = 0x00;
	UINT8 mask = 0x00;

	/* group A */
	if (ppi8255->group_a_mode == 1)
	{
		if (ppi8255->port_a_dir)
		{
			handshake |= ppi8255->ibf_a ? 0x20 : 0x00;
			handshake |= (ppi8255->ibf_a && ppi8255->inte_a) ? 0x08 : 0x00;
			mask |= 0x28;
		}
		else
		{
			handshake |= ppi8255->obf_a ? 0x00 : 0x80;
			handshake |= (ppi8255->obf_a && ppi8255->inte_a) ? 0x08 : 0x00;
			mask |= 0x88;
		}
	}
	else if (ppi8255->group_a_mode == 2)
  	{
		handshake |= ppi8255->obf_a ? 0x00 : 0x80;
		handshake |= ppi8255->ibf_a ? 0x20 : 0x00;
		handshake |= ((ppi8255->obf_a && ppi8255->inte_1) || (ppi8255->ibf_a && ppi8255->inte_2)) ? 0x08 : 0x00;
		mask |= 0xA8;
	}

	/* group B */
	if (ppi8255->group_b_mode == 1)
  	{
		if (ppi8255->port_b_dir)
		{
			handshake |= ppi8255->ibf_b ? 0x02 : 0x00;
			handshake |= (ppi8255->ibf_b && ppi8255->inte_b) ? 0x01 : 0x00;
			mask |= 0x03;
  		}
		else
  		{
			handshake |= ppi8255->obf_b ? 0x00 : 0x02;
			handshake |= (ppi8255->obf_b && ppi8255->inte_b) ? 0x01 : 0x00;
			mask |= 0x03;
		}
  	}

	*result &= ~mask;
	*result |= handshake & mask;
}



static void ppi8255_input(const device_config *device, int port, UINT8 data)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	int changed = 0;

	ppi8255->read[port] = data;

	/* port C is special */
	if (port == 2)
	{
		if (((ppi8255->group_a_mode == 1) && (ppi8255->port_a_dir == 0)) || (ppi8255->group_a_mode == 2))
		{
			/* is !ACKA asserted? */
			if (ppi8255->obf_a && !(data & 0x40))
			{
				ppi8255->obf_a = 0;
				changed = 1;
			}
		}

		if (((ppi8255->group_a_mode == 1) && (ppi8255->port_a_dir == 1)) || (ppi8255->group_a_mode == 2))
		{
			/* is !STBA asserted? */
			if (!ppi8255->ibf_a && !(data & 0x10))
			{
				ppi8255->ibf_a = 1;
				changed = 1;
			}
		}

		if ((ppi8255->group_b_mode == 1) && (ppi8255->port_b_dir == 0))
		{
			/* is !ACKB asserted? */
			if (ppi8255->obf_b && !(data & 0x04))
			{
				ppi8255->obf_b = 0;
				changed = 1;
			}
		}

		if ((ppi8255->group_b_mode == 1) && (ppi8255->port_b_dir == 1))
		{
			/* is !STBB asserted? */
			if (!ppi8255->ibf_b && !(data & 0x04))
			{
				ppi8255->ibf_b = 1;
				changed = 1;
			}
		}

		if (changed)
			ppi8255_write_port(device, 2);
	}
}



static UINT8 ppi8255_read_port(const device_config *device, int port)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	UINT8 result = 0x00;

	if (ppi8255->in_mask[port])
	{
		if (ppi8255->port_read[port].read != NULL)
			ppi8255_input(device, port, devcb_call_read8(&ppi8255->port_read[port], 0));

		result |= ppi8255->read[port] & ppi8255->in_mask[port];
	}
	result |= ppi8255->latch[port] & ppi8255->out_mask[port];

	switch (port)
	{
	case 0:
		/* clear input buffer full flag */
		ppi8255->ibf_a = 0;
		break;

	case 1:
		/* clear input buffer full flag */
		ppi8255->ibf_b = 0;
		break;

	case 2:
		/* read special port 2 signals */
		ppi8255_get_handshake_signals(ppi8255, 1, &result);
		break;
	}

	return result;
}



READ8_DEVICE_HANDLER( ppi8255_r )
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	UINT8 result = 0;

	offset %= 4;

	switch(offset)
	{
		case 0: /* Port A read */
		case 1: /* Port B read */
		case 2: /* Port C read */
			result = ppi8255_read_port(device, offset);
			break;

		case 3: /* Control word */
			result = ppi8255->control;
			break;
	}

	return result;
}



static void ppi8255_write_port(const device_config *device, int port)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	UINT8 write_data;

	write_data = ppi8255->latch[port] & ppi8255->out_mask[port];
	write_data |= 0xFF & ~ppi8255->out_mask[port];

	/* write out special port 2 signals */
	if (port == 2)
		ppi8255_get_handshake_signals(ppi8255, 0, &write_data);

	ppi8255->output[port] = write_data;
	if (ppi8255->port_write[port].write != NULL)
		devcb_call_write8(&ppi8255->port_write[port], 0, write_data);
}



WRITE8_DEVICE_HANDLER( ppi8255_w )
{
	ppi8255_t	*ppi8255 = get_safe_token(device);

	offset %= 4;

  	switch( offset )
  	{
	  	case 0: /* Port A write */
		case 1: /* Port B write */
  		case 2: /* Port C write */
			ppi8255->latch[offset] = data;
			ppi8255_write_port(device, offset);

			switch(offset)
			{
				case 0:
					if (!ppi8255->port_a_dir && (ppi8255->group_a_mode != 0))
					{
						ppi8255->obf_a = 1;
						ppi8255_write_port(device, 2);
					}
					break;

				case 1:
					if (!ppi8255->port_b_dir && (ppi8255->group_b_mode != 0))
					{
						ppi8255->obf_b = 1;
						ppi8255_write_port(device, 2);
					}
					break;
			}
		  	break;

		case 3: /* Control word */
			if (data & 0x80)
			{
				set_mode(device, data & 0x7f, 1);
			}
			else
			{
	  			/* bit set/reset */
	  			int bit;

	  			bit = (data >> 1) & 0x07;

	  			if (data & 1)
					ppi8255->latch[2] |= (1<<bit);	/* set bit */
	  			else
					ppi8255->latch[2] &= ~(1<<bit);	/* reset bit */

				if (bit == 2 && ppi8255->group_b_mode == 1)	ppi8255->inte_b = data & 1;
				if (bit == 4 && ppi8255->group_a_mode == 1 && ppi8255->port_a_dir) ppi8255->inte_a = data & 1;
				if (bit == 6 && ppi8255->group_a_mode == 1 && !ppi8255->port_a_dir) ppi8255->inte_a = data & 1;
				if (bit == 4 && ppi8255->group_a_mode == 2) ppi8255->inte_2 = data & 1;
				if (bit == 6 && ppi8255->group_a_mode == 2) ppi8255->inte_1 = data & 1;

				ppi8255_write_port(device, 2);
			}
			break;
	}
}


void ppi8255_set_port_a_read(const device_config *device, const devcb_read8 *config)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	devcb_resolve_read8(&ppi8255->port_read[0], config, device);
}

void ppi8255_set_port_b_read(const device_config *device, const devcb_read8 *config)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	devcb_resolve_read8(&ppi8255->port_read[1], config, device);
}

void ppi8255_set_port_c_read(const device_config *device, const devcb_read8 *config)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	devcb_resolve_read8(&ppi8255->port_read[2], config, device);
}


void ppi8255_set_port_a_write(const device_config *device, const devcb_write8 *config)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	devcb_resolve_write8(&ppi8255->port_write[0], config, device);
}

void ppi8255_set_port_b_write(const device_config *device, const devcb_write8 *config)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	devcb_resolve_write8(&ppi8255->port_write[1], config, device);
}

void ppi8255_set_port_c_write(const device_config *device, const devcb_write8 *config)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	devcb_resolve_write8(&ppi8255->port_write[2], config, device);
}


static void set_mode(const device_config *device, int data, int call_handlers)
{
	ppi8255_t	*ppi8255 = get_safe_token(device);
	int i;

	/* parse out mode */
	ppi8255->group_a_mode = (data >> 5) & 3;
	ppi8255->group_b_mode = (data >> 2) & 1;
	ppi8255->port_a_dir = (data >> 4) & 1;
	ppi8255->port_b_dir = (data >> 1) & 1;
	ppi8255->port_ch_dir = (data >> 3) & 1;
	ppi8255->port_cl_dir = (data >> 0) & 1;

	/* normalize group_a_mode */
	if (ppi8255->group_a_mode == 3)
		ppi8255->group_a_mode = 2;

  	/* Port A direction */
	if (ppi8255->port_a_dir)
		ppi8255->in_mask[0] = 0xFF, ppi8255->out_mask[0] = 0x00;	/* input */
    else
		ppi8255->in_mask[0] = 0x00, ppi8255->out_mask[0] = 0xFF; 	/* output */

  	/* Port B direction */
	if (ppi8255->port_b_dir)
		ppi8255->in_mask[1] = 0xFF, ppi8255->out_mask[1] = 0x00;	/* input */
	else
		ppi8255->in_mask[1] = 0x00, ppi8255->out_mask[1] = 0xFF; 	/* output */

	/* Port C upper direction */
	if (ppi8255->port_ch_dir)
		ppi8255->in_mask[2] = 0xF0, ppi8255->out_mask[2] = 0x00;	/* input */
	else
		ppi8255->in_mask[2] = 0x00, ppi8255->out_mask[2] = 0xF0;	/* output */

  	/* Port C lower direction */
	if (ppi8255->port_cl_dir)
		ppi8255->in_mask[2] |= 0x0F;	/* input */
	else
		ppi8255->out_mask[2] |= 0x0F;	/* output */

	/* now depending on the group modes, certain Port C lines may be replaced
     * with varying control signals */
	switch(ppi8255->group_a_mode)
	{
		case 0:	/* Group A mode 0 */
			/* no changes */
			break;

		case 1:	/* Group A mode 1 */
			/* bits 5-3 are reserved by Group A mode 1 */
			ppi8255->in_mask[2] &= ~0x38;
			ppi8255->out_mask[2] &= ~0x38;
			break;

		case 2: /* Group A mode 2 */
			/* bits 7-3 are reserved by Group A mode 2 */
			ppi8255->in_mask[2] &= ~0xF8;
			ppi8255->out_mask[2] &= ~0xF8;
			break;
	}

	switch(ppi8255->group_b_mode)
	{
		case 0:	/* Group B mode 0 */
			/* no changes */
			break;

		case 1:	/* Group B mode 1 */
			/* bits 2-0 are reserved by Group B mode 1 */
			ppi8255->in_mask[2] &= ~0x07;
			ppi8255->out_mask[2] &= ~0x07;
			break;
		}

	/* KT: 25-Dec-99 - 8255 resets latches when mode set */
	ppi8255->latch[0] = ppi8255->latch[1] = ppi8255->latch[2] = 0;

	if (call_handlers)
	{
		for (i = 0; i < 3; i++)
			ppi8255_write_port(device, i);
	}

	/* reset flip-flops */
	ppi8255->obf_a = ppi8255->ibf_a = 0;
	ppi8255->obf_b = ppi8255->ibf_b = 0;
	ppi8255->inte_a = ppi8255->inte_b = ppi8255->inte_1 = ppi8255->inte_2 = 0;

	/* store control word */
	ppi8255->control = data;
}


void ppi8255_set_port_a( const device_config *device, UINT8 data ) { ppi8255_input(device, 0, data); }
void ppi8255_set_port_b( const device_config *device, UINT8 data ) { ppi8255_input(device, 1, data); }
void ppi8255_set_port_c( const device_config *device, UINT8 data ) { ppi8255_input(device, 2, data); }

UINT8 ppi8255_get_port_a( const device_config *device ) {
	ppi8255_t	*ppi8255 = get_safe_token(device);

	return ppi8255->output[0];
}

UINT8 ppi8255_get_port_b( const device_config *device ) {
	ppi8255_t	*ppi8255 = get_safe_token(device);

	return ppi8255->output[1];
}

UINT8 ppi8255_get_port_c( const device_config *device ) {
	ppi8255_t	*ppi8255 = get_safe_token(device);

	return ppi8255->output[2];
}


static DEVICE_START( ppi8255 ) {
	ppi8255_t	*ppi8255 = get_safe_token(device);

	ppi8255->intf = (const ppi8255_interface *)device->static_config;

	devcb_resolve_read8(&ppi8255->port_read[0], &ppi8255->intf->port_a_read, device);
	devcb_resolve_read8(&ppi8255->port_read[1], &ppi8255->intf->port_b_read, device);
	devcb_resolve_read8(&ppi8255->port_read[2], &ppi8255->intf->port_c_read, device);

	devcb_resolve_write8(&ppi8255->port_write[0], &ppi8255->intf->port_a_write, device);
	devcb_resolve_write8(&ppi8255->port_write[1], &ppi8255->intf->port_b_write, device);
	devcb_resolve_write8(&ppi8255->port_write[2], &ppi8255->intf->port_c_write, device);

	/* register for state saving */
	state_save_register_device_item(device, 0, ppi8255->group_a_mode);
	state_save_register_device_item(device, 0, ppi8255->group_b_mode);
	state_save_register_device_item(device, 0, ppi8255->port_a_dir);
	state_save_register_device_item(device, 0, ppi8255->port_b_dir);
	state_save_register_device_item(device, 0, ppi8255->port_ch_dir);
	state_save_register_device_item(device, 0, ppi8255->port_cl_dir);
	state_save_register_device_item(device, 0, ppi8255->obf_a);
	state_save_register_device_item(device, 0, ppi8255->obf_b);
	state_save_register_device_item(device, 0, ppi8255->ibf_a);
	state_save_register_device_item(device, 0, ppi8255->ibf_b);
	state_save_register_device_item(device, 0, ppi8255->inte_a);
	state_save_register_device_item(device, 0, ppi8255->inte_b);
	state_save_register_device_item(device, 0, ppi8255->inte_1);
	state_save_register_device_item(device, 0, ppi8255->inte_2);
	state_save_register_device_item_array(device, 0, ppi8255->in_mask);
	state_save_register_device_item_array(device, 0, ppi8255->out_mask);
	state_save_register_device_item_array(device, 0, ppi8255->read);
	state_save_register_device_item_array(device, 0, ppi8255->latch);
}


static DEVICE_RESET( ppi8255 ) {
	ppi8255_t	*ppi8255 = get_safe_token(device);
	int			i;

	ppi8255->group_a_mode = 0;
	ppi8255->group_b_mode = 0;
	ppi8255->port_a_dir = 0;
	ppi8255->port_b_dir = 0;
	ppi8255->port_ch_dir = 0;
	ppi8255->port_cl_dir = 0;
	ppi8255->obf_a = ppi8255->ibf_a = 0;
	ppi8255->obf_b = ppi8255->ibf_b = 0;
	ppi8255->inte_a = ppi8255->inte_b = ppi8255->inte_1 = ppi8255->inte_2 = 0;

	for ( i = 0; i < 3; i++ ) {
		ppi8255->in_mask[i] = ppi8255->out_mask[i] = ppi8255->read[i] = ppi8255->latch[i] = ppi8255->output[i] = 0;
	}

	set_mode(device, 0x9b, 0);   /* Mode 0, all ports set to input */
}


DEVICE_GET_INFO(ppi8255) {
	switch ( state ) {
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(ppi8255_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0;								break;
		case DEVINFO_INT_CLASS:						info->i = DEVICE_CLASS_PERIPHERAL;			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(ppi8255);	break;
		case DEVINFO_FCT_STOP:						/* nothing */								break;
		case DEVINFO_FCT_RESET:						info->reset = DEVICE_RESET_NAME(ppi8255);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "Intel PPI8255");			break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "PPI8255");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.00");					break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright the MAME and MESS Teams"); break;
	}
}

