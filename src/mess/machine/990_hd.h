/*
    990_hd.h: include file for 990_hd.c
*/
#ifndef __990_HD_H_
#define __990_HD_H_

#include "imagedev/harddriv.h"

MACHINE_START( ti990_hdc );

void ti990_hdc_init(running_machine &machine, void (*interrupt_callback)(running_machine &machine, int state));

READ16_HANDLER(ti990_hdc_r);
WRITE16_HANDLER(ti990_hdc_w);

MACHINE_CONFIG_EXTERN( ti990_hdc );

#endif
