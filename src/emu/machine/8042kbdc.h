/**********************************************************************

    8042 Keyboard Controller Emulation

    This is the keyboard controller used in the IBM AT and further
    models.  It is a popular controller for PC style keyboards

**********************************************************************/

#ifndef KBDC8042_H
#define KBDC8042_H

typedef enum
{
	KBDC8042_STANDARD,
	KBDC8042_PS2,		/* another timing of integrated controller */
	KBDC8042_AT386		/* hack for at386 driver */
} kbdc8042_type_t;


struct kbdc8042_interface
{
	kbdc8042_type_t type;
	void (*set_gate_a20)(int a20);
	void (*keyboard_interrupt)(int state);
};



void kbdc8042_init(const struct kbdc8042_interface *intf);

READ8_HANDLER(kbdc8042_8_r);
WRITE8_HANDLER(kbdc8042_8_w);
READ32_HANDLER(kbdc8042_32le_r);
WRITE32_HANDLER(kbdc8042_32le_w);
READ64_HANDLER(kbdc8042_64be_r);
WRITE64_HANDLER(kbdc8042_64be_w);

#endif /* KBDC8042_H */

