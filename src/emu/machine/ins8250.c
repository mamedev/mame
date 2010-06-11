/**********************************************************************

    National Semiconductor 8250 UART interface and emulation

   More information on the different models can be found in
   section 1.6 at this location:
     http://www.freebsd.org/doc/en_US.ISO8859-1/articles/serial-uart/

Model overview (from page above):

INS8250
This part was used in the original IBM PC and IBM PC/XT. The original name
for this part was the INS8250 ACE (Asynchronous Communications Element) and
it is made from NMOS technology.

The 8250 uses eight I/O ports and has a one-byte send and a one-byte receive
buffer. This original UART has several race conditions and other flaws. The
original IBM BIOS includes code to work around these flaws, but this made
the BIOS dependent on the flaws being present, so subsequent parts like the
8250A, 16450 or 16550 could not be used in the original IBM PC or IBM PC/XT.

INS8250-B
This is the slower speed of the INS8250 made from NMOS technology. It contains
the same problems as the original INS8250.

INS8250A
An improved version of the INS8250 using XMOS technology with various functional
flaws corrected. The INS8250A was used initially in PC clone computers by vendors
who used "clean" BIOS designs. Because of the corrections in the chip, this part
could not be used with a BIOS compatible with the INS8250 or INS8250B.

INS82C50A
This is a CMOS version (low power consumption) of the INS8250A and has similar
functional characteristics.

NS16450
Same as NS8250A with improvements so it can be used with faster CPU bus designs.
IBM used this part in the IBM AT and updated the IBM BIOS to no longer rely on
the bugs in the INS8250.

NS16C450
This is a CMOS version (low power consumption) of the NS16450.

NS16550
Same as NS16450 with a 16-byte send and receive buffer but the buffer design
was flawed and could not be reliably be used.

NS16550A
Same as NS16550 with the buffer flaws corrected. The 16550A and its successors
have become the most popular UART design in the PC industry, mainly due to
its ability to reliably handle higher data rates on operating systems with
sluggish interrupt response times.

NS16C552
This component consists of two NS16C550A CMOS UARTs in a single package.

PC16550D
Same as NS16550A with subtle flaws corrected. This is revision D of the
16550 family and is the latest design available from National Semiconductor.


Known issues:
- MESS does currently not handle all these model specific features.


History:
    KT - 14-Jun-2000 - Improved Interrupt setting/clearing
    KT - moved into seperate file so it can be used in Super I/O emulation and
        any other system which uses a PC type COM port
    KT - 24-Jun-2000 - removed pc specific input port tests. More compatible
        with PCW16 and PCW16 doesn't requre the PC input port definitions
        which are not required by the PCW16 hardware

**********************************************************************/

#include "emu.h"
#include "machine/ins8250.h"


#define LOG(LEVEL,N,M,A)  \
	do { \
		if( M ) \
			logerror("%-24s",(char*)M ); \
		logerror A; \
	} while (0)


/* device types */
enum {
	TYPE_INS8250 = 0,
	TYPE_INS8250A,
	TYPE_NS16450,
	TYPE_NS16550,
	TYPE_NS16550A,
	TYPE_PC16550D,

	NUM_TYPES
};


/* device tags */
static const char * const device_tags[NUM_TYPES] = { "ins8250", "ins8250a", "ns16450", "ns16550", "ns16550a", "pc16550d" };


#define VERBOSE_COM 0
#define COM_LOG(n,m,a) LOG(VERBOSE_COM,n,m,a)

typedef struct {
	const ins8250_interface *interface;
	int	device_type;

	UINT8 thr;  /* 0 -W transmitter holding register */
	UINT8 rbr; /* 0 R- receiver buffer register */
	UINT8 ier;  /* 1 RW interrupt enable register */
	UINT8 dll;  /* 0 RW divisor latch lsb (if DLAB = 1) */
	UINT8 dlm;  /* 1 RW divisor latch msb (if DLAB = 1) */
	UINT8 iir;  /* 2 R- interrupt identification register */
	UINT8 lcr;  /* 3 RW line control register (bit 7: DLAB) */
	UINT8 mcr;  /* 4 RW modem control register */
	UINT8 lsr;  /* 5 R- line status register */
	UINT8 msr;  /* 6 R- modem status register */
	UINT8 scr;  /* 7 RW scratch register */

/* holds int pending state for com */
	UINT8 int_pending;

	// sending circuit
	struct {
		int active;
		UINT8 data;
		double time;
	} send;
} ins8250_t;

