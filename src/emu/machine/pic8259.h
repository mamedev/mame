/**********************************************************************

    8259 PIC interface and emulation

**********************************************************************/

#ifndef PIC8259_H
#define PIC8259_H

int pic8259_init(int count, void (*set_int_line)(int which, int interrupt));
int pic8259_acknowledge(int which);
void pic8259_set_irq_line(int which, int irq, int state);

READ8_HANDLER ( pic8259_0_r );
READ8_HANDLER ( pic8259_1_r );
WRITE8_HANDLER ( pic8259_0_w );
WRITE8_HANDLER ( pic8259_1_w );

READ16_HANDLER ( pic8259_16le_0_r );
READ16_HANDLER ( pic8259_16le_1_r );
WRITE16_HANDLER ( pic8259_16le_0_w );
WRITE16_HANDLER ( pic8259_16le_1_w );

READ32_HANDLER ( pic8259_32le_0_r );
READ32_HANDLER ( pic8259_32le_1_r );
WRITE32_HANDLER ( pic8259_32le_0_w );
WRITE32_HANDLER ( pic8259_32le_1_w );

READ64_HANDLER ( pic8259_64be_0_r );
READ64_HANDLER ( pic8259_64be_1_r );
WRITE64_HANDLER ( pic8259_64be_0_w );
WRITE64_HANDLER ( pic8259_64be_1_w );

#endif /* PIC8259_H */
