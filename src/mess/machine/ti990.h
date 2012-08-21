/*
    ti990.h: header file for ti990.c
*/

void ti990_reset_int(void);
void ti990_set_int_line(running_machine &machine, int line, int state);
void ti990_set_int2(device_t *device, int state);
void ti990_set_int3(running_machine &machine, int state);
void ti990_set_int6(running_machine &machine, int state);
void ti990_set_int7(running_machine &machine, int state);
void ti990_set_int9(running_machine &machine, int state);
void ti990_set_int10(running_machine &machine, int state);
void ti990_set_int13(running_machine &machine, int state);

void ti990_hold_load(running_machine &machine);

 READ8_HANDLER ( ti990_panel_read );
WRITE8_HANDLER ( ti990_panel_write );

void ti990_line_interrupt(running_machine &machine);
void ti990_ckon_ckof_callback(device_t *device, int state);

void ti990_cpuboard_reset(void);