/* int's pending */
#define COM_INT_PENDING_RECEIVED_DATA_AVAILABLE	0x0001
#define COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY 0x0002
#define COM_INT_PENDING_RECEIVER_LINE_STATUS 0x0004
#define COM_INT_PENDING_MODEM_STATUS_REGISTER 0x0008


INLINE ins8250_t *get_safe_token(running_device *device)
{
	assert( device != NULL );
	assert( ( device->type() == INS8250 ) ||
			( device->type() == INS8250A ) ||
			( device->type() == NS16450 ) ||
			( device->type() == NS16550 ) ||
			( device->type() == NS16550A ) ||
			( device->type() == PC16550D ) );
	return (ins8250_t *)downcast<legacy_device_base *>(device)->token();
}


/* setup iir with the priority id */
static void ins8250_setup_iir(running_device *device)
{
	ins8250_t	*ins8250 = get_safe_token(device);

	ins8250->iir &= ~(0x04|0x02);

	/* highest to lowest */
	if (ins8250->ier & ins8250->int_pending & COM_INT_PENDING_RECEIVER_LINE_STATUS)
	{
		ins8250->iir |=0x04|0x02;
		return;
	}

	if (ins8250->ier & ins8250->int_pending & COM_INT_PENDING_RECEIVED_DATA_AVAILABLE)
	{
		ins8250->iir |=0x04;
		return;
	}

	if (ins8250->ier & ins8250->int_pending & COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY)
	{
		ins8250->iir |=0x02;
		return;
	}

	/* modem status has both bits clear */
}


/* ints will continue to be set for as long as there are ints pending */
static void ins8250_update_interrupt(running_device *device)
{
	ins8250_t	*ins8250 = get_safe_token(device);
	int state;

	/* if any bits are set and are enabled */
	if (((ins8250->int_pending&ins8250->ier) & 0x0f) != 0)
	{
		/* trigger next highest priority int */

		/* set int */
		state = 1;
		ins8250_setup_iir(device);

		/* int pending */
		ins8250->iir &= ~0x01;
	}
	else
	{
		/* clear int */
		state = 0;

		/* no ints pending */
		ins8250->iir |= 0x01;
		/* priority level */
		ins8250->iir &= ~(0x04|0x02);
	}

	/* set or clear the int */
	if (ins8250->interface->interrupt)
		ins8250->interface->interrupt(device, state);
}



/* set pending bit and trigger int */
static void ins8250_trigger_int(running_device *device, int flag)
{
	ins8250_t	*ins8250 = get_safe_token(device);

	ins8250->int_pending |= flag;
	ins8250_update_interrupt(device);
}



/* clear pending bit, if any ints are pending, then int will be triggered, otherwise it
will be cleared */
static void ins8250_clear_int(running_device *device, int flag)
{
	ins8250_t	*ins8250 = get_safe_token(device);

	ins8250->int_pending &= ~flag;
	ins8250_update_interrupt(device);
}


