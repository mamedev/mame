/**********************************************************************

    MOS 6526/8520 CIA interface and emulation

    This function emulates all the functionality of up to 2 MOS6526 or
    MOS8520 complex interface adapters.

**********************************************************************/

#ifndef __6526CIA_H__
#define __6526CIA_H__

#include "devcb.h"


/***************************************************************************
    MACROS
***************************************************************************/

#define CIA6526R1		DEVICE_GET_INFO_NAME(cia6526r1)
#define CIA6526R2		DEVICE_GET_INFO_NAME(cia6526r1)
#define CIA8520			DEVICE_GET_INFO_NAME(cia8520)

#define MDRV_CIA6526_ADD(_tag, _variant, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, _variant, _clock) \
	MDRV_DEVICE_CONFIG(_config)

#define MDRV_CIA8520_ADD(_tag, _clock, _config) \
	MDRV_DEVICE_ADD(_tag, CIA8520, _clock) \
	MDRV_DEVICE_CONFIG(_config)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _cia6526_interface cia6526_interface;
struct _cia6526_interface
{
	devcb_write_line irq_func;
	devcb_write_line pc_func;

	int tod_clock;

	struct
	{
		devcb_read8 read;
		devcb_write8 write;
	} port[2];
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO(cia6526r1);
DEVICE_GET_INFO(cia6526r2);
DEVICE_GET_INFO(cia8520);

/* configuration */
void cia_set_port_mask_value(const device_config *device, int port, int data);

/* reading and writing */
READ8_DEVICE_HANDLER( cia_r );
WRITE8_DEVICE_HANDLER( cia_w );
void cia_clock_tod(const device_config *device);
void cia_issue_index(const device_config *device);
void cia_set_input_cnt(const device_config *device, int data);
void cia_set_input_sp(const device_config *device, int data);

WRITE_LINE_DEVICE_HANDLER( mos6526_tod_w );
WRITE_LINE_DEVICE_HANDLER( mos6526_cnt_w );
WRITE_LINE_DEVICE_HANDLER( mos6526_sp_w );
WRITE_LINE_DEVICE_HANDLER( mos6526_flag_w );

/* accessors */
UINT8 cia_get_output_a(const device_config *device);
UINT8 cia_get_output_b(const device_config *device);
int cia_get_irq(const device_config *device);

#endif /* __6526CIA_H__ */
