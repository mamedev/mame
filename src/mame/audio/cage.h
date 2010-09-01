/***************************************************************************

    Atari CAGE Audio Board

****************************************************************************/

#define CAGE_IRQ_REASON_DATA_READY		(1)
#define CAGE_IRQ_REASON_BUFFER_EMPTY	(2)

MACHINE_CONFIG_EXTERN( cage );
MACHINE_CONFIG_EXTERN( cage_seattle );

void cage_init(running_machine *machine, offs_t speedup);
void cage_set_irq_handler(void (*irqhandler)(running_machine *, int));
void cage_reset_w(int state);

UINT16 main_from_cage_r(address_space *space);
UINT16 cage_control_r(void);
void main_to_cage_w(UINT16 data);
void cage_control_w(running_machine *machine, UINT16 data);
