/*********************************************************************

    bitbngr.c

    TRS style "bitbanger" serial port

*********************************************************************/

#include "emu.h"
#include "bitbngr.h"



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _bitbanger_token bitbanger_token;
struct _bitbanger_token
{
	emu_timer *bitbanger_output_timer;
	emu_timer *bitbanger_input_timer;
	int output_value;
	int build_count;
	int build_byte;
	attotime idle_delay;
	attotime current_baud;
	UINT32 input_buffer_size;
	UINT32 input_buffer_cursor;
	int mode;
	int baud;
	int tune;
	UINT8 input_buffer[1000];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK(bitbanger_output_timer);
static TIMER_CALLBACK(bitbanger_input_timer);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/


/*-------------------------------------------------
    get_token - safely gets the bitbanger data
-------------------------------------------------*/

INLINE bitbanger_token *get_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == BITBANGER);
	return (bitbanger_token *) downcast<legacy_device_base *>(device)->token();
}


/*-------------------------------------------------
    get_config - safely gets the bitbanger config
-------------------------------------------------*/

INLINE const bitbanger_config *get_config(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == BITBANGER);
	return (const bitbanger_config *) device->baseconfig().static_config();
}



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


/*-------------------------------------------------
    native_output - outputs data to a file
-------------------------------------------------*/

static void native_output(device_t *bitbanger, UINT8 data)
{
	device_image_interface *image = dynamic_cast<device_image_interface *>(bitbanger);
	if (image->exists())
	{
		image->fwrite(&data, 1);
	}
}


/*-------------------------------------------------
    native_input - inputs data from a file
-------------------------------------------------*/

static UINT32 native_input(device_t *bitbanger, void *buffer, UINT32 length)
{
	device_image_interface *image = dynamic_cast<device_image_interface *>(bitbanger);
	if (image->exists())
	{
		return image->fread(buffer, length);
	}

	return 0;
}


/*-------------------------------------------------
    bitbanger_mode_string
-------------------------------------------------*/
const char *bitbanger_mode_string(device_t *device)
{
	bitbanger_token *bi = get_token(device);
	static const char *const modes[] = {"Printer", "Modem"};

	return(modes[bi->mode]);
}


/*-------------------------------------------------
    bitbanger_inc_mode
-------------------------------------------------*/
bool bitbanger_inc_mode(device_t *device, bool test)
{
	bitbanger_token *bi = get_token(device);
   int adjust_mode = (int)bi->mode + 1;

   if( adjust_mode >= BITBANGER_MODE_MAX )
      return FALSE;

   if( !test)
      bi->mode = adjust_mode;

   return TRUE;
}


/*-------------------------------------------------
    bitbanger_dec_mode
-------------------------------------------------*/
bool bitbanger_dec_mode(device_t *device, bool test)
{
	bitbanger_token *bi = get_token(device);
   int adjust_mode = bi->mode - 1;

   if( adjust_mode < 0 )
      return FALSE;

   if( !test)
      bi->mode = adjust_mode;

   return TRUE;
}


/*-------------------------------------------------
    bitbanger_tune_string
-------------------------------------------------*/
const char *bitbanger_tune_string(device_t *device)
{
	bitbanger_token *bi = get_token(device);
	static const char *const tunes[] = {"-2.0%", "-1.75%", "-1.5%", "-1.25%", "-1.0%", "-0.75%", "-0.5", "-0.25%", "\xc2\xb1""0%",
                                 "+0.25%",  "+0.5%", "+0.75%", "+1.0%", "+1.25%", "+1.5%", "+1.75%", "+2.0%"};

	return(tunes[bi->tune]);
}


/*-------------------------------------------------
    bitbanger_tune_value
-------------------------------------------------*/
static float bitbanger_tune_value(device_t *device)
{
	bitbanger_token *bi = get_token(device);
	static const float tunes[] = {0.97f, 0.9825f, 0.985f, 0.9875f, 0.99f, 0.9925f, 0.995f, 0.9975f, 1.0f,
                  1.0025f, 1.005f, 1.0075f, 1.01f, 1.0125f, 1.015f, 1.0175f, 1.02f};

	return(tunes[bi->tune]);
}


/*-------------------------------------------------
    bitbanger_baud_value
-------------------------------------------------*/

static UINT32 bitbanger_baud_value(device_t *device)
{
	bitbanger_token *bi = get_token(device);
	static const float bauds[] = { 150.0f, 300.0f, 600.0f, 1200.0f, 2400.0f, 4800.0f, 9600.0f,
            14400.0f, 28800.0f, 38400.0f, 57600.0f, 115200.0f};
	float result = bitbanger_tune_value(device) * bauds[bi->baud];
	return (UINT32)result;
}


/*-------------------------------------------------
    bitbanger_baud_string
-------------------------------------------------*/