WRITE8_DEVICE_HANDLER( ins8250_w )
{
    static const char P[] = "NONENHNL";  /* names for parity select */
	ins8250_t	*ins8250 = get_safe_token(device);
    int tmp;

	switch (offset)
	{
		case 0:
			if (ins8250->lcr & 0x80)
			{
				ins8250->dll = data;
				tmp = ins8250->dlm * 256 + ins8250->dll;
				COM_LOG(1,"COM_dll_w",("COM \"%s\" $%02x: [$%04x = %d baud]\n", device->tag(),
					 data, tmp, (tmp)?(int)(ins8250->interface->clockin/16/tmp):0));
			}
			else
			{
				ins8250->thr = data;
				COM_LOG(2,"COM_thr_w",("COM $%02x\n", data));

				if ( ins8250->mcr & 0x10 )
				{
					ins8250->lsr |= 1;
					ins8250->rbr = data;
					ins8250_trigger_int( device, COM_INT_PENDING_RECEIVED_DATA_AVAILABLE );
				}

				if ( ins8250->interface->transmit )
					ins8250->interface->transmit(device, ins8250->thr);

				/* writing to thr will clear the int */
				ins8250_clear_int(device, COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
			}
			break;
		case 1:
			if (ins8250->lcr & 0x80)
			{
				ins8250->dlm = data;
				tmp = ins8250->dlm * 256 + ins8250->dll;
                COM_LOG(1,"COM_dlm_w",("COM \"%s\" $%02x: [$%04x = %d baud]\n", device->tag(),
					data, tmp, (tmp)?(int)(ins8250->interface->clockin/16/tmp):0));
			}
			else
			{
				ins8250->ier = data;
				COM_LOG(2,"COM_ier_w",("COM \"%s\" $%02x: enable int on RX %d, THRE %d, RLS %d, MS %d\n", device->tag(),
					data, data&1, (data>>1)&1, (data>>2)&1, (data>>3)&1));
				COM_LOG(2,"COM_ier_w",("COM \"%s\" lsr = $%02x, int_pending = $%02x\n", device->tag(), ins8250->lsr, ins8250->int_pending ));
				ins8250_update_interrupt(device);
			}
            break;
		case 2:
			COM_LOG(1,"COM_fcr_w",("COM \"%s\" $%02x (16550 only)\n", device->tag(), data));
            break;
		case 3:
			ins8250->lcr = data;
			COM_LOG(1,"COM_lcr_w",("COM \"%s\" $%02x word length %d, stop bits %d, parity %c, break %d, DLAB %d\n", device->tag(),
				data, 5+(data&3), 1+((data>>2)&1), P[(data>>3)&7], (data>>6)&1, (data>>7)&1));
            break;
		case 4:
			if ( ( ins8250->mcr & 0x1f ) != ( data & 0x1f ) )
			{
				ins8250->mcr = data & 0x1f;
				COM_LOG(1,"COM_mcr_w",("COM \"%s\" $%02x DTR %d, RTS %d, OUT1 %d, OUT2 %d, loopback %d\n", device->tag(),
					data, data&1, (data>>1)&1, (data>>2)&1, (data>>3)&1, (data>>4)&1));
				if (ins8250->interface->handshake_out)
					ins8250->interface->handshake_out(device,data);

				if ( ins8250->mcr & 0x10 )		/* loopback test */
				{
					data = ( ( ins8250->mcr & 0x0c ) << 4 ) | ( ( ins8250->mcr & 0x01 ) << 5 ) | ( ( ins8250->mcr & 0x02 ) << 3 );
					if ( ( ins8250->msr & 0x20 ) != ( data & 0x20 ) )
					{
						data |= 0x02;
					}
					if ( ( ins8250->msr & 0x10 ) != ( data & 0x10 ) )
					{
						data |= 0x01;
					}
					if ( ( ins8250->msr & 0x40 ) && ! ( data & 0x40 ) )
					{
						data |= 0x04;
					}
					if ( ( ins8250->msr & 0x80 ) != ( data & 0x80 ) )
					{
						data |= 0x08;
					}
					ins8250->msr = data;
				}
			}
            break;
		case 5:
			/*
              This register can be written, but if you write a 1 bit into any of
              bits 5 - 0, you could cause an interrupt if the appropriate IER bit
              is set.
            */
			COM_LOG(1,"COM_lsr_w",("COM \"%s\" $%02x\n", device->tag(), data ));

			ins8250->lsr = data;

			tmp = 0;
			tmp |= ( ins8250->lsr & 0x01 ) ? COM_INT_PENDING_RECEIVED_DATA_AVAILABLE : 0;
			tmp |= ( ins8250->lsr & 0x1e ) ? COM_INT_PENDING_RECEIVER_LINE_STATUS : 0;
			tmp |= ( ins8250->lsr & 0x20 ) ? COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY : 0;
			ins8250_trigger_int( device, tmp );

			break;
		case 6:
			/*
              This register can be written, but if you write a 1 bit into any of
              bits 3 - 0, you could cause an interrupt if the appropriate IER bit
              is set.
             */
			COM_LOG(1,"COM_msr_w",("COM \"%s\" $%02x\n", device->tag(), data ));

			ins8250->msr = data;

			if ( ins8250->msr & 0x0f )
			{
				ins8250_trigger_int( device, COM_INT_PENDING_MODEM_STATUS_REGISTER );
			}
			break;
		case 7:
			ins8250->scr = data;
			COM_LOG(2,"COM_scr_w",("COM \"%s\" $%02x\n", device->tag(), data));
            break;
	}

	if (ins8250->interface->refresh_connected)
		ins8250->interface->refresh_connected(device);
}



READ8_DEVICE_HANDLER( ins8250_r )
{
	ins8250_t	*ins8250 = get_safe_token(device);
	int data = 0x0ff;

	switch (offset)
	{
		case 0:
			if (ins8250->lcr & 0x80)
			{
				data = ins8250->dll;
				COM_LOG(1,"COM_dll_r",("COM \"%s\" $%02x\n", device->tag(), data));
			}
			else
			{
				data = ins8250->rbr;
				if( ins8250->lsr & 0x01 )
				{
					ins8250->lsr &= ~0x01;		/* clear data ready status */
					COM_LOG(2,"COM_rbr_r",("COM \"%s\" $%02x\n", device->tag(), data));
				}

				ins8250_clear_int(device, COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);
			}
			break;
		case 1:
			if (ins8250->lcr & 0x80)
			{
				data = ins8250->dlm;
				COM_LOG(1,"COM_dlm_r",("COM \"%s\" $%02x\n", device->tag(), data));
			}
			else
			{
				data = ins8250->ier & 0x0f;
				COM_LOG(2,"COM_ier_r",("COM \"%s\" $%02x\n", device->tag(), data));
            }
            break;
		case 2:
			data = ins8250->iir;
			COM_LOG(2,"COM_iir_r",("COM \"%s\" $%02x\n", device->tag(), data));
			/* The documentation says that reading this register will
            clear the int if this is the source of the int */
			if ( ins8250->ier & COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY )
			{
				ins8250_clear_int(device, COM_INT_PENDING_TRANSMITTER_HOLDING_REGISTER_EMPTY);
			}
            break;
		case 3:
			data = ins8250->lcr;
			COM_LOG(2,"COM_lcr_r",("COM \"%s\" $%02x\n", device->tag(), data));
            break;
		case 4:
			data = ins8250->mcr;
			COM_LOG(2,"COM_mcr_r",("COM \"%s\" $%02x\n", device->tag(), data));
            break;
		case 5:

#if 0
			if (ins8250->send.active && (timer_get_time(machine)-ins8250->send.time>uart_byte_time(n)))
			{
				// currently polling is enough for pc1512
				ins8250->lsr |= 0x40; /* set TSRE */
				ins8250->send.active = 0;
				if ( ins8250->mcr & 0x10 )
				{
					ins8250->lsr |= 1;
					ins8250->rbr = ins8250->send.data;
				}
			}
#endif
			ins8250->lsr |= 0x20; /* set THRE */
			data = ins8250->lsr;
			if( ins8250->lsr & 0x1f )
			{
				ins8250->lsr &= 0xe1; /* clear FE, PE and OE and BREAK bits */
				COM_LOG(2,"COM_lsr_r",("COM \"%s\" $%02x, DR %d, OE %d, PE %d, FE %d, BREAK %d, THRE %d, TSRE %d\n", device->tag(),
					data, data&1, (data>>1)&1, (data>>2)&1, (data>>3)&1, (data>>4)&1, (data>>5)&1, (data>>6)&1));
			}

			/* reading line status register clears int */
			ins8250_clear_int(device, COM_INT_PENDING_RECEIVER_LINE_STATUS);
            break;
		case 6:
			data = ins8250->msr;
			ins8250->msr &= 0xf0; /* reset delta values */
			COM_LOG(2,"COM_msr_r",("COM \"%s\" $%02x\n", device->tag(), data));

			/* reading msr clears int */
			ins8250_clear_int(device, COM_INT_PENDING_MODEM_STATUS_REGISTER);

			break;
		case 7:
			data = ins8250->scr;
			COM_LOG(2,"COM_scr_r",("COM \"%s\" $%02x\n", device->tag(), data));
            break;
	}

	if (ins8250->interface->refresh_connected)
		ins8250->interface->refresh_connected(device);

    return data;
}



void ins8250_receive(running_device *device, int data)
{
	ins8250_t	*ins8250 = get_safe_token(device);

    /* check if data rate 1200 baud is set */
	if( ins8250->dlm != 0x00 || ins8250->dll != 0x60 )
        ins8250->lsr |= 0x08; /* set framing error */

    /* if data not yet serviced */
	if( ins8250->lsr & 0x01 )
		ins8250->lsr |= 0x02; /* set overrun error */

    /* put data into receiver buffer register */
    ins8250->rbr = data;

    /* set data ready status */
    ins8250->lsr |= 0x01;

	/* set pending state for this interrupt. */
	ins8250_trigger_int(device, COM_INT_PENDING_RECEIVED_DATA_AVAILABLE);


//  /* OUT2 + received line data avail interrupt enabled? */
//  if( (COM_mcr[n] & 0x08) && (COM_ier[n] & 0x01) )
//  {
//      if (com_interface.interrupt)
//          com_interface.interrupt(4-(n&1), 1);
//
//  }
}

/**************************************************************************
 *  change the modem status register
 **************************************************************************/
void ins8250_handshake_in(running_device *device, int new_msr)
{
	ins8250_t	*ins8250 = get_safe_token(device);

	/* no change in modem status bits? */
	if( ((ins8250->msr ^ new_msr) & 0xf0) == 0 )
		return;

	/* set delta status bits 0..3 and new modem status bits 4..7 */
    ins8250->msr = (((ins8250->msr ^ new_msr) >> 4) & 0x0f) | (new_msr & 0xf0);

	ins8250_trigger_int(device, COM_INT_PENDING_MODEM_STATUS_REGISTER);

//  /* set up interrupt information register */
  //  COM_iir[n] &= ~(0x06 | 0x01);

//    /* OUT2 + modem status interrupt enabled? */
//  if( (COM_mcr[n] & 0x08) && (COM_ier[n] & 0x08) )
//  {
//      if (com_interface.interrupt)
//          com_interface.interrupt(4-(n&1), 1);
//  }
}


static void common_start( running_device *device, int device_type )
{
	ins8250_t	*ins8250 = get_safe_token(device);

	ins8250->interface = (const ins8250_interface*)device->baseconfig().static_config();
	ins8250->device_type = device_type;
}


static DEVICE_START( ins8250 )
{
	common_start( device, TYPE_INS8250 );
}


static DEVICE_START( ins8250a )
{
	common_start( device, TYPE_INS8250A );
}


static DEVICE_START( ns16450 )
{
	common_start( device, TYPE_NS16450 );
}


static DEVICE_START( ns16550 )
{
	common_start( device, TYPE_NS16550 );
}


static DEVICE_START( ns16550a )
{
	common_start( device, TYPE_NS16550A );
}


static DEVICE_START( pc16550d )
{
	common_start( device, TYPE_PC16550D );
}


static DEVICE_RESET( ins8250 )
{
	ins8250_t	*ins8250 = get_safe_token(device);

	ins8250->ier = 0;
	ins8250->iir = 1;
	ins8250->lcr = 0;
	ins8250->mcr = 0;
	ins8250->lsr = (1<<5) | (1<<6);

	ins8250->send.active=0;

	/* refresh with reset state of register */
	if (ins8250->interface->refresh_connected)
		ins8250->interface->refresh_connected(device);
}


DEVICE_GET_INFO( ins8250 )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:				info->i = sizeof(ins8250_t);				break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:		info->i = 0;								break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(ins8250);	break;
		case DEVINFO_FCT_STOP:						/* nothing */								break;
		case DEVINFO_FCT_RESET:						info->reset = DEVICE_RESET_NAME(ins8250);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "National Semiconductor INS8250/INS8250B");	break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "INS8250");					break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "1.00");					break;
		case DEVINFO_STR_SOURCE_FILE:				strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright the MESS Team");	break;
	}
}


