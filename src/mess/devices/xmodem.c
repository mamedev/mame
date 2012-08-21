	/* XMODEM protocol implementation.

   Transfer between an emulated machine and an image using the XMODEM protocol.

   Used in the HP48 G/GX emulation.

   Based on the "Xmodem protol specification" by Ward Christensen.

   Does not support any extension (such as 16-bit CRC, 1K packet size,
   batchs, YMODEM, ZMODEM).

   Author: Antoine Mine'
   Date: 29/03/2008
 */

#include "emu.h"
#include "xmodem.h"


/* debugging */
#define VERBOSE          0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)


/* XMODEM protocol bytes */
#define XMODEM_SOH 0x01  /* data packet header */
#define XMODEM_EOT 0x04  /* end of transmission */
#define XMODEM_ACK 0x06  /* acknowledge (packet received) */
#define XMODEM_NAK 0x15  /* not-acknowledge (10-second timeout) */
#define XMODEM_CAN 0x18  /* cancel */

/* XMODEM protocol is:

   sender        receiver

            <--   <NAK>
   packet   -->
            <--   <ACK>
            ...
   packet   -->
            <--   <ACK>
   <EOT>    -->
            <--   <ACK>

   and packet is:  <SOH> <id> <0xff-id> <data0> ... <data127> <checksum>

   <id> is 1 for the first packet, 2 for the 2d, etc...
   <checksum> is the sum of <data0> to <data127> modulo 256

   there are always 128 bytes of data in a packet, so, any file transfered is
   rounded to a multiple of 128 bytes, using 0-filler
*/


/* our state */
#define XMODEM_NOIMAGE    0
#define XMODEM_IDLE       1
#define XMODEM_SENDING    2 /* image to emulated machine */
#define XMODEM_RECEIVING  3 /* emulated machine to image */

typedef struct {

	UINT8   m_block[132];          /* 3-byte header + 128-byte block + checksum */
	UINT32  m_id;                  /* block id, starting at 1 */
	UINT16  m_pos;                 /* position in block, including header */
	UINT8   m_state;               /* one of XMODEM_ */

	device_image_interface* m_image;  /* underlying image */

	emu_timer* m_timer;            /* used to send periodic NAKs */

	running_machine *m_machine;

	xmodem_config* m_conf;

} xmodem;

INLINE xmodem *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == XMODEM);

	return (xmodem*)downcast<legacy_device_base *>(device)->token();
}

/* compute checksum of the data part of the packet, i.e., sum modulo 256 */
static UINT8 xmodem_chksum( xmodem* state )
{
	UINT8 sum = 0;
	int i;
	for ( i = 0; i < 128; i++ ) sum += state->m_block[ i + 3 ];
	return sum;
}

/* fills state->m_block with the data for packet number state->m_id
   returns != 0 if fail or end of file
*/
static int xmodem_make_send_block( xmodem* state )
{
	if ( ! state->m_image ) return -1;
	memset( state->m_block + 3, 0, 128 );
	if ( state->m_image->fseek( (state->m_id - 1) * 128, SEEK_SET ) ) return -1;
	if ( state->m_image->fread( state->m_block + 3, 128 ) <= 0 ) return -1;
	state->m_block[0] = XMODEM_SOH;
	state->m_block[1] = state->m_id & 0xff;
	state->m_block[2] = 0xff - state->m_block[1];
	state->m_block[131] = xmodem_chksum( state );
	return 0;
}

/* checks the received state->m_block packet and stores the data in the image
   returns != 0 if fail (bad packet or bad image)
 */
static int xmodem_get_receive_block( xmodem* state )
{
	int next_id = state->m_id + 1;
	if ( ! state->m_image ) return -1;
	if ( state->m_image->is_readonly() ) return -1;
	if ( state->m_block[0] != XMODEM_SOH ) return -1;
	if ( state->m_block[1] != 0xff - state->m_block[2] ) return -1;
	if ( state->m_block[131] != xmodem_chksum( state ) ) return -1;
	if ( state->m_block[1] != (next_id & 0xff) ) return -1;
	if ( state->m_image->fseek( (next_id - 1) * 128, SEEK_SET ) ) return -1;
	if ( state->m_image->fwrite( state->m_block + 3, 128 ) != 128 ) return -1;
	return 0;
}

