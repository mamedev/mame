/*********************************************************************

    pc_turbo.h

    The PC "turbo" button

**********************************************************************/

#ifndef PC_TURBO_H
#define PC_TURBO_H

int pc_turbo_setup(running_machine &machine, device_t *cpu, const char *port, int mask, double off_speed, double on_speed);


#endif /* PC_TURBO_H */
