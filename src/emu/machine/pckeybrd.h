/**********************************************************************

    pckeybrd.h

    PC-style keyboard emulation

    This emulation is decoupled from the AT 8042 emulation used in the
    IBM ATs and above

**********************************************************************/

#ifndef PCKEYBRD_H
#define PCKEYBRD_H

enum AT_KEYBOARD_TYPE
{
	AT_KEYBOARD_TYPE_PC,
	AT_KEYBOARD_TYPE_AT,
	AT_KEYBOARD_TYPE_MF2
};

void at_keyboard_init(running_machine &machine, AT_KEYBOARD_TYPE type);

void at_keyboard_polling(void);
int at_keyboard_read(void);
void at_keyboard_write(running_machine &machine, UINT8 data);
void at_keyboard_reset(running_machine &machine);
void at_keyboard_set_scan_code_set(int set);

INPUT_PORTS_EXTERN( pc_keyboard );
INPUT_PORTS_EXTERN( at_keyboard );

#endif /* PCKEYBRD_H */