/* the outside (us) sends a byte to the emulated machine */
static void xmodem_send_byte( xmodem* state, UINT8 data )
{
	if ( state->m_conf && state->m_conf->send )
	{
		state->m_conf->send(*state->m_machine, data );
	}
}

static void xmodem_send_packet_byte( xmodem* state )
{
	assert( state->m_pos < 132 );
	xmodem_send_byte( state, state->m_block[ state->m_pos ] );
	state->m_pos++;
}


static TIMER_CALLBACK( xmodem_nak_cb )
{
	xmodem* state = (xmodem*) ptr;
	if ( state->m_state != XMODEM_IDLE ) return;
	LOG(( "xmodem: sending NAK keep-alive\n" ));
	xmodem_send_byte( state, XMODEM_NAK );
}

static void xmodem_make_idle( xmodem* state )
{
	LOG(( "xmodem: entering idle state\n" ));
	state->m_state = XMODEM_IDLE;
	state->m_id = 0;
	state->m_pos = 0;
        /* When idle, we send NAK periodically to tell the emulated machine that we are
       always ready to receive.
       The 2 sec start time is here so that the machine does not get NAK instead of
       ACK or EOT as the last byte of a transfer.
     */
	state->m_timer->adjust( attotime::from_seconds( 2 ), 0, attotime::from_seconds( 2 ) );
}

/* emulated machine has read the last byte we sent */
void xmodem_byte_transmitted( device_t *device )
{
	xmodem* state = get_safe_token(device);
	if ( (state->m_state == XMODEM_SENDING) && (state->m_pos < 132) )
	{
		/* send next byte */
		xmodem_send_packet_byte( state );
	}
}

/* emulated machine sends a byte to the outside (us) */
void xmodem_receive_byte( device_t *device, UINT8 data )
{
	xmodem* state = get_safe_token(device);
	switch ( state->m_state )
	{

	case XMODEM_NOIMAGE:
		break;

	case XMODEM_IDLE:
		if ( data == XMODEM_NAK )
		{
			/* start sending */
			LOG(( "xmodem: got NAK, start sending\n" ));
			state->m_id = 1;
			if ( xmodem_make_send_block( state ) )
			{
				/* error */
				LOG(( "xmodem: nothing to send, sending NAK\n" ));
				xmodem_send_byte( state, XMODEM_NAK );
				xmodem_make_idle( state );
			}
			else
			{
				/* send first packet */
				state->m_state = XMODEM_SENDING;
				state->m_pos = 0;
				xmodem_send_packet_byte( state );
			}
			break;
		}
		else if ( data == XMODEM_SOH )
		{
			/* start receiving */
			LOG(( "xmodem: got SOH, start receiving\n" ));
			state->m_state = XMODEM_RECEIVING;
			state->m_block[ 0 ] = data;
			state->m_pos = 1;
			state->m_id = 0;
		}
		else
		{
			LOG(( "xmodem: ignored data %02x while idle\n", data ));
		}
		break;

	case XMODEM_SENDING:
		if ( (data != XMODEM_NAK) && (data != XMODEM_ACK) )
		{
			/* error */
			LOG(( "xmodem: invalid date %02x while sending, sending CAN\n", data ));
			xmodem_send_byte( state, XMODEM_CAN );
			xmodem_make_idle( state );
			break;
		}
		if ( data == XMODEM_ACK )
		{
			/* send next packet */
			state->m_id++;
			LOG(( "xmodem: got ACK, sending next packet (%i)\n", state->m_id ));
			if ( xmodem_make_send_block( state ) )
			{
				/* end of file */
				LOG(( "xmodem: no more packet, sending EOT\n" ));
				xmodem_send_byte( state, XMODEM_EOT );
				xmodem_make_idle( state );
				break;
			}

		}
		else
		{
			/* empty - resend last packet */
			LOG(( "xmodem: got NAK, resending packet %i\n", state->m_id ));
		}
		state->m_pos = 0;
		xmodem_send_packet_byte( state );
		break;


	case XMODEM_RECEIVING:
		assert( state->m_pos < 132 );
		state->m_block[ state->m_pos ] = data;
		state->m_pos++;
		if ( state->m_pos == 1 )
		{
			/* header byte */
			if ( data == XMODEM_EOT )
			{
				/* end of file */
				LOG(( "xmodem: got EOT, stop receiving\n" ));
				xmodem_send_byte( state, XMODEM_ACK );
				xmodem_make_idle( state );
				break;
			}
		}
		else if ( state->m_pos == 132 )
		{
			LOG(( "xmodem: received packet %i\n", state->m_id ));
			/* end of block */
			if ( xmodem_get_receive_block( state ) )
			{
				/* error */
				LOG(( "xmodem: packet is invalid, sending NAK\n" ));
				xmodem_send_byte( state, XMODEM_NAK );
			}
			else
			{
				/* ok */
				LOG(( "xmodem: packet is valid, sending ACK\n" ));
				xmodem_send_byte( state, XMODEM_ACK );
				state->m_id++;
			}
			state->m_pos = 0;
		}
		break;

	}
}

