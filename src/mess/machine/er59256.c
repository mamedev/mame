/*********************************************************************

    er59256.c

    Microchip ER59256 serial eeprom.


*********************************************************************/

#include "emu.h"
#include "er59256.h"

/* LOGLEVEL 0=no logging, 1=just commands and data, 2=everything ! */

#define LOGLEVEL			0

#define LOG(level,...)      if(LOGLEVEL>=level) logerror(__VA_ARGS__)
#define LOG_BITS(bits)      logerror("CS=%d CK=%d DI=%d DO=%d", (bits&CS_MASK) ? 1 : 0, (bits&CK_MASK) ? 1 : 0, (bits&DI_MASK) ? 1 : 0, (bits&DO_MASK) ? 1 : 0)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _er59256_t er59256_t;
struct _er59256_t
{
    /* The actual memory */
	UINT16  eerom[EEROM_WORDS];

    /* Bits as they appear on the io pins, current state */
    UINT8   io_bits;

    /* Bits as they appear on the io pins, previous state */
    UINT8   old_io_bits;


    /* the 16 bit shift in/out reg */
    UINT16  in_shifter;
    UINT32  out_shifter;

    /* Count of bits received since last CS low->high */
    UINT8   bitcount;

    /* Command & addresss */
    UINT8   command;

    /* Write enable and write in progress flags */
    UINT8   flags;
};

/***************************************************************************
    FUNCTION PROTOTYPES
************************************************************************/

static void decode_command(er59256_t *er59256);

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

