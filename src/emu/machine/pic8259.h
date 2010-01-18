/**********************************************************************

    8259 PIC interface and emulation

**********************************************************************/

#ifndef __PIC8259_H_
#define __PIC8259_H_

#define PIC8259	DEVICE_GET_INFO_NAME(pic8259)

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*pic8259_set_int_line_func)(running_device *device, int interrupt);
#define PIC8259_SET_INT_LINE(name)	void name(running_device *device, int interrupt)


struct pic8259_interface {
	/* Called when int line changes */
	pic8259_set_int_line_func	set_int_line;
};


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_PIC8259_ADD(_tag, _intrf) \
	MDRV_DEVICE_ADD(_tag, PIC8259, 0) \
	MDRV_DEVICE_CONFIG(_intrf)


/* device interface */
DEVICE_GET_INFO(pic8259);
READ8_DEVICE_HANDLER( pic8259_r );
WRITE8_DEVICE_HANDLER( pic8259_w );
int pic8259_acknowledge(running_device *device);
void pic8259_set_irq_line(running_device *device, int irq, int state);

#endif /* __PIC8259_H_ */
