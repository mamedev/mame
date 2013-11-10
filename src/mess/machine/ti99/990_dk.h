/*
    990_dk.h: include file for 990_dk.c
*/
LEGACY_FLOPPY_OPTIONS_EXTERN(fd800);

void fd800_machine_init(running_machine &machine, void (*interrupt_callback)(running_machine &machine, int state));

extern  DECLARE_READ8_HANDLER(fd800_cru_r);
extern DECLARE_WRITE8_HANDLER(fd800_cru_w);
