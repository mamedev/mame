/**********************************************************************

    Motorola 6821 PIA interface and emulation

    Notes:
        * pia_get_port_b_z_mask() gives the caller the bitmask
          that show which bits are high-impendance when
          reading port B, thus neither 0 or 1.
          pia_get_output_cb2_z() returns the same
          information for the CB2 pin
        * pia_set_port_a_z_mask allows the input callback to
          indicate which port A bits are disconnected.
          For these bit, the read operation will return the
          output buffer's contents
        * the 'alt' interface functions are used when the A0 and A1
          address bits are swapped
        * all 'int' data or return values are Boolean

**********************************************************************/

#ifndef __6821NEW_H__
#define __6821NEW_H__

#include "driver.h"
#include "devcb.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define PIA6821		DEVICE_GET_INFO_NAME(pia6821)
#define PIA6822		DEVICE_GET_INFO_NAME(pia6822)


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/*------------- PIA interface structure -----------------*/

typedef struct _pia6821_interface pia6821_interface;
struct _pia6821_interface
{
	devcb_read8 in_a_func;
	devcb_read8 in_b_func;
	devcb_read8 in_ca1_func;
	devcb_read8 in_cb1_func;
	devcb_read8 in_ca2_func;
	devcb_read8 in_cb2_func;
	devcb_write8 out_a_func;
	devcb_write8 out_b_func;
	devcb_write8 out_ca2_func;
	devcb_write8 out_cb2_func;
	devcb_write_line irq_a_func;
	devcb_write_line irq_b_func;
};


/***************************************************************************
    PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO(pia6821);
DEVICE_GET_INFO(pia6822);

READ8_DEVICE_HANDLER(pia_r);
WRITE8_DEVICE_HANDLER(pia_w);

READ8_DEVICE_HANDLER(pia_alt_r);
WRITE8_DEVICE_HANDLER(pia_alt_w);

UINT8 pianew_get_port_b_z_mask(const device_config *device);  /* see first note */
void pianew_set_port_a_z_mask(const device_config *device, UINT8 data);  /* see second note */

READ8_DEVICE_HANDLER(pia_porta_r);
WRITE8_DEVICE_HANDLER(pia_porta_w);
void pianew_set_input_a(const device_config *device, UINT8 data, UINT8 z_mask);
UINT8 pianew_get_output_a(const device_config *device);

READ8_DEVICE_HANDLER(pia_ca1_r);
WRITE8_DEVICE_HANDLER(pia_ca1_w);

READ8_DEVICE_HANDLER(pia_ca2_r);
WRITE8_DEVICE_HANDLER(pia_ca2_w);
int pianew_get_output_ca2(const device_config *device);
int pianew_get_output_ca2_z(const device_config *device);

READ8_DEVICE_HANDLER(pia_portb_r);
WRITE8_DEVICE_HANDLER(pia_portb_w);
UINT8 pianew_get_output_b(const device_config *device);

READ8_DEVICE_HANDLER(pia_cb1_r);
WRITE8_DEVICE_HANDLER(pia_cb1_w);

READ8_DEVICE_HANDLER(pia_cb2_r);
WRITE8_DEVICE_HANDLER(pia_cb2_w);
int pianew_get_output_cb2(const device_config *device);
int pianew_get_output_cb2_z(const device_config *device);

int pianew_get_irq_a(const device_config *device);
int pianew_get_irq_b(const device_config *device);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PIA6821_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PIA6821, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6821_MODIFY(_tag, _intrf) \
	MDRV_DEVICE_MODIFY(_tag, PIA6821) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6821_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, PIA6821)

#define MDRV_PIA6822_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PIA6822, 0) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6822_MODIFY(_tag, _intrf) \
	MDRV_DEVICE_MODIFY(_tag, PIA6822) \
	MDRV_DEVICE_CONFIG(_intrf)

#define MDRV_PIA6822_REMOVE(_tag) \
	MDRV_DEVICE_REMOVE(_tag, PIA6821)

#endif /* __6821NEW_H__ */
