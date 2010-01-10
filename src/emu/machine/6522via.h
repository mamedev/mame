/**********************************************************************

    Rockwell 6522 VIA interface and emulation

    This function emulates all the functionality of 6522
    versatile interface adapters.

    This is based on the M6821 emulation in MAME.

    Written by Mathis Rosenhauer

**********************************************************************/

#ifndef __6522VIA_H__
#define __6522VIA_H__



/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

#define VIA6522		DEVICE_GET_INFO_NAME(via6522)

#define MDRV_VIA6522_ADD(_tag, _clock, _intrf) \
	MDRV_DEVICE_ADD(_tag, VIA6522, _clock) \
	MDRV_DEVICE_CONFIG(_intrf)

#define	VIA_PB	    0
#define	VIA_PA	    1
#define	VIA_DDRB    2
#define	VIA_DDRA    3
#define	VIA_T1CL    4
#define	VIA_T1CH    5
#define	VIA_T1LL    6
#define	VIA_T1LH    7
#define	VIA_T2CL    8
#define	VIA_T2CH    9
#define	VIA_SR     10
#define	VIA_ACR    11
#define	VIA_PCR    12
#define	VIA_IFR    13
#define	VIA_IER    14
#define	VIA_PANH   15


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _via6522_interface via6522_interface;
struct _via6522_interface
{
	devcb_read8 in_a_func;
	devcb_read8 in_b_func;
	devcb_read_line in_ca1_func;
	devcb_read_line in_cb1_func;
	devcb_read_line in_ca2_func;
	devcb_read_line in_cb2_func;
	devcb_write8 out_a_func;
	devcb_write8 out_b_func;
	devcb_write_line out_ca1_func;
	devcb_write_line out_cb1_func;
	devcb_write_line out_ca2_func;
	devcb_write_line out_cb2_func;
	devcb_write_line irq_func;
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO(via6522);

READ8_DEVICE_HANDLER(via_r);
WRITE8_DEVICE_HANDLER(via_w);

WRITE8_DEVICE_HANDLER(via_porta_w);

READ8_DEVICE_HANDLER(via_portb_r);
WRITE8_DEVICE_HANDLER(via_portb_w);

READ_LINE_DEVICE_HANDLER(via_ca1_r);
WRITE_LINE_DEVICE_HANDLER(via_ca1_w);

READ_LINE_DEVICE_HANDLER(via_ca2_r);
WRITE_LINE_DEVICE_HANDLER(via_ca2_w);

READ_LINE_DEVICE_HANDLER(via_cb1_r);
WRITE_LINE_DEVICE_HANDLER(via_cb1_w);

READ_LINE_DEVICE_HANDLER(via_cb2_r);
WRITE_LINE_DEVICE_HANDLER(via_cb2_w);

#endif /* __6522VIA_H__ */
