/*****************************************************************************
 *
 * machine/i8271.h
 *
 ****************************************************************************/

#ifndef I8271_H_
#define I8271_H_

/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(I8271, i8271);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct i8271_interface
{
	void (*interrupt)(device_t *device, int state);
	void (*dma_request)(device_t *device, int state, int read_);
	const char *floppy_drive_tags[2];
} i8271_interface;

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/
READ8_DEVICE_HANDLER (i8271_r);
WRITE8_DEVICE_HANDLER(i8271_w);

READ8_DEVICE_HANDLER (i8271_dack_r);
WRITE8_DEVICE_HANDLER(i8271_dack_w);

READ8_DEVICE_HANDLER (i8271_data_r);
WRITE8_DEVICE_HANDLER(i8271_data_w);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8271_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, I8271, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#endif /* I8271_H_ */