static DEVICE_START( xmodem )
{
	xmodem* state = get_safe_token(device);
	LOG(( "xmodem: start\n" ));
	state->m_state = XMODEM_NOIMAGE;
	state->m_image = NULL;
	state->m_conf = (xmodem_config*) device->static_config();
	state->m_machine = &device->machine();
	state->m_timer = device->machine().scheduler().timer_alloc(FUNC(xmodem_nak_cb), state );
}

static DEVICE_RESET( xmodem )
{
	xmodem* state = get_safe_token(device);
	LOG(( "xmodem: reset\n" ));
	if ( state->m_state != XMODEM_NOIMAGE ) xmodem_make_idle( state );
}

static DEVICE_IMAGE_LOAD( xmodem )
{
	xmodem* state = get_safe_token(&image.device());
	LOG(( "xmodem: image load\n" ));
	state->m_image = &image;
	xmodem_make_idle( state );
	return IMAGE_INIT_PASS;
}

static DEVICE_IMAGE_CREATE( xmodem )
{
	xmodem* state = get_safe_token(&image.device());
	LOG(( "xmodem: image create\n" ));
	state->m_image = &image;
	xmodem_make_idle( state );
	return IMAGE_INIT_PASS;
}

static DEVICE_IMAGE_UNLOAD( xmodem )
{
	xmodem* state = get_safe_token(&image.device());
	LOG(( "xmodem: image unload\n" ));
	state->m_state = XMODEM_NOIMAGE;
	state->m_image = NULL;
}

DEVICE_GET_INFO( xmodem )

{
	switch ( state ) {
	case DEVINFO_INT_TOKEN_BYTES:               info->i = sizeof( xmodem );                              break;
	case DEVINFO_INT_INLINE_CONFIG_BYTES:       info->i = 0;                                             break;
	case DEVINFO_INT_IMAGE_TYPE:	            info->i = IO_SERIAL;                                     break;
	case DEVINFO_INT_IMAGE_READABLE:            info->i = 1;                                             break;
	case DEVINFO_INT_IMAGE_WRITEABLE:	    info->i = 1;                                             break;
	case DEVINFO_INT_IMAGE_CREATABLE:	    info->i = 1;                                             break;
	case DEVINFO_FCT_START:	                    info->start = DEVICE_START_NAME( xmodem );               break;
	case DEVINFO_FCT_RESET:	                    info->reset = DEVICE_RESET_NAME( xmodem );               break;
	case DEVINFO_FCT_IMAGE_LOAD:		    info->f = (genf *) DEVICE_IMAGE_LOAD_NAME( xmodem );     break;
	case DEVINFO_FCT_IMAGE_UNLOAD:		    info->f = (genf *) DEVICE_IMAGE_UNLOAD_NAME( xmodem );   break;
	case DEVINFO_FCT_IMAGE_CREATE:		    info->f = (genf *) DEVICE_IMAGE_CREATE_NAME( xmodem );   break;
	case DEVINFO_STR_IMAGE_BRIEF_INSTANCE_NAME: strcpy(info->s, "x");	                                     break;
	case DEVINFO_STR_IMAGE_INSTANCE_NAME:
	case DEVINFO_STR_NAME:		            strcpy(info->s, "Xmodem");	                                     break;
	case DEVINFO_STR_FAMILY:                    strcpy(info->s, "Serial protocol");	                     break;
	case DEVINFO_STR_SOURCE_FILE:		    strcpy(info->s, __FILE__);                                      break;
	case DEVINFO_STR_IMAGE_FILE_EXTENSIONS:	    strcpy(info->s, "");                                            break;
	}
}

DEFINE_LEGACY_IMAGE_DEVICE(XMODEM, xmodem);