DEVICE_GET_INFO( ins8250a )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "National Semiconductor INS8250A/INS82C50A");	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(ins8250a);	break;

		default:									DEVICE_GET_INFO_CALL(ins8250);				break;
	}
}


DEVICE_GET_INFO( ns16450 )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "National Semiconductor NS16450/PC16450");	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(ns16450);	break;

		default:									DEVICE_GET_INFO_CALL(ins8250);				break;
	}
}


DEVICE_GET_INFO( ns16550 )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "National Semiconductor NS16550/PC16550");	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(ns16550);	break;

		default:									DEVICE_GET_INFO_CALL(ins8250);				break;
	}
}


DEVICE_GET_INFO( ns16550a )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "National Semiconductor NS16550A/PC16550A");	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(ns16550a);	break;

		default:									DEVICE_GET_INFO_CALL(ins8250);				break;
	}
}


DEVICE_GET_INFO( pc16550d )
{
	switch ( state )
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_STR_NAME:						strcpy(info->s, "National Semiconductor PC16550D");	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:						info->start = DEVICE_START_NAME(pc16550d);	break;

		default:									DEVICE_GET_INFO_CALL(ins8250);				break;
	}
}



DEFINE_LEGACY_DEVICE(INS8250, ins8250);
DEFINE_LEGACY_DEVICE(INS8250A, ins8250a);
DEFINE_LEGACY_DEVICE(NS16450, ns16450);
DEFINE_LEGACY_DEVICE(NS16550, ns16550);
DEFINE_LEGACY_DEVICE(NS16550A, ns16550a);
DEFINE_LEGACY_DEVICE(PC16550D, pc16550d);
