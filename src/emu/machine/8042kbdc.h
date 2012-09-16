/**********************************************************************

    8042 Keyboard Controller Emulation

    This is the keyboard controller used in the IBM AT and further
    models.  It is a popular controller for PC style keyboards

**********************************************************************/

#ifndef KBDC8042_H
#define KBDC8042_H

enum kbdc8042_type_t 
{
	KBDC8042_STANDARD,
	KBDC8042_PS2,		/* another timing of integrated controller */
	KBDC8042_AT386		/* hack for at386 driver */
};


struct kbdc8042_interface
{
	kbdc8042_type_t type;
	void (*set_gate_a20)(running_machine &machine, int a20);
	void (*keyboard_interrupt)(running_machine &machine, int state);
	void (*set_spkr)(running_machine &machine, int speaker);
	int (*get_out2)(running_machine &machine);



};



void kbdc8042_init(running_machine &machine, const struct kbdc8042_interface *intf);

READ8_HANDLER(kbdc8042_8_r);
WRITE8_HANDLER(kbdc8042_8_w);
READ64_HANDLER(kbdc8042_64be_r);
WRITE64_HANDLER(kbdc8042_64be_w);

#endif /* KBDC8042_H */