const char *bitbanger_baud_string(device_t *device)
{
	bitbanger_token *bi = get_token(device);
	static const char *const bauds[] = { "150", "300", "600", "1200", "2400", "4800",
                     "9600", "14400", "28800", "38400", "57600", "115200"};

	return(bauds[bi->baud]);
}


/*-------------------------------------------------
    bitbanger_inc_baud
-------------------------------------------------*/
bool bitbanger_inc_baud(device_t *device, bool test)
{
	bitbanger_token *bi = get_token(device);
   int adjust_baud = (int)bi->baud + 1;

   if( adjust_baud >= BITBANGER_BAUD_MAX )
      return FALSE;

   if( !test)
   {
      bi->baud = adjust_baud;
      bi->current_baud = ATTOTIME_IN_HZ(bitbanger_baud_value(device));
   }

   return TRUE;
}


/*-------------------------------------------------
    bitbanger_dec_baud
-------------------------------------------------*/
bool bitbanger_dec_baud(device_t *device, bool test)
{
	bitbanger_token *bi = get_token(device);
   int adjust_baud = bi->baud - 1;

   if( adjust_baud < 0 )
      return FALSE;

   if( !test)
   {
      bi->baud = adjust_baud;
      bi->current_baud = ATTOTIME_IN_HZ(bitbanger_baud_value(device));
   }

   return TRUE;
}


/*-------------------------------------------------
    bitbanger_inc_tune
-------------------------------------------------*/
bool bitbanger_inc_tune(device_t *device, bool test)
{
	bitbanger_token *bi = get_token(device);
   int adjust_tune = (int)bi->tune + 1;

   if( adjust_tune >= BITBANGER_TUNE_MAX )
      return FALSE;

   if( !test)
   {
      bi->tune = adjust_tune;
      bi->current_baud = ATTOTIME_IN_HZ(bitbanger_baud_value(device));
   }

   return TRUE;
}


/*-------------------------------------------------
    bitbanger_dec_tune
-------------------------------------------------*/
bool bitbanger_dec_tune(device_t *device, bool test)
{
	bitbanger_token *bi = get_token(device);
   int adjust_tune = bi->tune - 1;

   if( adjust_tune < 0 )
      return FALSE;

   if( !test)
   {
      bi->tune = adjust_tune;
      bi->current_baud = ATTOTIME_IN_HZ(bitbanger_baud_value(device));
   }

   return TRUE;
}


/*-------------------------------------------------
    bitbanger_bytes_to_bits_81N
-------------------------------------------------*/
static void bitbanger_bytes_to_bits_81N(device_t *img)
{
   bitbanger_token *bi = get_token(img);
   UINT8 byte_buffer[100];
   UINT32 byte_buffer_size, bit_buffer_size;
   int i, j;
   static const UINT8 bitmask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

   bit_buffer_size = 0;
   byte_buffer_size = native_input(img, byte_buffer, sizeof(byte_buffer));

   /* Translate byte buffer into bit buffer using: 1 start bit, 8 data bits, 1 stop bit, no parity */

   for( i=0; i<byte_buffer_size; i++ )
   {
      bi->input_buffer[bit_buffer_size++] = 0;

      for( j=0; j<8; j++ )
      {
         if( byte_buffer[i] & bitmask[j] )
            bi->input_buffer[bit_buffer_size++] = 1;
         else
            bi->input_buffer[bit_buffer_size++] = 0;
      }

      bi->input_buffer[bit_buffer_size++] = 1;
   }

   bi->input_buffer_size = bit_buffer_size;
   bi->input_buffer_cursor = 0;
}


/*-------------------------------------------------
    DEVICE_START(bitbanger)
-------------------------------------------------*/

static DEVICE_START(bitbanger)
{
	bitbanger_token *bi;
	const bitbanger_config *config = get_config(device);

	bi = get_token(device);

	/* output config */
	bi->build_count = 0;
	bi->bitbanger_output_timer = timer_alloc(device->machine, bitbanger_output_timer, (void *) device);

	/* input config */
	bi->bitbanger_input_timer = timer_alloc(device->machine, bitbanger_input_timer, (void *) device );
	bi->idle_delay = ATTOTIME_IN_SEC(1);
	bi->input_buffer_size = 0;
	bi->input_buffer_cursor = 0;

	bi->mode = config->default_mode;
	bi->baud = config->default_baud;
	bi->tune = config->default_tune;
	bi->current_baud = ATTOTIME_IN_HZ(bitbanger_baud_value(device));

	/* test callback */
	if(!config->input_callback)
		fatalerror("Misconfigured bitbanger device: input_callback cannot be NULL\n");
}


/*-------------------------------------------------
    TIMER_CALLBACK(bitbanger_output_timer)
-------------------------------------------------*/

