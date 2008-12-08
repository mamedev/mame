/*
  fairchild f3853 static ram interface smi
  with integrated interrupt controller and timer

  timer shift register basically the same as in f3851!
*/

#pragma once

#ifndef __F3853_H__
#define __F3853_H__


#include "cpuintrf.h"

typedef struct _f3853_config f3853_config;
struct _f3853_config
{
    int frequency;
    void (*interrupt_request)(UINT16 addr, int level);
};

void f3853_init(running_machine *machine, const f3853_config *config);
CPU_RESET( f3853 );

// ports 0x0c - 0x0f
 READ8_HANDLER(f3853_r);
WRITE8_HANDLER(f3853_w);

void f3853_set_external_interrupt_in_line(int level);
void f3853_set_priority_in_line(int level);

#endif /* __F3853_H__ */