INLINE er59256_t *get_token(device_t *device)
{
	assert(device->type() == ER59256);
	return (er59256_t *) downcast<legacy_device_base *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void er59256_preload_rom(device_t *device, const UINT16 *rom_data, int count)
{
	er59256_t *er59256 = get_token(device);
    int WordNo;

    logerror("Preloading %d words of data\n",count);

    if(count>EEROM_WORDS)
        memcpy(&er59256->eerom,rom_data,count*2);
    else
        memcpy(&er59256->eerom,rom_data,EEROM_WORDS*2);

    for(WordNo=0;WordNo<EEROM_WORDS;WordNo++)
        logerror("%04X ",er59256->eerom[WordNo]);

    logerror("\n");
}

UINT8 er59256_data_loaded(device_t *device)
{
	er59256_t *er59256 = get_token(device);

    return (er59256->flags & FLAG_DATA_LOADED) ? 1 : 0;
}

/*-------------------------------------------------
    DEVICE_START( er59256 )
-------------------------------------------------*/

static DEVICE_START( er59256 )
{
	er59256_t *er59256 = get_token(device);

    memset(er59256, 0x00, sizeof(er59256_t));

    // Start with rom defaulted to erased
	memset(&er59256->eerom, 0xFF, EEROM_WORDS*2);

    er59256->command=CMD_INVALID;

    er59256->flags&= ~FLAG_DATA_LOADED;
}

static DEVICE_STOP( er59256 )
{
    /* Save contents of eerom */
}

void er59256_set_iobits(device_t *device, UINT8 newbits)
{
	er59256_t *er59256 = get_token(device);
    //UINT32  bit;

    // Make sure we only apply valid bits
    newbits&=ALL_MASK;

    if(LOGLEVEL>1)
    {
        logerror("er59256:newbits=%02X : ",newbits);
        LOG_BITS(newbits);
        logerror(" io_bits=%02X : ",er59256->io_bits);
        LOG_BITS(er59256->io_bits);
        logerror(" old_io_bits=%02X : ",er59256->old_io_bits);
        LOG_BITS(er59256->old_io_bits);
        logerror(" bitcount=%d, in_shifter=%04X, out_shifter=%05X, flags=%02X\n",er59256->bitcount,er59256->in_shifter,er59256->out_shifter,er59256->flags);
    }
    // Only do anything if the inputs have changed
    if((newbits&IN_MASK)!=(er59256->io_bits&IN_MASK))
    {
        // save the current state, then set the new one, remembering to preserve data out
        er59256->old_io_bits=er59256->io_bits;
        er59256->io_bits=(newbits & ~DO_MASK) | (er59256->old_io_bits&DO_MASK);

        if(CS_RISE(er59256))
        {
            er59256->flags&=~FLAG_START_BIT;
            er59256->command=CMD_INVALID;
        }

        if(LOGLEVEL>1)
        {
            if(CK_RISE(er59256)) logerror("er59256:CK rise\n");
            if(CS_RISE(er59256)) logerror("er59256:CS rise\n");
            if(CK_FALL(er59256)) logerror("er59256:CK fall\n");
            if(CS_FALL(er59256)) logerror("er59256:CS fall\n");
        }

        if(CK_RISE(er59256) && CS_VALID(er59256))
        {
            if((STARTED(er59256)==0) && (GET_DI(er59256)==1))
            {
                er59256->bitcount=0;
                er59256->flags|=FLAG_START_BIT;
            }
            else
            {
                SHIFT_IN(er59256);
                er59256->bitcount++;

                if(er59256->bitcount==CMD_BITLEN)
                    decode_command(er59256);

                if((er59256->bitcount==WRITE_BITLEN) && ((er59256->command & CMD_MASK)==CMD_WRITE))
                {
                    er59256->eerom[er59256->command & ADDR_MASK]=er59256->in_shifter;
                    LOG(1,"er59256:write[%02X]=%04X\n",(er59256->command & ADDR_MASK),er59256->in_shifter);
                    er59256->command=CMD_INVALID;
                }
                LOG(1,"out_shifter=%05X, io_bits=%02X\n",er59256->out_shifter,er59256->io_bits);
                SHIFT_OUT(er59256);
            }

            LOG(2,"io_bits:out=%02X\n",er59256->io_bits);
        }
    }
}

UINT8 er59256_get_iobits(device_t *device)
{
    er59256_t *er59256 = get_token(device);

    return er59256->io_bits;
}


static void decode_command(er59256_t *er59256)
{
    er59256->out_shifter=0x0000;
    er59256->command=(er59256->in_shifter & (CMD_MASK | ADDR_MASK));

    switch(er59256->command & CMD_MASK)
    {
        case CMD_READ   : er59256->out_shifter=er59256->eerom[er59256->command & ADDR_MASK];
                          LOG(1,"er59256:read[%02X]=%04X\n",(er59256->command&ADDR_MASK),er59256->eerom[er59256->command & ADDR_MASK]);
                          break;
        case CMD_WRITE  : break;
        case CMD_ERASE  : if (WRITE_ENABLED(er59256)) er59256->eerom[er59256->command & ADDR_MASK]=0xFF;
                          LOG(1,"er59256:erase[%02X]\n",(er59256->command&ADDR_MASK));
                          break;
        case CMD_EWEN   : er59256->flags|=FLAG_WRITE_EN;
                          LOG(1,"er59256:erase/write enabled\n");
                          break;
        case CMD_EWDS   : er59256->flags&=~FLAG_WRITE_EN;
                          LOG(1,"er59256:erase/write disabled\n");
                          break;
        case CMD_ERAL   : if (WRITE_ENABLED(er59256)) memset(&er59256->eerom, 0xFF, EEROM_WORDS*2);
                          LOG(1,"er59256:erase all\n");
                          break;
    }

    if ((er59256->command & CMD_MASK)!=CMD_WRITE)
        er59256->command=CMD_INVALID;
}

/*-------------------------------------------------
    DEVICE_GET_INFO( er59256 )
-------------------------------------------------*/

DEVICE_GET_INFO( er59256 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(er59256_t);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(er59256);	break;
		case DEVINFO_FCT_STOP:							info->stop = DEVICE_STOP_NAME(er59256);    break;
		case DEVINFO_FCT_RESET:							/* Nothing */								break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Microchip ER59256 serial eeprom.");		break;
		case DEVINFO_STR_FAMILY:						strcpy(info->s, "Microchip ER59256 serial eeprom.");		break;
		case DEVINFO_STR_VERSION:						strcpy(info->s, "1.0");						break;
		case DEVINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);					break;
		case DEVINFO_STR_CREDITS:						/* Nothing */								break;
	}
}

DEFINE_LEGACY_DEVICE(ER59256, er59256);
