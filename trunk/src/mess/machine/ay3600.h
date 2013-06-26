/***************************************************************************

    ay3600.h

    Include file for AY3600 keyboard; used by Apple IIs

***************************************************************************/

#ifndef AY3600_H
#define AY3600_H


/*----------- defined in machine/ay3600.c -----------*/

int AY3600_init(running_machine &machine);
int AY3600_anykey_clearstrobe_r(running_machine &machine);
int AY3600_keydata_strobe_r(running_machine &machine);
int AY3600_keymod_r(running_machine &machine);

#endif /* AY3600_H */
