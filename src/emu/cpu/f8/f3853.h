/*
  fairchild f3853 static ram interface smi
  with integrated interrupt controller and timer

  timer shift register basically the same as in f3851!
*/

#include "cpuintrf.h"

typedef struct {
    int frequency;
    void (*interrupt_request)(UINT16 addr, int level);
} F3853_CONFIG;

void f3853_init(F3853_CONFIG *config);
void f3853_reset(void);

// ports 0x0c - 0x0f
 READ8_HANDLER(f3853_r);
WRITE8_HANDLER(f3853_w);

void f3853_set_external_interrupt_in_line(int level);
void f3853_set_priority_in_line(int level);

