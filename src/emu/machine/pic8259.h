/**********************************************************************

    8259 PIC interface and emulation

**********************************************************************/

#ifndef __PIC8259_H_
#define __PIC8259_H_

#define PIC8259	DEVICE_GET_INFO_NAME(pic8259)

typedef void (*pic8259_set_int_line_func)(const device_config *device, int interrupt);
#define PIC8259_SET_INT_LINE(name)	void name(const device_config *device, int interrupt)

struct pic8259_interface {
	/* Called when int line changes */
	pic8259_set_int_line_func	set_int_line;
};

DEVICE_GET_INFO(pic8259);
READ8_DEVICE_HANDLER( pic8259_r );
WRITE8_DEVICE_HANDLER( pic8259_w );
int pic8259_acknowledge(const device_config *device);
void pic8259_set_irq_line(const device_config *device, int irq, int state);

#endif /* __PIC8259_H_ */
