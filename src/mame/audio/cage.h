// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari CAGE Audio Board

****************************************************************************/

#define CAGE_IRQ_REASON_DATA_READY      (1)
#define CAGE_IRQ_REASON_BUFFER_EMPTY    (2)

MACHINE_CONFIG_EXTERN( cage );
MACHINE_CONFIG_EXTERN( cage_seattle );

void cage_init(running_machine &machine, offs_t speedup);
void cage_set_irq_handler(void (*irqhandler)(running_machine &, int));
void cage_reset_w(address_space &space, int state);

UINT16 cage_main_r(address_space &space);
void cage_main_w(address_space &space, UINT16 data);

UINT16 cage_control_r(running_machine &machine);
void cage_control_w(running_machine &machine, UINT16 data);