static TIMER_CALLBACK(bitbanger_output_timer)
{
	device_t *device = (device_t *) ptr;
	bitbanger_token *bi = get_token(device);

   /* this ia harded coded for 8-1-N */
   if( bi->output_value )
      bi->build_byte |= 0x200;

   bi->build_byte >>= 1;
   bi->build_count--;

   if(bi->build_count == 0)
   {
      if( bi->output_value == 1 )
         native_output( device, bi->build_byte );
      else
         logerror("Bitbanger: Output framing error.\n" );

      timer_reset(bi->bitbanger_output_timer, attotime_never);
   }
}


/*-------------------------------------------------
    TIMER_CALLBACK(bitbanger_input_timer)
-------------------------------------------------*/

static TIMER_CALLBACK(bitbanger_input_timer)
{
	device_t *device = (device_t *) ptr;
	bitbanger_token *bi = get_token(device);
	const bitbanger_config *config = get_config(device);

   if(bi->input_buffer_cursor == bi->input_buffer_size)
   {
      /* get more data */
      bitbanger_bytes_to_bits_81N(device);

      if(bi->input_buffer_size == 0)
      {
         /* no more data, wait and try again */
         bi->idle_delay = attotime_min(attotime_add(bi->idle_delay, ATTOTIME_IN_MSEC(100)), ATTOTIME_IN_SEC(1));
         timer_adjust_oneshot(bi->bitbanger_input_timer, bi->idle_delay, 0);


         if( bi->mode == BITBANGER_MODEM )
            config->input_callback(machine, 1);
         else
            config->input_callback(machine, 0);

         return;
      }
      else
      {
         bi->idle_delay = bi->current_baud;
         timer_adjust_periodic(bi->bitbanger_input_timer, bi->idle_delay, 0, bi->idle_delay);
      }
   }

   /* send bit to driver */
   config->input_callback(machine, bi->input_buffer[(bi->input_buffer_cursor)++]);
}


/*-------------------------------------------------
    bitbanger_output - outputs data to a bitbanger
    port
-------------------------------------------------*/

void bitbanger_output(device_t *device, int value)
{
	bitbanger_token *bi = get_token(device);
   attotime one_point_five_baud;

   if( bi->build_count == 0 && bi->output_value == 1 && value == 0 )
   {
      /* we found our start bit */
      /* eight bits of data, plus one of stop */
      bi->build_count = 9;
      bi->build_byte = 0;

      one_point_five_baud = attotime_add(bi->current_baud, attotime_div(bi->current_baud,2));
      timer_adjust_periodic(bi->bitbanger_output_timer, one_point_five_baud, 0, bi->current_baud);
   }

   //fprintf(stderr,"%s, %d\n", attotime_string(timer_get_time(device->machine),9), value);
   bi->output_value = value;
}


/*-------------------------------------------------
    DEVICE_IMAGE_LOAD( bitbanger )
-------------------------------------------------*/

static DEVICE_IMAGE_LOAD( bitbanger )
{
	device_t *device = &image.device();
	bitbanger_token *bi;
	bi = get_token(device);

	timer_enable(bi->bitbanger_input_timer, TRUE);
	timer_adjust_periodic(bi->bitbanger_input_timer, attotime_zero, 0, ATTOTIME_IN_SEC(1));

	/* we don't need to do anything special */
	return IMAGE_INIT_PASS;
}


/*-------------------------------------------------
    DEVICE_IMAGE_UNLOAD( bitbanger )
-------------------------------------------------*/

static DEVICE_IMAGE_UNLOAD( bitbanger )
{
	device_t *device = &image.device();
	bitbanger_token *bi;
	bi = get_token(device);

	timer_enable(bi->bitbanger_input_timer, FALSE);
}


/*-------------------------------------------------
    DEVICE_GET_INFO(bitbanger) - device getinfo
    function
-------------------------------------------------*/

DEVICE_GET_INFO(bitbanger)
{
	switch(state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(bitbanger_token); break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:			info->i = 0; break;
		case DEVINFO_INT_IMAGE_TYPE:					info->i = IO_PRINTER; break;
		case DEVINFO_INT_IMAGE_READABLE:				info->i = 1; break;
		case DEVINFO_INT_IMAGE_WRITEABLE:				info->i = 1; break;
		case DEVINFO_INT_IMAGE_CREATABLE:				info->i = 1; break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(bitbanger); break;
		case DEVINFO_FCT_IMAGE_LOAD:					info->f = (genf *) DEVICE_IMAGE_LOAD_NAME(bitbanger);		break;
		case DEVINFO_FCT_IMAGE_UNLOAD:					info->f = (genf *) DEVICE_IMAGE_UNLOAD_NAME(bitbanger);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Bitbanger"); break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Bitbanger"); break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__); break;
		case DEVINFO_STR_IMAGE_FILE_EXTENSIONS:			strcpy(info->s, "prn"); break;
	}
}

DEFINE_LEGACY_IMAGE_DEVICE(BITBANGER, bitbanger);
