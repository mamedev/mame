/***************************************************************************

    machine/upd765.h

    Functions to emulate a NEC upd765/Intel 8272 compatible
    floppy disk controller

***************************************************************************/

#ifndef __UPD765_H__
#define __UPD765_H__

#include "devcb.h"
#include "imagedev/flopdrv.h"


/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(UPD765A, upd765a);
DECLARE_LEGACY_DEVICE(UPD765B, upd765b);
DECLARE_LEGACY_DEVICE(SMC37C78, smc37c78);
DECLARE_LEGACY_DEVICE(UPD72065, upd72065);

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* RDY pin connected state */
typedef enum
{
	UPD765_RDY_PIN_NOT_CONNECTED = 0,
	UPD765_RDY_PIN_CONNECTED = 1
} UPD765_RDY_PIN;

#define UPD765_DAM_DELETED_DATA 0x0f8
#define UPD765_DAM_DATA 0x0fb

typedef device_t *(*upd765_get_image_func)(device_t *device, int floppy_index);
#define UPD765_GET_IMAGE(name)	device_t *name(device_t *device, int floppy_index )


typedef struct upd765_interface
{
	/* interrupt issued */
	devcb_write_line out_int_func;

	/* dma data request */
	devcb_write_line out_drq_func;

	/* image lookup */
	upd765_get_image_func get_image;

	UPD765_RDY_PIN rdy_pin;

	const char *floppy_drive_tags[4];
} upd765_interface;


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* read of data register */
READ8_DEVICE_HANDLER(upd765_data_r);
/* write to data register */
WRITE8_DEVICE_HANDLER(upd765_data_w);
/* read of main status register */
READ8_DEVICE_HANDLER(upd765_status_r);

/* dma acknowledge with write */
WRITE8_DEVICE_HANDLER(upd765_dack_w);
/* dma acknowledge with read */
READ8_DEVICE_HANDLER(upd765_dack_r);

/* reset upd765 */
void upd765_reset(device_t *device, int);

/* reset pin of upd765 */
WRITE_LINE_DEVICE_HANDLER(upd765_reset_w);

/* set upd765 terminal count input state */
WRITE_LINE_DEVICE_HANDLER(upd765_tc_w);

/* set upd765 ready input*/
WRITE_LINE_DEVICE_HANDLER(upd765_ready_w);

void upd765_idle(device_t *device);

/*********************/
/* STATUS REGISTER 1 */

/* this is set if a TC signal was not received after the sector data was read */
#define UPD765_ST1_END_OF_CYLINDER (1<<7)
/* this is set if the sector ID being searched for is not found */
#define UPD765_ST1_NO_DATA (1<<2)
/* set if disc is write protected and a write/format operation was performed */
#define UPD765_ST1_NOT_WRITEABLE (1<<1)

/*********************/
/* STATUS REGISTER 2 */

/* C parameter specified did not match C value read from disc */
#define UPD765_ST2_WRONG_CYLINDER (1<<4)
/* C parameter specified did not match C value read from disc, and C read from disc was 0x0ff */
#define UPD765_ST2_BAD_CYLINDER (1<<1)
/* this is set if the FDC encounters a Deleted Data Mark when executing a read data
command, or FDC encounters a Data Mark when executing a read deleted data command */
#define UPD765_ST2_CONTROL_MARK (1<<6)

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_UPD765A_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, UPD765A, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_UPD765A_MODIFY(_tag, _intrf) \
  MCFG_DEVICE_MODIFY(_tag)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_UPD765A_REMOVE(_tag)		\
  MCFG_DEVICE_REMOVE(_tag)

#define MCFG_UPD765B_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, UPD765B, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_UPD765B_MODIFY(_tag, _intrf) \
  MCFG_DEVICE_MODIFY(_tag)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_UPD765B_REMOVE(_tag)		\
  MCFG_DEVICE_REMOVE(_tag)

#define MCFG_SMC37C78_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, SMC37C78, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_SMC37C78_MODIFY(_tag, _intrf) \
  MCFG_DEVICE_MODIFY(_tag)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_SMC37C78_REMOVE(_tag)		\
  MCFG_DEVICE_REMOVE(_tag)

#define MCFG_UPD72065_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, UPD72065, 0) \
	MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_UPD72065_MODIFY(_tag, _intrf) \
  MCFG_DEVICE_MODIFY(_tag)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_UPD72065_REMOVE(_tag)		\
  MCFG_DEVICE_REMOVE(_tag)


#endif /* __UPD765_H__ */
